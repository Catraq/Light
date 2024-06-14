#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <sys/time.h>

#include "math/vec.h"
#include "math/mat4x4.h"
#include "platform.h"

#include "scene.h"
#include "scene/state.h"
#include "scene/object.h"
#include "scene/implicit.h"
#include "scene/particle.h"
#include "scene/particle_emitter.h"

#include "misc/file.h"


#include "camera.h"
#include "camera_input.h"


#include "framebuffer.h"
#include "frame.h"
#include "surface.h"
#include "error.h"

#include "physic/physic.c"
#include "physic/physic_rope.c"

GLuint light_shader_compute_create(const GLchar **source, GLint *length, uint32_t count)
{
	GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
	if(compute_shader == 0)
	{
		fprintf(stderr, "glCreateShader(GL_COMPUTE_SHADER) failed.");
		return -1;
	}

	glShaderSource(compute_shader, count, source , length);
	glCompileShader(compute_shader);
	
	GLint compiled = GL_FALSE;
	glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE){

		GLchar log[8192];
		GLsizei length;
		glGetShaderInfoLog(compute_shader, 8192, &length, log);
		if( length != 0 )
		{
			fprintf(stderr, " ---- Compilation of shader failed.  ---- \n %s \n", log);
		}

		glDeleteShader(compute_shader);

		return 0;
	}
	
	GLuint compute_program = glCreateProgram();
	glAttachShader(compute_program, compute_shader);
	glLinkProgram(compute_program);
	
	glDeleteShader(compute_shader);
	
	GLint linked = GL_FALSE;
	glGetProgramiv(compute_program, GL_LINK_STATUS, &linked);
	if(linked == GL_FALSE) {
		GLsizei length;
		GLchar log[8192];
		glGetProgramInfoLog(compute_program, 8192, &length, log);
		fprintf(stderr, " ---- Linking of program failed ---- \n %s \n", log);

		glDeleteProgram(compute_program);
		return 0;
	}


	return compute_program;
}



