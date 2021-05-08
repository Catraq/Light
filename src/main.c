#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "math/vec.h"
#include "math/mat4x4.h"
#include "platform.h"

#include "camera.h"
#include "camera_input.h"


#include "model/raw_model.h"


#include "render_framebuffer.h"

#include "vertex_buffer.h"
#include "vertex_buffer_model.h"
#include "vertex_instance.h"
#include "vertex_instance_attribute.c"

#include "light_frame.c"

#if 0
#include <joystick_ps3.h>
#endif 

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
	"	color_texture = vec3(1.0, 0.0, 0.0); 		\n"
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


#include "light_surface.c"


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
	view_state->position = (struct vec3){0.0f, 0.0f, 0.0f};
	view_state->rotation = (struct vec3){0.0f, 0.0f, 0.0f};

}


struct scene_object
{
	GLuint 			object_program;
	uint32_t 		object_instance_count;
	struct vertex_instance 	object_instance;
};

struct scene_instance 
{

	struct frame_info frame_info;
	struct camera_view_state view_state;
	struct camera_update_state update_state;

		
	uint32_t object_count;
	struct scene_object *object;

};

int scene_initialize(struct scene_instance *instance, struct scene_object *object, uint32_t object_count)
{
	int result = 0;

	/* Platform Joystick initialization */
	result = camera_input_initialize(&instance->update_state);
	if(result < 0){
		fprintf(stderr, "Error: could not initialize camera input.\n");
		return -1;	
	}


	/* Initlaize camera. */
	const GLuint uniform_buffer_block_location = 0;

	struct frame_info frame_info = frame_info_update(NULL);
	view_initialize(&instance->view_state, uniform_buffer_block_location, frame_info.width, frame_info.height);

	instance->object = object;
	instance->object_count = object_count;
	
	for(uint32_t i = 0; i < instance->object_count; i++)
	{
		camera_buffer_bind(&instance->view_state, instance->object[i].object_program);
	}
		

	return 0;
}

int scene_render(struct scene_instance *instance, uint32_t width, uint32_t height, const float deltatime)
{	
	struct frame_info frame_result = frame_info_update(&instance->frame_info);
	struct vec2 delta_mouse = frame_info_mouse_delta(&frame_result, &instance->frame_info);
	instance->frame_info = frame_result; 

	camera_input_update(&instance->update_state, &instance->view_state, 10.0f, delta_mouse, deltatime);




	camera_view_matrix(&instance->view_state, width, height);

	glViewport(0,0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	

	for(uint32_t i = 0; i < instance->object_count; i++)
	{
		if(instance->object[i].object_instance_count > 0)
		{
			glUseProgram(instance->object[i].object_program);
			vertex_instance_draw(&instance->object[i].object_instance, instance->object[i].object_instance_count);
		}

	}

	return 0;
}




int main(int args, char *argv[])
{
	int result;

	
	struct scene_instance scene;
	memset(&scene, 0, sizeof scene);

#if 1
	const int model_count = 2;
	const char *model_str[model_count];
       	model_str[0] = "data/cube.raw";
       	model_str[1] = "data/ship.raw";	
#endif 

	const int scene_object_count = model_count;
	struct scene_object scene_object[scene_object_count];

	struct vertex_buffer vertex_buffer;
	struct vertex_buffer_handler model[model_count];


		
	/* OpenGL Platform initialization */
	result = platform_initialize();
	if(result < 0)
	{
		fprintf(stderr, "Error: could not initialize platform.\n");
		exit(EXIT_FAILURE);
	}
	platform_update();



	/* Load 3D models to GPU memory.  */
	result = vertex_buffer_model_load(&vertex_buffer, model, model_str, model_count);
	if(result < 0){
		fprintf(stderr, "Error: could not load vertex buffer data. \n ");	
		exit(EXIT_FAILURE);
	}

	
	/*Create a quad as render surface */	
	struct light_surface quad_surface;
	struct light_surface_config quad_config = {
		.vertices = light_surface_quad_vertices,
		.vertices_size = sizeof(light_surface_quad_vertices),

		.indices = light_surface_quad_indices,
		.indices_size = sizeof(light_surface_quad_indices),

		.vertex_shader_source = textured_light_surface_vertex_shader_source,
		.fragment_shader_source =textured_light_surface_fragment_shader_source
	};
	
	const GLuint SAMPLER_INDEX = 0;
	light_surface_initialize(&quad_surface, &quad_config);
	light_surface_texture(&quad_surface, "in_texture", SAMPLER_INDEX);

	

	/* Initialize framebuffer */
	uint32_t frame_width = 512, frame_height = 512;
	struct render_framebuffer framebuffer;
	framebuffer_initialize(&framebuffer, frame_width, frame_height);


	/* Render program for vertex buffer */
	GLint program = create_program(vertex_shader_source, fragment_shader_source);
	if(program < 0){
		fprintf(stderr, "Error: could not create shader. \n");	
		exit(EXIT_FAILURE);
	}


		
	for(uint32_t i = 0; i < scene_object_count; i++)
	{	
		vertex_instance_initialize(&scene_object[i].object_instance, &vertex_buffer, &model[i]);

		scene_object[i].object_instance_count = 0;
		scene_object[i].object_program = program;
	
		const GLuint vertex_buffer_mat4x4_offset = 0;
		const char *model_transform_attribute = "r_model";
		if(vertex_instance_attribute_mat4x4(&scene_object[i].object_instance, vertex_buffer_mat4x4_offset, program, model_transform_attribute, VERTEX_INSTANCE_ATTRIBUTE_MAT4X4) < 0)
		{
			fprintf(stderr, "Error: attribute missing in shader. \n");	
			exit(EXIT_FAILURE);
		}

	}


	result = scene_initialize(&scene, scene_object, scene_object_count);

	{
		struct mat4x4 translation;
		struct vec3 position = {.x = 0.0f, .y =  0.0f, .z = 20.0f};
		translation = m4x4trs(position);
		vertex_instance_update(scene_object[0].object_instance.instance_buffer, &translation, sizeof(translation));
		scene_object[0].object_instance_count = 1;
	}


	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	clock_t 	fps_sample_last		= clock();
	
	clock_t time = clock();

	while(!platform_exit())
    	{
		fps_frame_count++;
		float fps_interval_time = (float)(clock() - fps_sample_last)/(float)CLOCKS_PER_SEC;
		if(fps_interval_time > fps_sample_interval)
		{
			uint32_t frames_per_sec = (float)fps_frame_count/fps_interval_time;
			fps_frame_count  = 0;
			fps_sample_last = clock();
			
			printf("5s FPS average: %u \n", frames_per_sec);

		}


		float deltatime = (float)(clock() - time)/(float)CLOCKS_PER_SEC;
		time = clock();


	
		uint32_t width, height;
		platform_resolution(&width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	

		/* In render */	
		framebuffer_resize(&framebuffer, frame_width, frame_height);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer);

		scene_render(&scene, frame_width, frame_height, deltatime);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);



#if 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.position_texture);
		glViewport(0,0, width/2, height/2);
		glBindSampler(0, framebuffer.position_texture);
		light_surface_render(&quad_surface);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.normal_texture);
		glViewport(width/2, 0, width/2, height/2);
		glBindSampler(0, framebuffer.normal_texture);

		light_surface_render(&quad_surface);
#endif 

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color_texture);
		glViewport(0, 0, width, height);
		glBindSampler(0, framebuffer.color_texture);
		light_surface_render(&quad_surface);

		platform_update();
    	}
	
	printf( "Exiting");
	platform_deinitialize();
		
	return (result);
}
