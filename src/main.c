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
#include "vertex_shader_program.c"

#include "light_frame.c"
#include "light_surface.c"

#include "scene.c"

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

	
		
	/* OpenGL Platform initialization */
	result = platform_initialize();
	if(result < 0)
	{
		fprintf(stderr, "Error: could not initialize platform.\n");
		exit(EXIT_FAILURE);
	}
	platform_update();



	struct vertex_buffer vertex_buffer;
	struct vertex_buffer_handler model[model_count];

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
		.fragment_shader_source = textured_light_surface_fragment_shader_source
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


	const int scene_object_count = model_count;
	struct scene_object scene_object[scene_object_count];



	/* 
	 * Initialize shader vertex attribute location for instances of scene object 
	 * thats using this shader. 
	 */	

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


	/* 
	 * Set the positons for the instances of the objectes in the scene. 
	 */
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