int main(int args, char *argv[])
{
	int result;

	
	/* opengl platform initialization */
	result = light_platform_initialize();
	if(result < 0)
	{
		fprintf(stderr, "error: could not initialize platform.\n");
		exit(EXIT_FAILURE);
	}
	light_platform_update();


	
	
	/*create a quad for rendering textures */
	struct light_surface_textured quad_surface;
	result = light_surface_textured_initialize(&quad_surface);
	if(result <0)
	{
		printf("light_surface_textured_initialize(): failed. \n");
		exit(EXIT_FAILURE);	
	}
	CHECK_GL_ERROR();

	/* Initialize scene */	
	struct light_scene_instance scene;
	result = light_scene_initialize(&scene);
	if(result < 0)
	{
		printf("light_scene_initialize(): failed. \n");
		exit(EXIT_FAILURE);
	}

	CHECK_GL_ERROR();
	
	/* 
	 * The values are used as defines in the shader
	 * and have to be one or greater. 
	 */	
	struct light_scene_state_build state_build = {

		.implicit_build = {
			.object_count = 4, 		//Maximum number of objects 
			.object_node_count = 8,		//Maximum number of node, that objects are made of
			.object_node_max_level = 1,	//Objects are trees, max number of levels of the tree.
			.sphere_count 	= 1,		//Number of sphere nodes 
			.box_count 	= 2,
			.cylinder_count = 1,
			.light_count 	= 2,		//Number of lights
		},
		/* 
		 * A particle emitter owns a range of emitters, first 0 to 1, 2:nd 2 to 5 
		 * and so on. 
		 */
		.particle_build = {
			.emitter_count = 8,
			.emitter_particle_count = 64,	
		},
		/* Particle emitter that owns a range. The normal indicates 
		 * that the particles are spread as normal distrobuted, that
		 * is uniform distrobution for each particle that is normal
		 * by central limit theorem. 
		 */
		.particle_emitter_build  = {
			.emitter_normal_count = 2,
		},
	};

	struct light_scene_state_instance state_instance;
	result = light_scene_state_initialize(&state_instance, state_build);
	if(result < 0){
		printf("light_scene_state_initialize(): failed. \n");
		exit(EXIT_FAILURE);
	}
	
	/* Light is required otherwise everything will be black */	
	const uint32_t light_count = 2;
	struct light_scene_light_light_instance light_instance[light_count];
	light_instance[0].position = (struct vec3){.x=0.0f, 2.0f, 10.0};
	light_instance[0].color = (struct vec3){.x=0.0, .y=1.0f, .z=1.0f};

	light_instance[1].position = (struct vec3){.x=0.0f, 2.0f, -10.0};
	light_instance[1].color = (struct vec3){.x=1.0, .y=1.0f, .z=0.0f};

	light_scene_implicit_commit_light(&state_instance, light_instance, light_count);

	const uint32_t sphere_count = 1;
	struct light_scene_implicit_sphere_instance sphere_instance[sphere_count];
	sphere_instance[0].radius = 1.0f;
	sphere_instance[0].color = (struct vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f};

	
	const uint32_t cylinder_count = 1;
	struct light_scene_implicit_cylinder_instance cylinder_instance[cylinder_count];

	cylinder_instance[0].radius = 1.0f;
	cylinder_instance[0].height = 1.0f;
	cylinder_instance[0].color = (struct vec3){.x = 1.0f, .y = 1.0f, .z = 0.0f};


	/* Something as the floor */
	const uint32_t box_count = 2;
	struct light_scene_implicit_box_instance box_instance[box_count];
	box_instance[0].dimension = (struct vec3){.x = 40.0f, .y = 0.1f, .z = 40.0f};
	box_instance[0].color = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 1.0f};

	box_instance[1].dimension = (struct vec3){.x = 10.0f, .y = 1.0f, .z = 10.0f};
	box_instance[1].color = (struct vec3){.x = 1.0f, .y = 0.0f, .z = 1.0f};

	light_scene_implicit_commit_sphere(&state_instance, sphere_instance, sphere_count);
	light_scene_implicit_commit_cylinder(&state_instance, cylinder_instance, cylinder_count);
	light_scene_implicit_commit_box(&state_instance, box_instance, box_count);
	light_scene_implicit_commit_light(&state_instance, light_instance, light_count);

	/* Create the floor object */
	struct light_scene_implicit_object_instance object_instance[4];
	struct light_scene_implicit_object_node object_node[8];
	
	{
		struct vec3 center = {.x=0, .y=0, .z=0};
		object_node[0].translation = m4x4trs(center);
		object_node[0].translation_inv = m4x4inv(&object_node[0].translation, &result);
		object_node[0].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_node[0].object_index = 2;

		struct vec3 box_p= {.x=-20, .y=-1.0, .z=-20};
		object_instance[0].translation = m4x4trs(box_p);
		object_instance[0].translation_inv = m4x4inv(&object_instance[0].translation, &result); 
		object_instance[0].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_instance[0].index_left = 0;
		object_instance[0].index_right = 0;
		object_instance[0].levels = 1;
	}
	
	{
		struct vec3 center = {.x=0, .y=0, .z=0};
		object_node[1].translation = m4x4trs(center);
		object_node[1].translation_inv = m4x4inv(&object_node[1].translation, &result);
		object_node[1].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_node[1].object_index = 1;

		struct vec3 box_p= {.x=2.0, .y=0.0, .z=2.0};
		object_instance[1].translation = m4x4trs(box_p);
		object_instance[1].translation_inv = m4x4inv(&object_instance[1].translation, &result); 
		object_instance[1].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_instance[1].index_left = 1;
		object_instance[1].index_right = 1;
		object_instance[1].levels = 1;
	}

	{
		struct vec3 center = {.x=0, .y=0, .z=0};
		object_node[2].translation = m4x4trs(center);
		object_node[2].translation_inv = m4x4inv(&object_node[2].translation, &result);
		object_node[2].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_node[2].object_index = 0;

		struct vec3 box_p= {.x=2, .y=0, .z=5};
		object_instance[2].translation = m4x4trs(box_p);
		object_instance[2].translation_inv = m4x4inv(&object_instance[2].translation, &result); 
		object_instance[2].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_instance[2].index_left = 2;
		object_instance[2].index_right = 2;
		object_instance[2].levels = 1;
	}
	{
		struct vec3 center = {.x=0, .y=0, .z=0};
		object_node[3].translation = m4x4trs(center);
		object_node[3].translation_inv = m4x4inv(&object_node[2].translation, &result);
		object_node[3].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_node[3].object_index = 0;

		struct vec3 box_p= {.x=2, .y=0, .z=5};
		object_instance[2].translation = m4x4trs(box_p);
		object_instance[2].translation_inv = m4x4inv(&object_instance[2].translation, &result); 
		object_instance[2].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_instance[2].index_left = 2;
		object_instance[2].index_right = 2;
		object_instance[2].levels = 1;
	}



	light_scene_implicit_commit_objects(&state_instance, object_instance, 3, object_node, 3);
	CHECK_GL_ERROR();

	struct light_scene_particle_emitter_normal particle_emitter_normal[2];

       	particle_emitter_normal[0] = (struct light_scene_particle_emitter_normal){
		.position = (struct vec3){.x=0.0, .y=0.0, .z=2.0},
		.lifetime_expected = 5,
		.velocity_expected = (struct vec3){.x=1.0, .y=0.0, .z=0.0},
		.lifetime_variance = 5,	
		.velocity_variance = (struct vec3){.x=1, .y=0.1, .z=0.1},
		.emitter_offset	= 0,
		.emitter_count = 4
	};

       	particle_emitter_normal[1] = (struct light_scene_particle_emitter_normal){
		.position = (struct vec3){.x=0.0, .y=0.0, .z=5.0},
		.lifetime_expected = 5,
		.velocity_expected = (struct vec3){.x=1.0, .y=0.0, .z=0.0},
		.lifetime_variance = 5,	
		.velocity_variance = (struct vec3){.x=0.5, .y=0.1, .z=0.1},
		.emitter_offset	= 4,
		.emitter_count = 4
	};

	light_scene_particle_emitter_commit_normal(
		&state_instance, 
		particle_emitter_normal,
		2
	);




	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	float 		fps_time;
	struct timeval 	fps_time_last;
	struct timeval 	fps_time_curr;

	gettimeofday(&fps_time_last, NULL);
	fps_time_curr = fps_time_last;

	light_scene_state_bind(&state_instance);
	while(!light_platform_exit())
    	{
		struct timeval fps_time_curr_tmp;
		gettimeofday(&fps_time_curr_tmp, NULL);	
		float deltatime = fps_time_curr_tmp.tv_sec - fps_time_curr.tv_sec + (float)(fps_time_curr_tmp.tv_usec - fps_time_curr.tv_usec)/(1000.0f*1000.0f);
		fps_time_curr = fps_time_curr_tmp;
		fps_time = fps_time + deltatime;

		fps_frame_count++;
		if(fps_time> fps_sample_interval)
		{
			uint32_t frames_per_sec = (float)fps_frame_count/fps_time;
			gettimeofday(&fps_time_last, NULL);

			printf("5s fps average(%u frames): %u \n", fps_frame_count, frames_per_sec);

			fps_time = 0.0f;
			fps_frame_count  = 0;

		}
		
		int width=128, height=128;		
		light_scene_bind(&scene, width, height, deltatime);
		light_scene_state_dispatch(&state_instance, &scene.framebuffer, width, height, deltatime);


		light_platform_resolution(&width, &height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, width, height);
		glDisable(GL_DEPTH_TEST);

		light_surface_textured_render(&quad_surface, scene.framebuffer.color_texture);
		
		light_platform_update();
		glError("main loop:");
    	}
		
	light_scene_deinitialize(&scene);




	printf( "exiting");
	light_platform_deinitialize();
		
	return (result);
}
