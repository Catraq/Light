#include <time.h>
#include <stdint.h>

#include "platform.h"


#include "math/vec.h"
#include "math/mat4x4.h"

#include "camera.h"
#include "camera_input.h"


#include "model/raw_model.h"


#include "render_framebuffer.h"

#include "vertex_buffer.h"
#include "vertex_buffer_model.h"
#include "vertex_instance.h"

#include <joystick_ps3.h>

struct frame_info
{
	struct vec2 delta;
	struct vec2 mouse;

	uint32_t resize;
	uint32_t width;
	uint32_t height;
};

static struct frame_info frame_info_update(struct frame_info *prev)
{
	struct frame_info result = {};
	
	struct vec2 mouse;
	platform_mouse(&mouse);

	uint32_t width, height;
	platform_resolution(&width, &height);

	result.mouse = mouse;
	
	if(prev != NULL)
		result.resize = (width != prev->width || height != prev->height);

	result.width = width;
	result.height = height;
	
	return result;
}

static struct vec2 frame_info_mouse_delta(struct frame_info *curr, struct frame_info *prev)
{

	struct vec2 np = {prev->mouse.x/prev->width, -prev->mouse.y/prev->height};
	struct vec2 nc = {curr->mouse.x/curr->width, -curr->mouse.y/curr->height};
	struct vec2 delta = v2sub(np, nc); 
	
	return delta;

}


const char vertex_shader_source[] = 
{
	"#version 330 core 					\n"
	" layout ( location = 0 ) in vec3 r_position; 		\n"
	" layout ( location = 1 ) in vec3 r_normal;   		\n"
	" layout ( location = 2 ) in vec3 r_color; 		\n"
	" layout ( location = 3 ) in mat4 r_model; 		\n"
	" out vec3 v_normal; 					\n"
	" out vec3 v_position; 					\n"
	" uniform scene{ 					\n"
	"	mat4 view; 					\n"
	" };							\n"
	" void main(){						\n"
	" 	mat4 mn = transpose(inverse(r_model));		\n"
	" 	vec4 n = normalize(vec4(r_normal, 0.0));	\n"
	" 	vec4 m_n = normalize(mn * n);			\n"
	" 	v_normal = m_n.xyz;				\n"
	"   	vec4 w_position = r_model * vec4(r_position , 1.0f ); \n"	
	"	v_position = w_position.xyz;			\n"
	" 	gl_Position = view  * w_position; \n"
	"} 							\n"
};


const char fragment_shader_source[] = 
{
	"#version 330 core 				\n"
	"out vec4 fcolor; 				\n"
	"in vec3  v_normal;				\n"
	"in vec3 v_position;				\n"
	"layout(location=0) out vec3 normal_texture;	\n"
	"layout(location=1) out vec3 position_texture;	\n"
	"layout(location=2) out vec3 color_texture;	\n"
	"void main(){					\n"
	"	color_texture = vec3(0,0,1); 		\n"
	"	normal_texture = normalize(v_normal);	\n"
	"	position_texture = v_position;		\n"
	"}						\n"
};



		
	
GLint create_program(const char *vertex_source, const char *fragment_source)
{
	int result = 0;
	
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	
	
	const GLchar *vsource = (const GLchar *)vertex_source;
	const GLchar *fsource = (const GLchar *)fragment_source;

	glShaderSource(vertex_shader, 1, &vsource, 0);
	glShaderSource(fragment_shader, 1, &fsource, 0);
	
	glCompileShader(vertex_shader);
	glCompileShader(fragment_shader);

	
	GLint compiled = GL_FALSE;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE){
		GLchar log[255];
		GLsizei length;
		glGetShaderInfoLog(vertex_shader, 255, &length, (GLchar*)&log);
		if( length != 0 )
		{
			printf(" ---- Vertexshader compile log ---- \n %s \n", (GLchar*)&log);
		}

		result = -1;
	}

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE){
		GLchar log[255];
		GLsizei length;
		glGetShaderInfoLog(fragment_shader, 255, &length, (GLchar*)&log);
		if( length != 0 )
		{
			printf(" ---- Fragmentshader compile log ---- \n %s \n", (GLchar*)&log);
		}

		result = -1;
	}
	
	
	if(result < 0){
		/* Cleanup and return */	
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		return -1;
	}	
	
	GLuint program = glCreateProgram();

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
		printf(" ---- Program Link log ---- \n %s \n", (GLchar*)&log);

		result = -1;
	}

	if(result < 0){
		glDeleteProgram(program);
		return -1;
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
	struct camera_update_state update_state;

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
	
struct render_quad{
	GLuint program;
	GLuint vertex_array;
	GLuint vertex_buffer;
	GLuint element_buffer;
};

void quad_initialize(struct render_quad *quad, const char *vertex_shader_source, const char *fragment_shader_source)
{
	
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


	GLuint quad_program = create_program(vertex_shader_source, fragment_shader_source);

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

	quad->program = quad_program;
	quad->vertex_array = quad_vertex_array;
	quad->element_buffer = quad_element_buffer;
	quad->vertex_buffer = quad_vertex_buffer;

}

void quad_render(struct render_quad *quad)
{

	glUseProgram(quad->program);
	glBindVertexArray(quad->vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad->element_buffer);
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);	
	glBindVertexArray(0);

}

