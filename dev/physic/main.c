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
#include "scene/mesh.c"

#include "physic_intersect.c"
#include "physic_kd_tree.c"

#include "physic/physic.c"


int main(int args, char *argv[])
{
	int result;

	
	/* OpenGL Platform initialization */
	result = light_platform_initialize();
	if(result < 0)
	{
		fprintf(stderr, "Error: could not initialize platform.\n");
		exit(EXIT_FAILURE);
	}
	light_platform_update();


	
	
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
	
	struct light_scene_instance scene;
	result = light_scene_initialize(&scene);
	if(result < 0)
	{
		printf("light_scene_initialize(): Failed. \n");
		exit(EXIT_FAILURE);
	}


	


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

	const uint32_t 			game_mesh_instance_count = 10*10+1;
	struct light_game_mesh 		game_mesh;
	struct light_game_mesh_instance game_mesh_instance[game_mesh_instance_count];

	result = light_game_mesh_init(&scene, &game_mesh, &vertex_buffer, &model[0]);
	if(result < 0)
	{
		printf("light_game_mesh_init(): Failed. \n");
		exit(EXIT_FAILURE);
	}

	result = light_game_mesh_instance_init(&game_mesh, game_mesh_instance, game_mesh_instance_count);
       	if(result < 0)
	{
		printf("light_game_mesh_instance_init(): Failed. \n");
		exit(EXIT_FAILURE);
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	
	uint32_t body_count = 10*10+1;
	struct light_physic_sphere_body *body = (struct light_physic_sphere_body *)malloc(sizeof(struct light_physic_sphere_body) * body_count); 
	struct light_physic_sphere_body_pair *pair = (struct light_physic_sphere_body_pair *)malloc(sizeof(struct light_physic_sphere_body_pair) * body_count*(body_count - 1)/2);  
       	float *d = (float *)malloc(sizeof(float) * body_count);
	
	for(uint32_t i = 0; i < 10; i++)
	{
		for(uint32_t j = 0; j < 10; j++)
		{
			body[i*10 + j] = (struct light_physic_sphere_body){
				.position = (struct vec3){.x = i*2.1f - 10.0f, .y = j*2.1f-10.0f, .z = 20.0f},
				.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0},
				.mass = 1.0f, 
				.radius = 1.0f,
			};
		}
	}


	body[100] = (struct light_physic_sphere_body){
		.position = (struct vec3){.x = 20.0f, .y = 20.0f, .z = 20.0f},
		.velocity = (struct vec3){.x = -2.0f, .y = -2.0f, .z = 0},
		.mass = 1.0f, 
		.radius = 1.0f,
	};
			






	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	clock_t 	fps_sample_last		= clock();
	
	float deltatime = 0.0f;
		
	clock_t time = clock();
	while(!light_platform_exit())
    	{


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



		light_physic_update(body, pair, d, body_count, deltatime);
		for(uint32_t i = 0; i < game_mesh_instance_count; i++)
		{
			struct vec3 p = body[i].position;
			game_mesh_instance[i].translation = m4x4trs(p);
		}

		light_game_mesh_instance_commit(&game_mesh, game_mesh_instance, game_mesh_instance_count);


	
		uint32_t width, height;
		light_platform_resolution(&width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
	

		/* In render */	
		light_framebuffer_resize(&framebuffer, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer);

		light_scene_update(&scene, width, height, deltatime);
		light_game_mesh_render(&game_mesh, game_mesh_instance, game_mesh_instance_count);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);

		light_surface_render(&quad_surface, framebuffer.color_texture);

		light_platform_update();

		deltatime = (float)(clock() - time)/(float)CLOCKS_PER_SEC;
    	}
	
	printf( "Exiting");
	light_platform_deinitialize();
		
	return (result);
}
