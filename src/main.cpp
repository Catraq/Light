#include <time.h>
#include <stdint.h>

#include "platform.h"


#include "math/vec.h"
#include "math/mat4x4.h"

#include "camera.h"
#include "camera_input.h"


#include "model/raw_model.h"


#include "vertex_buffer.h"
#include "vertex_buffer_model.h"
#include "vertex_instance.h"


struct frame_info_update_result
{
	struct vec2 mouse_delta;
};


struct frame_info
{
	struct vec2 mouse;
	uint32_t width;
	uint32_t height;
};

static struct frame_info_update_result frame_info_update(struct frame_info *info)
{
	struct frame_info_update_result result = {};
	
	struct vec2 mouse;
	platform_mouse(&mouse);
	platform_resolution(&info->width, &info->height);
	
	const float half_width_f  = (float)info->width/2.0f;
	const float half_height_f = (float)info->height/2.0f;
	
	struct vec2 tmp;
	tmp.x = -(half_width_f/info->width -  mouse.x/info->width );
	tmp.y =  (half_height_f/info->height - mouse.y/info->height );
	
	result.mouse_delta = v2sub(info->mouse, tmp); 
	result.mouse_delta = v2scl(result.mouse_delta, 2.0f); 
	info->mouse = tmp;
	
	return result;
}



const char vertex_shader_source[] = 
{
	"#version 330 core 					\n"
	" layout ( location = 0 ) in vec3 r_position; 		\n"
	" layout ( location = 1 ) in vec3 r_normal;   		\n"
	" layout ( location = 2 ) in vec3 r_color; 		\n"
	" layout ( location = 3 ) in mat4 r_model; 		\n"
	" out vec3 v_normal; 					\n"
	" uniform scene{ 					\n"
	"	mat4 view; 					\n"
	" };							\n"
	" void main(){						\n"
	" 	vec4 n = normalize(vec4(r_normal, 0.0));		\n"
	" 	vec4 m_n = normalize(r_model * n);			\n"
	" 	vec4 c_n = normalize(view * m_n);			\n"
	" 	v_normal = c_n.xyz;				\n"
	" 	gl_Position = view  * r_model *vec4(r_position , 1.0f );        \n"
	"} 							\n"
};


const char fragment_shader_source[] = 
{
	"#version 330 core 				\n"
	"out vec4 fcolor; 				\n"
	"in vec3  v_normal;				\n"
	"layout(location=0) out vec3 normal_texture;	\n"
	"void main(){					\n"
	"	normal_texture = normalize(v_normal);	\n"
	"}						\n"
};



		
	
GLuint create_program(const char *vertex_source, const char *fragment_source)
{
	int result = 0;
	
	GLuint program = glCreateProgram();
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	
	
	const GLchar *vsource = (const GLchar *)vertex_source;
	const GLchar *fsource = (const GLchar *)fragment_source;

	glShaderSource(vertex_shader, 1, &vsource, 0);
	glShaderSource(fragment_shader, 1, &fsource, 0);
	
	glCompileShader(vertex_shader);
	glCompileShader(fragment_shader);
	
	
	GLint v_compiled = GL_FALSE;
	GLint f_compiled = GL_FALSE;
	
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &v_compiled);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &f_compiled);
	
	if( v_compiled == GL_FALSE 
	||  f_compiled == GL_FALSE )
	{
		
		GLchar log[255];
		GLsizei length;
		
		glGetShaderInfoLog(vertex_shader, 255, &length, (GLchar*)&log);
		if( length != 0 )
		{
			printf(" ---- Vertexshader log ---- \n %s \n", (GLchar*)&log);
		}
		
		length = 0;
		glGetShaderInfoLog(fragment_shader, 255, &length, (GLchar*)&log);
		if( length != 0)
		{
			printf(" ---- Fragmentshader log ---- \n %s \n", (GLchar*)&log);
		}
		
	}
	
	
	
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	
	glLinkProgram(program);
	
	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	
	GLint linked = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if(linked == GL_FALSE) {
		GLsizei length;
		GLchar log[255];
		glGetProgramInfoLog(program, 255, &length, (GLchar*)&log);
		printf(" ---- Link log ---- \n %s \n", (GLchar*)&log);
	}
	

	return program;
}



