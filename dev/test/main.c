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


#include "framebuffer.h"

#include "vertex_buffer.h"
#include "vertex_buffer_model.h"
#include "vertex_instance.h"
#include "vertex_instance_attribute.c"
#include "vertex_shader_program.c"

#include "frame.c"
#include "surface.c"

#include "scene.c"

#include "physic_intersect.c"
#include "physic_kd_tree.c"


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
	"	vec3 light_dir = normalize(vec3(10.0f, 100.0f, 0.0f) - v_normal);		\n"
	"	float diffuse = max(dot(v_normal, light_dir), 0.0);		\n"
	"	color_texture = vec3(1.0, 1.0, 1.0) * (0.1 + diffuse); 			\n"
	"	normal_texture = normalize(v_normal);	\n"
	"	position_texture = v_position;		\n"
	"}						\n"
};



		



#define SPHERE_COUNT 10
struct light_physic_shape_sphere shape_sphere[SPHERE_COUNT];
struct light_physic_body body[SPHERE_COUNT];



int main(int args, char *argv[])
{
	int result;


	for(uint32_t i = 0; i < SPHERE_COUNT; i++)
	{
		shape_sphere[i].radius = 1.0f;

		body[i] = (struct light_physic_body){
			.position = (struct vec3){.x = 0.0f, .z = 20.0f, .y = (float)(i+1) * 2.001f},
			.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
			.mass = 1.0f,
		};
	}

	
	/* OpenGL Platform initialization */
	result = light_platform_initialize();
	if(result < 0)
	{
		fprintf(stderr, "Error: could not initialize platform.\n");
		exit(EXIT_FAILURE);
	}
	light_platform_update();


	
	struct light_scene_instance scene;
	memset(&scene, 0, sizeof scene);

	const int model_count = 3;
	const char *model_str[model_count];
       	model_str[0] = "../data/sphere.raw";
       	model_str[1] = "../data/ship.raw";	
       	model_str[2] = "../data/cube.raw";	

	
	

	struct light_vertex_buffer vertex_buffer;
	struct light_vertex_buffer_handler model[model_count];

	/* Load 3D models to GPU memory.  */
	result = light_vertex_buffer_model_load(&vertex_buffer, model, model_str, model_count);
	if(result < 0){
		fprintf(stderr, "Error: could not load vertex buffer data. \n ");	
		exit(EXIT_FAILURE);
	}

	
	/*Create a quad as render surface */	
	struct light_surface quad_surface;
	result = light_surface_initialize(&quad_surface);
	if(result <0)
	{
		printf("light_surface_initialize(): Failed. \n");
		exit(EXIT_FAILURE);	
	}

	

	/* Initialize framebuffer */
	uint32_t frame_width = 512, frame_height = 512;
	struct light_framebuffer framebuffer;
	light_framebuffer_initialize(&framebuffer, frame_width, frame_height);


	/* Render program for vertex buffer */
	GLint program = light_create_program(vertex_shader_source, fragment_shader_source);
	if(program < 0){
		fprintf(stderr, "Error: could not create shader. \n");	
		exit(EXIT_FAILURE);
	}

	

	const int scene_object_count = model_count;
	struct light_scene_object scene_object[scene_object_count];

	/* 
	 * Initialize shader vertex attribute location for instances of scene object 
	 * thats using this shader. 
	 */	

	for(uint32_t i = 0; i < scene_object_count; i++)
	{	
		light_vertex_instance_initialize(&scene_object[i].object_instance, &vertex_buffer, &model[i]);

		scene_object[i].object_instance_count = 0;
		scene_object[i].object_program = program;
	
		const GLuint vertex_buffer_mat4x4_offset = 0;
		const char *model_transform_attribute = "r_model";
		if(light_vertex_instance_attribute(&scene_object[i].object_instance, vertex_buffer_mat4x4_offset, program, model_transform_attribute, VERTEX_INSTANCE_ATTRIBUTE_MAT4X4) < 0)
		{
			fprintf(stderr, "Error: attribute missing in shader. \n");	
			exit(EXIT_FAILURE);
		}

	}


	result = light_scene_initialize(&scene, scene_object, scene_object_count);

	/* 
	 * Set the positons for the instances of the objectes in the scene. 
	 */
	struct mat4x4 translation[SPHERE_COUNT];
	for(uint32_t i = 0; i < SPHERE_COUNT; i++)
	{
		translation[i] = m4x4trs(body[i].position);
	}

	scene_object[0].object_instance_count = SPHERE_COUNT;
	light_vertex_instance_update(scene_object[0].object_instance.instance_buffer, translation, sizeof(translation));

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	clock_t 	fps_sample_last		= clock();
	
	clock_t time = clock();
	while(!light_platform_exit())
    	{

		float deltatime = (float)(clock() - time)/(float)CLOCKS_PER_SEC;

		time = clock();

		fps_frame_count++;
		float fps_interval_time = (float)(clock() - fps_sample_last)/(float)CLOCKS_PER_SEC;
		if(fps_interval_time > fps_sample_interval)
		{
			uint32_t frames_per_sec = (float)fps_frame_count/fps_interval_time;
			fps_frame_count  = 0;
			fps_sample_last = clock();
			
			printf("5s FPS average: %u \n", frames_per_sec);

		}


		struct mat4x4 translation[SPHERE_COUNT];
		for(uint32_t i = 0; i < SPHERE_COUNT; i++)
		{
			translation[i] = m4x4trs(body[i].position);
		}
		scene_object[0].object_instance_count = SPHERE_COUNT;
		light_vertex_instance_update(scene_object[0].object_instance.instance_buffer, translation, sizeof(translation));





	
		uint32_t width, height;
		light_platform_resolution(&width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
	

		/* In render */	
		light_framebuffer_resize(&framebuffer, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer);

		light_scene_render(&scene, width, height, deltatime);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);

		light_surface_render(&quad_surface, framebuffer.color_texture);

		light_platform_update();
    	}
	
	printf( "Exiting");
	light_platform_deinitialize();
		
	return (result);
}