int main(int args, char *argv[])
{
	int result;


	struct frame_info 		frame_info;
	struct camera_view_state 	camera_view;
	struct camera_update_state 	camera_update;


	const int model_count = 2;
	const char *model_str[model_count] = {"data/cube.raw", "data/ship.raw"};	

	struct vertex_buffer vertex_buffer;
	struct vertex_buffer_handler model[model_count];

	struct render_framebuffer framebuffer;


	const int cw = 10, ch = 10, cd = 10;
	const int instance_cube_count = ch*cw*cd;
	struct vertex_instance instance_cube;

	const int translation_offset = 0;
		
	/* OpenGL Platform initialization */
	result = platform_initialize();
	if(result < 0)
	{
		fprintf(stderr, "Error: could not initialize platform.\n");
		exit(EXIT_FAILURE);
	}
	platform_update();
	
	/* Joystick initialization */
	result = camera_input_initialize(&camera_update, "/dev/input/js0");
	if(result < 0){
		fprintf(stderr, "Error: could not initialize input.\n");
		exit(EXIT_FAILURE);
	}



	/* Load 3D models */
	result = vertex_buffer_model_load(&vertex_buffer, model, model_str, model_count);
	if(result < 0){
		fprintf(stderr, "Error: could not load vertex buffer data. \n ");	
		exit(EXIT_FAILURE);
	}
	
	/* Get viewport size and so on */	
	frame_info = frame_info_update(NULL);
	int width = frame_info.width;
	int height = frame_info.height;

	/* Initlaize camera. */
	const GLuint block_location = 0;
	view_initialize(&camera_view, block_location, width, height);

	/* Render program for vertex buffer */
	GLint program = create_program(vertex_shader_source, fragment_shader_source);
	if(program < 0){
		fprintf(stderr, "Error: could not create shader. \n");	
		exit(EXIT_FAILURE);
	}
	
	camera_buffer_bind(&camera_view, program);

	vertex_instance_initialize(&instance_cube, &vertex_buffer, &model[1]);


	glBindVertexArray(instance_cube.vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, instance_cube.instance_buffer);

	GLuint translation_index = glGetAttribLocation(program, "r_model");
	translation_attribute_pointer(translation_index, sizeof(struct mat4x4), 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	{
		struct mat4x4 translation[instance_cube_count];
		for(int i = 0; i < cw; i++){
			for(int j = 0; j < ch; j++){
				for(int h = 0; h < cd; h++){
					int k = h + j * cd + i * cd * ch;
					struct vec3 position = {i*11.0f, h*5.0f, j*13.0f};
					translation[k] = m4x4trs(position);
				}
			}
		}
		vertex_instance_update(instance_cube.instance_buffer, translation, instance_cube_count*sizeof(struct mat4x4));
	}

	framebuffer_initialize(&framebuffer, width, height);
	
	
	
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

	
	struct render_quad quad;
	quad_initialize(&quad, textured_quad_vertex_shader_source, textured_quad_fragment_shader_source);



	glUseProgram(quad.program);
	GLuint texture_sampler = glGetUniformLocation(quad.program, "in_texture");
	glUniform1i(texture_sampler, 0);



	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	
	clock_t time = clock();
	while(!platform_exit())
    	{
		float deltatime = (float)(clock() - time)/(float)CLOCKS_PER_SEC;
		time = clock();
	
		struct frame_info frame_result = frame_info_update(&frame_info);
		struct vec2 delta_mouse = frame_info_mouse_delta(&frame_result, &frame_info);
		memcpy(&frame_info, &frame_result, sizeof frame_info);
		width = frame_result.width;
		height = frame_result.height;


		camera_view_projection(&camera_view, width, height);
		struct mat4x4 view = camera_input_update(&camera_update, &camera_view, 700.0f, delta_mouse, deltatime);


		if(frame_info.resize == 1)
		{
			framebuffer_resize(&framebuffer, width, height);
		}


		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	

		/* In render */	
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer);

		glViewport(0,0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		glUseProgram(program);
		vertex_instance_draw(&instance_cube, instance_cube_count);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);





		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.position_texture);
		glViewport(0,0, width/2, height/2);
		glBindSampler(0, framebuffer.position_texture);
		quad_render(&quad);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.normal_texture);
		glViewport(width/2, 0, width/2, height/2);
		glBindSampler(0, framebuffer.normal_texture);
		quad_render(&quad);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color_texture);
		glViewport(width/2, height/2, width/2, height/2);
		glBindSampler(0, framebuffer.color_texture);
		quad_render(&quad);


		platform_update();
    	}
	
	printf( "Exiting");
	platform_deinitialize();
		
	return (result);
}