void translation_attribute_pointer(GLuint location, GLsizei stride, GLuint offset)
{	

	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(offset));
	glVertexAttribDivisor(location, 1);
	location++;
	
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(sizeof(struct vec4) + offset));
	glVertexAttribDivisor(location, 1);
	location++;

	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(2*sizeof(struct vec4) + offset));
	glVertexAttribDivisor(location, 1);
	location++;	
	
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(3*sizeof(struct vec4) + offset));
	glVertexAttribDivisor(location, 1);

}

struct scene_instance 
{

	struct frame_info frame_info;

	struct camera_view_state view_state;


};



void view_initialize(struct camera_view_state *view_state, GLuint buffer_base_index, uint32_t width, uint32_t height)
{


	camera_initialize(view_state, buffer_base_index);

	//Camera attributes 
	const float fov = 3.14f/3.0f;
	const float far = 1000.0f;
	const float close = 1.0f;
	float ratio = ((float)width / (float)height); 

	view_state->near = close;
	view_state->far = far;
	view_state->fov = fov;
	
	//Setup camera. 
	camera_view_projection(view_state, width, height);
	view_state->position = (vec3){0.0f, 0.0f, 5.0f};
	view_state->rotation = (vec3){0.0f, 0.0f, 0.0f};

}


int main(int args, char *argv[])
{
	int result;
	struct scene_instance scene;

	const int model_count = 2;
	const char *model_str[model_count] = {"data/cube.raw", "data/ship.raw"};	

	struct vertex_buffer vertex_buffer;
	struct vertex_buffer_handler model[model_count];


	const int cw = 100, ch = 100;
	const int instance_cube_count = ch*cw;
	struct vertex_instance instance_cube;

	struct cube_instance{
		struct mat4x4 translation;
		struct vec3 color;	
	};

	const int translation_offset = 0;
		
	const int instance_ship_count = 3;
	struct vertex_instance instance_ship;


	result = platform_initialize();
	if(result < 0)
	{
		fprintf(stderr, "Error: could not initialize platform.\n");
		exit(EXIT_FAILURE);
	}
	platform_update();


	result = vertex_buffer_model_load(&vertex_buffer, model, model_str, model_count);
	if(result < 0){
		fprintf(stderr, "Error: could not load vertex buffer data. ");	
		exit(EXIT_FAILURE);
	}
	
	const GLuint block_location = 0;
	view_initialize(&scene.view_state,  block_location, scene.frame_info.width, scene.frame_info.height);

	GLuint program[2]; 
	program[0] = create_program(vertex_shader_source, fragment_shader_source);
	program[1] = create_program(vertex_shader_source, fragment_shader_source);


	for(int i = 0; i < 2; i++)
	{

		camera_buffer_bind(&scene.view_state, program[i]);
	}



	vertex_instance_initialize(&instance_cube, &vertex_buffer, &model[0]);


	glBindVertexArray(instance_cube.vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, instance_cube.instance_buffer);

	GLuint translation_index = glGetAttribLocation(program[0], "r_model");
	translation_attribute_pointer(translation_index, sizeof(struct mat4x4), 0);

#if 0
	GLuint color_index = glGetAttribLocation(program[0], "r_color");
	glEnableVertexAttribArray(color_index);
	glVertexAttribPointer(color_index, 3, GL_FLOAT, GL_FALSE, sizeof(struct cube_instance), (const GLvoid*)(sizeof(struct mat4x4)));
	glVertexAttribDivisor(color_index, 1);
#endif
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	{
		struct mat4x4 translation[instance_cube_count];
		for(int i = 0; i < cw; i++){
			for(int j = 0; j < ch; j++){
				int k = i*ch + j;
				struct vec3 position = {i*3.0f, 0.0f, j*3.0f};
				translation[k] = m4x4trs(position);
			}
		}
		vertex_instance_update(instance_cube.instance_buffer, translation, instance_cube_count*sizeof(struct mat4x4));
	}


	vertex_instance_initialize(&instance_ship, &vertex_buffer, &model[1]);

	glBindVertexArray(instance_ship.vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, instance_ship.instance_buffer);

	translation_index = glGetAttribLocation(program[1], "r_model");
	translation_attribute_pointer(translation_index, sizeof(struct mat4x4), 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	{
		struct mat4x4 translation[instance_ship_count];
		for(int i = 0; i < instance_ship_count ; i++){
			struct vec3 position = {i*15.0f, 5.0f, 0.0f};
			translation[i] = m4x4trs(position);
		}
		vertex_instance_update(instance_ship.instance_buffer, translation, instance_ship_count*sizeof(struct mat4x4));
	}

	struct frame_info_update_result frame_result = frame_info_update(&scene.frame_info);
	int width = scene.frame_info.width;
	int height = scene.frame_info.height;
	

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	GLuint normal_texture;
	glGenTextures(1, &normal_texture);
	glBindTexture(GL_TEXTURE_2D, normal_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, normal_texture, 0);

	GLuint depth_texture;
#if 0
	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depth_texture, 0);
#else
	glGenRenderbuffers(1, &depth_texture);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_texture);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_texture);
