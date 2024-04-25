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
#include "scene/object.h"

#include "misc/file.h"


#include "camera.h"
#include "camera_input.h"


#include "framebuffer.h"
#include "frame.h"
#include "surface.c"

#include "physic/physic.c"
#include "physic/physic_rope.c"


GLuint light_shader_compute_create(const GLchar **source, GLint *length)
{
	GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
	if(compute_shader == 0)
	{
		fprintf(stderr, "glCreateShader(GL_COMPUTE_SHADER) failed.");
		return -1;
	}

	glShaderSource(compute_shader, 1, source , length);
	glCompileShader(compute_shader);
	
	GLint compiled = GL_FALSE;
	glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE){

		GLchar log[8192];
		GLsizei length;
		glGetShaderInfoLog(compute_shader, 8192, &length, (GLchar*)&log);
		if( length != 0 )
		{
			fprintf(stderr, " ---- Compilation of shader failed.  ---- \n %s \n", (GLchar*)&log);
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
		glGetProgramInfoLog(compute_program, 8192, &length, (GLchar*)&log);
		fprintf(stderr, " ---- Linking of program failed ---- \n %s \n", (GLchar*)&log);

		glDeleteProgram(compute_program);
		return 0;
	}


	return compute_program;
}



struct light_implicit_instance
{
	GLuint compute_program;
	
	GLuint prepass_compute_program;	
	GLuint prepass_texture;
};


