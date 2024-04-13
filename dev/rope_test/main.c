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

#include <cblas.h>
#include <lapacke.h>

#include "physic/physic.c"
#include "physic/physic_rope.c"

#include "scene.c"
#include "scene/mesh.c"
#include "scene/plane.c"
#include "scene/particle_system.c"



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
	


	const uint32_t 			game_mesh_instance_count = 10;
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

		
	
	uint32_t body_count = game_mesh_instance_count;
	struct light_physic_particle *body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * body_count); 
	for(uint32_t i = 0; i < body_count; i++)
	{
		{
			body[i] = (struct light_physic_particle){
				.position = (struct vec3){.x = 2.0f*i, .y = 2.0f-2.0f*i, .z = 10.0f},
				.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
				.mass = 1.0f, 
				//.radius = 1.0f,
			};
		}
	}
	


	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	clock_t 	fps_sample_last		= clock();
	
	clock_t time = clock();


	int t = 1;
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

		struct vec3 gravity = (struct vec3){.x = 0.0f, .y = -9.82f, .z=0.0f};
		for(uint32_t i = 1; i < body_count; i++)
		{
			body[i].velocity = v3add(body[i].velocity, v3scl(gravity, deltatime));	
		}
	
		body[0].velocity = (struct vec3){.x = 0.0f, .y=0.0f, .z=10.0f};	
		body[0].position = (struct vec3){.x = 0.0f, .y=0.0f, .z=10.0f};	
		light_physic_rope(body, body_count);
	

		for(uint32_t i = 0; i < game_mesh_instance_count; i++)
		{
			body[i].position = v3add(body[i].position, v3scl(body[i].velocity, deltatime));
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
    	}
	
	printf( "Exiting");
	light_platform_deinitialize();
		
	return (result);
}