#endif 
	

	GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, draw_buffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		fprintf(stderr, "Error: glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE \n");
	}

	
	const GLchar textured_quad_vertex_shader_source[] = 
	{
		"#version 330 core 					\n"
		" layout ( location = 0 ) in vec2 r_position; 		\n"
		" out vec2 f_uv;					\n"
		" void main(){						\n"
		"	f_uv = (r_position + vec2(1))/2;		\n"
		" 	gl_Position = vec4(r_position, 0.0, 1.0);       \n"
		"} 							\n"
	};


	const GLchar textured_quad_fragment_shader_source[] = {
		"#version 330 core 				\n"
		"uniform sampler2D in_texture;			\n"
		"out vec4 fcolor; 				\n"
		"in vec2 f_uv;					\n"
		"void main(){					\n"
		"	fcolor = texture(in_texture, f_uv);	\n"
		"}						\n"
	};
	
	const GLfloat quad_vertices[] = {
		-1.0f, 1.0f, 
		1.0f, 1.0f,
		1.0, -1.0f,
		-1.0f, -1.0f
	};	

	const GLuint quad_element[] = {
		1, 0, 2,
		2, 0, 3
	};

	GLuint textured_quad_program = create_program(textured_quad_vertex_shader_source, textured_quad_fragment_shader_source);
	
	GLuint quad_vertex_array, quad_vertex_buffer, quad_element_buffer;
	
	glGenVertexArrays(1, &quad_vertex_array);
	glBindVertexArray(quad_vertex_array);

	glGenBuffers(1, &quad_element_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof quad_element, quad_element, GL_STATIC_DRAW); 

	glGenBuffers(1, &quad_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof quad_vertices, quad_vertices, GL_STATIC_DRAW); 

	const GLuint quad_vertex_index = 0;
	glEnableVertexAttribArray(quad_vertex_index);
	glVertexAttribPointer(quad_vertex_index, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	glUseProgram(textured_quad_program);
	GLuint texture_sampler = glGetUniformLocation(textured_quad_program, "in_texture");
	glUniform1i(texture_sampler, 0);


	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	
	clock_t time = clock();
	while(!platform_exit())
    	{
		float deltatime = (float)(clock() - time)/(float)CLOCKS_PER_SEC;
		time = clock();
	
		struct frame_info_update_result frame_result = frame_info_update(&scene.frame_info);
		camera_view_projection(&scene.view_state, scene.frame_info.width, scene.frame_info.height);
		struct mat4x4 view = camera_input_update(&scene.view_state, 200.0f, frame_result.mouse_delta, deltatime);

		int new_width = scene.frame_info.width;
		int new_height = scene.frame_info.height;
		int new_window_size = (new_width != width) | (new_height != height);
		width = new_width;
		height = new_height;

		if(new_window_size)
		{

			glBindRenderbuffer(GL_RENDERBUFFER, depth_texture);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

			glBindTexture(GL_TEXTURE_2D, normal_texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

		}


		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	

		/* In render */	
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glViewport(0,0, width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		glUseProgram(program[0]);
		vertex_instance_draw(&instance_cube, instance_cube_count);

		glUseProgram(program[1]);
		vertex_instance_draw(&instance_ship, instance_ship_count);



		glBindFramebuffer(GL_FRAMEBUFFER, 0);
#if 1	

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, normal_texture);
		glBindSampler(0, depth_texture);

		glUseProgram(textured_quad_program);
		glBindVertexArray(quad_vertex_array);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_element_buffer);
		glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);	
		glBindVertexArray(0);
#endif 



		platform_update();
    	}
	
	printf( "Exiting");
	platform_deinitialize();
		
	return (result);
}
