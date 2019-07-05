#include <time.h>
#include <stdint.h>


#include "scene.h"


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
	" out vec3 normal; 					\n"
	" uniform scene{ 					\n"
	"	mat4 view; 					\n"
	" };							\n"
	" void main(){						\n"
	" 	normal = r_color;				\n"
	" 	gl_Position = view  * r_model *vec4(r_position , 1.0f );        \n"
	"} 							\n"
};

const char fragment_shader_source[] = 
{
	"#version 330 core 				\n"
	"out vec4 fcolor; 				\n"
	"in vec3  normal;				\n"
	"void main(){					\n"
	"	fcolor = vec4(normal,0);		\n"
	"}								\n"
};


const char vertex_shader_2_source[] = 
{
	"#version 330 core 					\n"
	" layout ( location = 0 ) in vec3 r_position; 		\n"
	" layout ( location = 1 ) in vec3 r_normal;   		\n"
	" layout ( location = 2 ) in mat4 r_model; 		\n"
	" out vec3 normal; 					\n"
	" uniform scene{ 					\n"
	"	mat4 view; 					\n"
	" };							\n"
	" void main(){						\n"
	" 	gl_Position = view  * r_model *vec4(r_position , 1.0);        \n"
	"	vec4 tn = view*r_model * vec4(r_normal, 0.0);"
	"	vec3 light = vec3(0.0, 10.0, -10.0);		\n"
	"	vec3 delta = light - gl_Position.xyz;		\n"
	"	float distance = length(delta); 		\n"
	"	vec3 n = delta/distance;			\n"
	"	float angle = dot(n,tn.xyz);			\n"
	"	if(angle > 0.0){				\n"
	" 		normal = (distance*distance)*vec3(1.0,0.0,0.0);		\n"
	"	}else{						\n"
	"		normal = vec3(0.0, 0.0, 1.0);		\n"
	"	}						\n"
	"} 							\n"
};

const char fragment_shader_2_source[] = 
{
	"#version 330 core 				\n"
	"out vec4 fcolor; 				\n"
	"in vec3  normal;				\n"
	"void main(){					\n"
	"	fcolor = vec4(normal,0.0);		\n"
	"}								\n"
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


#if 0
struct vertex_instance
{
	GLuint vertex_array;

	GLuint instance_buffer;

	GLuint indice_count;
	GLuint indice_offset;
};

#ifndef VERTEX_INSTANCE_H
#define VERTEX_INSTANCE_H
void vertex_instance_initialize(struct vertex_instance *instance, struct vertex_buffer *buffer, struct vertex_buffer_handler *handler)
{
	glGenBuffers(1, &instance->instance_buffer);
	glGenVertexArrays(1, &instance->vertex_array);

	glBindVertexArray(instance->vertex_array);
	vertex_buffer_attribute_pointer(buffer, handler);
	glBindVertexArray(0);


	instance->indice_count = handler->indice_count;
	instance->indice_offset = handler->indice_offset;

}
void vertex_instance_draw(struct vertex_instance *instance, GLuint instance_count)
{
	GLuint indice_count 	= instance->indice_count;
	GLuint indice_offset 	= instance->indice_offset;
	GLuint vertexarray 	= instance->vertex_array;
	GLuint instance_buffer 	= instance->instance_buffer;

	glBindVertexArray(vertexarray);
	glDrawElementsInstanced(GL_TRIANGLES, 
		  indice_count,
		  GL_UNSIGNED_INT, 
		  (const void*)(indice_offset), 
		  instance_count
	);
}

void vertex_instance_update(GLuint buffer, void *src, GLuint src_len)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, src_len, src, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#endif //VERTEX_INSTANCE_H
#endif

int main(int args, char *argv[])
{
	int result;
	struct scene_instance scene;

	const int model_count = 2;
	const char *model_str[model_count] = {"data/cube.raw", "data/ship.raw"};	

	struct vertex_buffer vertex_buffer;
	struct vertex_buffer_handler model[model_count];


	const int cw = 10, ch = 10;	
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



	result = vertex_buffer_model_load(&vertex_buffer, model, model_str, model_count);
	if(result < 0){
		fprintf(stderr, "Error: could not load vertex buffer data. ");	
		exit(EXIT_FAILURE);
	}
	
	const GLuint block_location = 0;
	view_initialize(&scene.view_state,  block_location, scene.frame_info.width, scene.frame_info.height);

	GLuint program[2]; 
	program[0] = create_program(vertex_shader_source, fragment_shader_source);
	program[1] = create_program(vertex_shader_2_source, fragment_shader_2_source);


	for(int i = 0; i < 2; i++)
	{

		camera_buffer_bind(&scene.view_state, program[i]);
	}



	vertex_instance_initialize(&instance_cube, &vertex_buffer, &model[0]);


	glBindVertexArray(instance_cube.vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, instance_cube.instance_buffer);

	GLuint translation_index = glGetAttribLocation(program[0], "r_model");
	translation_attribute_pointer(translation_index, sizeof(struct cube_instance), translation_offset);

	GLuint color_index = glGetAttribLocation(program[0], "r_color");
	glEnableVertexAttribArray(color_index);
	glVertexAttribPointer(color_index, 3, GL_FLOAT, GL_FALSE, sizeof(struct cube_instance), (const GLvoid*)(sizeof(struct mat4x4)));
	glVertexAttribDivisor(color_index, 1);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	{
		struct cube_instance instance[instance_cube_count];
		for(int i = 0; i < cw; i++){
			for(int j = 0; j < ch; j++){
				int k = i*ch + j;
				struct vec3 position = {i*3.0f, 0.0f, j*3.0f};
				instance[k].translation = m4x4trs(position);
				float randrr = (float)(rand()%255)/255.0f;
				float randrg = (float)(rand()%255)/255.0f;
				float randrb = (float)(rand()%255)/255.0f;
				instance[k].color = {randrr, randrg, randrb}; 
			}
		}
		vertex_instance_update(instance_cube.instance_buffer, instance, instance_cube_count*sizeof(struct cube_instance));
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



	

	
	clock_t time = clock();
	while(!platform_exit())
    	{
		float deltatime = (float)(clock() - time)/(float)CLOCKS_PER_SEC;
		time = clock();
	
		struct frame_info_update_result frame_result = frame_info_update(&scene.frame_info);
		camera_view_projection(&scene.view_state, scene.frame_info.width, scene.frame_info.height);
		struct mat4x4 view = camera_input_update(&scene.view_state, 200.0f, frame_result.mouse_delta, deltatime);

		glViewport(0, 0, scene.frame_info.width, scene.frame_info.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
		
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		glUseProgram(program[0]);
		vertex_instance_draw(&instance_cube, instance_cube_count);

		glUseProgram(program[1]);
		vertex_instance_draw(&instance_ship, instance_ship_count);



		glBindVertexArray(0);
		platform_update();
    	}
	
	printf( "Exiting");
	platform_deinitialize();
		
	return (result);
}