int light_implicit_initialize(struct light_scene_instance *scene, struct light_implicit_instance *instance)
{
	{
		const char *compute_shader_filename = "../data/implicit.txt";
		uint8_t compute_shader_source[8192];
		size_t read = light_file_read_buffer(compute_shader_filename, compute_shader_source, 8192);
		if(read == 0){
			fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", compute_shader_filename);
			return -1;
		}

		const GLchar *source = compute_shader_source;
		GLint length = read;

		GLuint compute_program = light_shader_compute_create(&source, &length);
		if(compute_program == 0)
		{
			fprintf(stderr, "light_shader_compute_create() failed.");
			return -1;
		}

		int result = light_scene_bind_program(scene, compute_program);
		if(result < 0)
		{
			glDeleteProgram(compute_program);

			fprintf(stderr, "light_scene_bind() failed. \n");	
			return -1;
		}
		
		
		instance->compute_program = compute_program;
	}
	
	{
		const char *compute_shader_filename = "../data/implicit_pre.txt";
		uint8_t compute_shader_source[8192];
		size_t read = light_file_read_buffer(compute_shader_filename, compute_shader_source, 8192);
		if(read == 0){

			glDeleteProgram(instance->compute_program);

			fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", compute_shader_filename);
			return -1;
		}

		const GLchar *source = compute_shader_source;
		GLint length = read;

		GLuint compute_program = light_shader_compute_create(&source, &length);
		if(compute_program == 0)
		{

			glDeleteProgram(instance->compute_program);

			fprintf(stderr, "light_shader_compute_create() failed.");
			return -1;
		}

		int result = light_scene_bind_program(scene, compute_program);
		if(result < 0)
		{

			glDeleteProgram(instance->compute_program);
			glDeleteProgram(compute_program);

			fprintf(stderr, "light_scene_bind() failed. \n");	
			return -1;
		}
		
		
		instance->prepass_compute_program = compute_program;

	}

	GLuint prepass_texture;
	glGenTextures(1, &prepass_texture);
	glBindTexture(GL_TEXTURE_2D, prepass_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	instance->prepass_texture = prepass_texture;

	return 0;
}

void light_implicit_dispatch(struct light_implicit_instance *instance, uint32_t width, uint32_t height)
{

	glBindTexture(GL_TEXTURE_2D, instance->prepass_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width/4, height/4, 0, GL_RED, GL_FLOAT, NULL);
	glBindImageTexture(4, instance->prepass_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);


	glUseProgram(instance->prepass_compute_program);
	glDispatchCompute(width/4, height/4, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


	glUseProgram(instance->compute_program);
	glDispatchCompute(width, height, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

}

void light_implicit_deinitialize(struct light_implicit_instance *implicit)
{
	glDeleteProgram(implicit->compute_program);
}

struct light_light_instance
{
	GLuint compute_program;

};

int light_light_initialize(struct light_scene_instance *scene, struct light_light_instance *instance)
{
	const char *compute_shader_filename = "../data/light.txt";
	uint8_t compute_shader_source[8192];
	size_t read = light_file_read_buffer(compute_shader_filename, compute_shader_source, 8192);
	if(read == 0){
		fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", compute_shader_filename);
		return -1;
	}

	const GLchar *source = compute_shader_source;
	GLint length = read;

	GLuint compute_program = light_shader_compute_create(&source, &length);
	if(compute_program == 0)
	{
		fprintf(stderr, "light_shader_compute_create() failed.");
		return -1;
	}

	int result = light_scene_bind_program(scene, compute_program);
	if(result < 0)
	{
		glDeleteProgram(compute_program);

		fprintf(stderr, "light_scene_bind() failed. \n");	
		return -1;
	}

	instance->compute_program = compute_program;
	return 0;
}

void light_light_deinitialize(struct light_light_instance *instance)
{
	glDeleteProgram(instance->compute_program);
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


	
	
	/*create a quad as render surface */	
	struct light_surface quad_surface;
	result = light_surface_initialize(&quad_surface);
	if(result <0)
	{
		printf("light_surface_initialize(): failed. \n");
		exit(EXIT_FAILURE);	
	}
	
	struct light_scene_instance_build scene_instance_build = {
		.sphere_count 	= 1,
		.box_count 	= 1,
		.cylinder_count = 1,
		.light_count 	= 3,
	};
	
	struct light_scene_instance scene;
	result = light_scene_initialize(&scene, scene_instance_build);
	if(result < 0)
	{
		printf("light_scene_initialize(): failed. \n");
		exit(EXIT_FAILURE);
	}

	struct light_implicit_instance implicit_instance;
	result = light_implicit_initialize(&scene, &implicit_instance);
	if(result < 0)
	{
		printf("light_implicit_init(): failed. \n");
		exit(EXIT_FAILURE);
	}

	struct light_light_instance _light_instance;
	result = light_light_initialize(&scene, &_light_instance);
	if(result < 0)
	{
		printf("light_light_init(): failed. \n");
		exit(EXIT_FAILURE);
	}



	const uint32_t sphere_count = 1;
	struct light_scene_implicit_sphere_instance sphere_instance[sphere_count];
	
	struct light_physic_particle *body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * sphere_count); 
	for(uint32_t i = 0; i < sphere_count; i++)
	{
		{
			sphere_instance[i].radius = 1.0f;
			sphere_instance[i].color = (struct vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f};

			body[i] = (struct light_physic_particle){
				.position = (struct vec3){.x = 1.0f*i, .y = -2.0f*i, .z = 5.0f},
				.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
				.mass = 1.0f, 
				//.radius = 1.0f,
			};
		}
	}
	
	const uint32_t cylinder_count = 1;
	struct light_scene_implicit_cylinder_instance cylinder_instance[cylinder_count];
	struct light_physic_particle *cylinder_body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * cylinder_count); 

	for(uint32_t i = 0; i < cylinder_count; i++)
	{
		{
			cylinder_instance[i].radius = 1.0f;
			cylinder_instance[i].height = 1.0f;
			cylinder_instance[i].color = (struct vec3){.x = 1.0f, .y = 1.0f, .z = 0.0f};

			cylinder_body[i] = (struct light_physic_particle){
				.position = (struct vec3){.x = 4.0f, .y = 2.0f*i, .z = 5.0f},
				.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
				.mass = 1.0f, 
				//.radius = 1.0f,
			};
		}
	}


	const uint32_t box_count = 1;
	struct light_scene_implicit_box_instance box_instance[box_count];
	struct light_physic_particle *box_body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * box_count); 

	for(uint32_t i = 0; i < box_count; i++)
	{
		{
			box_instance[i].dimension = (struct vec3){.x = 1.0f, .y = 1.0f, .z = 1.0f};
			box_instance[i].color = (struct vec3){.x = 1.0f, .y = 1.0f, .z = 1.0f};

			box_body[i] = (struct light_physic_particle){
				.position = (struct vec3){.x = -4.0f, .y = -2.0f*i, .z = 5.0f},
				.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
				.mass = 1.0f, 
				//.radius = 1.0f,
			};
		}
	}

	
	const uint32_t light_count = 3;
	struct light_scene_light_light_instance light_instance[light_count];
	for(uint32_t i = 0; i < light_count; i++)
	{
		light_instance[i].position = (struct vec3){.x=-5.0f + 5.0f*i, 0.0f, 0.0f};
	}

	light_instance[0].color = (struct vec3){.x=1.0, .y=0.0f, .z=1.0f};
	light_instance[1].color = (struct vec3){.x=0.0, .y=1.0f, .z=1.0f};
	light_instance[2].color = (struct vec3){.x=0.0, .y=0.0f, .z=1.0f};

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	float 		fps_time;
	struct timeval 	fps_time_last;
	struct timeval 	fps_time_curr;

	gettimeofday(&fps_time_last, NULL);

	for(uint32_t i = 0; i < sphere_count; i++)
	{
		struct vec3 p = body[i].position;
		sphere_instance[i].translation = m4x4trs(p);
		sphere_instance[i].translation_inv = m4x4inv(&sphere_instance[i].translation, &result); 
	}




	for(uint32_t i = 0; i < cylinder_count; i++)
	{
		struct vec3 p = cylinder_body[i].position;
		cylinder_instance[i].translation = m4x4trs(p);
		cylinder_instance[i].translation_inv = m4x4inv(&cylinder_instance[i].translation, &result); 
	}



	for(uint32_t i = 0; i < box_count; i++)
	{
		struct vec3 p = box_body[i].position;
		box_instance[i].translation = m4x4trs(p);
		box_instance[i].translation_inv = m4x4inv(&box_instance[i].translation, &result); 
	}
		
	light_scene_buffer_commit_sphere(&scene, sphere_instance, sphere_count, 0);
	light_scene_buffer_commit_cylinder(&scene, cylinder_instance, cylinder_count, 0);
	light_scene_buffer_commit_box(&scene, box_instance, box_count, 0);
	light_scene_buffer_commit_light(&scene, light_instance, light_count, 0);


	
	while(!light_platform_exit())
    	{
		struct timeval fps_time_curr_tmp;
		gettimeofday(&fps_time_curr_tmp, NULL);	
		float deltatime = fps_time_curr_tmp.tv_sec - fps_time_curr.tv_sec + (float)(fps_time_curr_tmp.tv_usec - fps_time_curr.tv_usec)/(1000.0f*1000.0f);
		fps_time_curr = fps_time_curr_tmp ;
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
#if 0
		struct vec3 gravity = (struct vec3){.x = 0.0f, .y = -9.82f, .z=0.0f};
		for(uint32_t i = 1; i < sphere_count; i++)
		{
			body[i].velocity = v3add(body[i].velocity, v3scl(gravity, deltatime));	
		}
	
		body[0].velocity = (struct vec3){.x = 0.0f, .y=0.0f, .z=0.0f};	
		body[0].position = (struct vec3){.x = 0.0f, .y=0.0f, .z=10.0f};	
		light_physic_rope(body, sphere_count);
#endif 
	
		
		/* copy the physic simulation into the rendering buffer */
		uint32_t width, height;
		light_platform_resolution(&width, &height);

		width = 512;
		height = 512;

		light_scene_bind(&scene, width, height, deltatime);
		
		light_implicit_dispatch(&implicit_instance, width, height);

		glUseProgram(_light_instance.compute_program);
		glDispatchCompute(width, height, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glViewport(0, 0, width/2, height/2);
		glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		light_surface_render(&quad_surface, scene.framebuffer.color_texture);
		
		glViewport(width/2, 0, width/2, height/2);
		light_surface_render(&quad_surface, scene.framebuffer.position_texture);

		glViewport(0, height/2, width/2, height/2);
		light_surface_render(&quad_surface, scene.framebuffer.normal_texture);

		glViewport(width/2, height/2, width/2, height/2);
		light_surface_render(&quad_surface, implicit_instance.prepass_texture);




		light_platform_update();
    	}
		
	light_implicit_deinitialize(&implicit_instance);
	light_light_deinitialize(&_light_instance);
	light_scene_deinitialize(&scene);




	printf( "exiting");
	light_platform_deinitialize();
		
	return (result);
}
