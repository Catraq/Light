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

/*
 * mirrored in gpu. have to correspond with shader input. 
 * adjust vertex attribute location if changed. 
 */
struct light_game_implicit_sphere_instance
{
	struct mat4x4 translation;

	struct mat4x4 translation_inv;

	/* todo: radius of each sphere  */
	float radius[4] 
};


/*
 * mirrored in gpu. have to correspond with shader input. 
 * adjust vertex attribute location if changed. 
 */
struct light_game_implicit_cylinder_instance
{
	struct mat4x4 translation;

	struct mat4x4 translation_inv;

	/* radius of each cylinder  */
	float radius; 

	/* height of cylinder */
	float height;
	
	/* Required padding */
	float dummy[2];
};

/*
 * mirrored in gpu. have to correspond with shader input. 
 * adjust vertex attribute location if changed. 
 */
struct light_game_implicit_box_instance
{
	struct mat4x4 translation;
	
	struct mat4x4 translation_inv;

	/* box dimension */	
	struct vec3 dimension;

	float padding[1];
};

struct light_game_light_light_instance
{
	struct vec3 postion;
	float power;
};


size_t light_file_read_buffer(const char *file, uint8_t *buffer, size_t buffer_size)
{
	FILE *fp = fopen(file, "r");
	if(fp == NULL){
		fprintf(stderr, "Could not open file %s \n", file);
		return 0;
	}
	
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	rewind(fp);

	if(size > buffer_size){
		fprintf(stderr, "file content of %s (%lu bytes)  greater than buffer size(%lu bytes). \n", file, size, buffer_size);
		return 0;
	}

	size_t read = fread(buffer, 1, size, fp);
	if(read != size){
		fprintf(stderr, "Expected to read (%lu bytes) of %s, but got (%lu bytes). \n", size, file, buffer_size);
		return 0;
	}


	return read;

}

struct light_implicit_instance
{
	GLuint compute_program;
};

int light_implicit_init(struct light_scene_instance *scene, struct light_implicit_instance *instance)
{
	const char *compute_shader_filename = "../data/implicit.txt";
	uint8_t compute_shader_source[8192];
	size_t read = light_file_read_buffer(compute_shader_filename, compute_shader_source, 8192);
	if(read == 0){
		fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", compute_shader_filename);
		return -1;
	}

	GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
	if(compute_shader == 0)
	{
		fprintf(stderr, "glCreateShader(GL_COMPUTE_SHADER) failed.");
		return -1;
	}

	
	const GLchar *source = compute_shader_source;
	GLint length = read;
	glShaderSource(compute_shader, 1, &source, &length);
	glCompileShader(compute_shader);
	
	GLint compiled = GL_FALSE;
	glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE){

		GLchar log[8192];
		GLsizei length;
		glGetShaderInfoLog(compute_shader, 8192, &length, (GLchar*)&log);
		if( length != 0 )
		{
			fprintf(stderr, " ---- Compilation of shader for implicit rendering failed.  ---- \n %s \n", (GLchar*)&log);
		}

		glDeleteShader(compute_shader);

		return -1;
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
		fprintf(stderr, " ---- Linking of program for implcit rendering failed ---- \n %s \n", (GLchar*)&log);

		glDeleteProgram(compute_program);
		return -1;
	}

	int result = light_scene_bind(scene, compute_program);
	if(result < 0)
	{
		glDeleteProgram(compute_program);

		fprintf(stderr, "light_scene_bind() failed. \n");	
		return -1;
	}

	instance->compute_program = compute_program;
	return 0;
}

struct light_light_instance
{
	GLuint compute_program;

};

int light_light_init(struct light_scene_instance *scene, struct light_implicit_instance *instance)
{
	const char *compute_shader_filename = "../data/light.txt";
	uint8_t compute_shader_source[8192];
	size_t read = light_file_read_buffer(compute_shader_filename, compute_shader_source, 8192);
	if(read == 0){
		fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", compute_shader_filename);
		return -1;
	}

	GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
	if(compute_shader == 0)
	{
		fprintf(stderr, "glCreateShader(GL_COMPUTE_SHADER) failed.");
		return -1;
	}

	
	const GLchar *source = compute_shader_source;
	GLint length = read;
	glShaderSource(compute_shader, 1, &source, &length);
	glCompileShader(compute_shader);
	
	GLint compiled = GL_FALSE;
	glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE){

		GLchar log[8192];
		GLsizei length;
		glGetShaderInfoLog(compute_shader, 8192, &length, (GLchar*)&log);
		if( length != 0 )
		{
			fprintf(stderr, " ---- Compilation of shader for lightning rendering failed.  ---- \n %s \n", (GLchar*)&log);
		}

		glDeleteShader(compute_shader);

		return -1;
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
		fprintf(stderr, " ---- Linking of program for lightning rendering failed ---- \n %s \n", (GLchar*)&log);

		glDeleteProgram(compute_program);
		return -1;
	}

	int result = light_scene_bind(scene, compute_program);
	if(result < 0)
	{
		glDeleteProgram(compute_program);

		fprintf(stderr, "light_scene_bind() failed. \n");	
		return -1;
	}

	instance->compute_program = compute_program;
	return 0;
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
		.sphere_count 	= 10,
		.box_count 	= 10,
		.cylinder_count = 10
	};
	
	struct light_scene_instance scene;
	result = light_scene_initialize(&scene);
	if(result < 0)
	{
		printf("light_scene_initialize(): failed. \n");
		exit(EXIT_FAILURE);
	}

	struct light_implicit_instance implicit_instance;
	result = light_implicit_init(&scene, &implicit_instance);
	if(result < 0)
	{
		printf("light_implicit_init(): failed. \n");
		exit(EXIT_FAILURE);
	}

	struct light_light_instance light_instance;
	result = light_light_init(&scene, &light_instance);
	if(result < 0)
	{
		printf("light_light_init(): failed. \n");
		exit(EXIT_FAILURE);
	}

	GLuint composed_texture;
	glGenTextures(1, &composed_texture);
	glBindTexture(GL_TEXTURE_2D, composed_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



	const uint32_t sphere_count = 10;
	struct light_game_implicit_sphere_instance sphere_instance[sphere_count];
	
	struct light_physic_particle *body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * sphere_count); 
	for(uint32_t i = 0; i < sphere_count; i++)
	{
		{
			sphere_instance[i].radius[0] = 1.0f;

			body[i] = (struct light_physic_particle){
				.position = (struct vec3){.x = 1.0f*i, .y = -2.0f*i, .z = 10.0f},
				.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
				.mass = 1.0f, 
				//.radius = 1.0f,
			};
		}
	}
	
	const uint32_t cylinder_count = 5;
	struct light_game_implicit_cylinder_instance cylinder_instance[cylinder_count];
	struct light_physic_particle *cylinder_body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * cylinder_count); 

	for(uint32_t i = 0; i < cylinder_count; i++)
	{
		{
			cylinder_instance[i].radius = 1.0f;
			cylinder_instance[i].height = 1.0f;

			cylinder_body[i] = (struct light_physic_particle){
				.position = (struct vec3){.x = 1.0f*i, .y = 2.0f*i, .z = 7.0f},
				.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
				.mass = 1.0f, 
				//.radius = 1.0f,
			};
		}
	}


	const uint32_t box_count = 5;
	struct light_game_implicit_box_instance box_instance[box_count];
	struct light_physic_particle *box_body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * box_count); 

	for(uint32_t i = 0; i < box_count; i++)
	{
		{
			box_instance[i].dimension = (struct vec3){.x = 1.0f, .y = 1.0f, .z = 1.0f};

			box_body[i] = (struct light_physic_particle){
				.position = (struct vec3){.x = 1.0f*i, .y = -2.0f*i, .z = 4.0f},
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

	GLuint sphere_buffer;
	glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphere_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(sphere_instance), sphere_instance, GL_STREAM_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	GLuint cylinder_buffer;
	glGenBuffers(1, &cylinder_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, cylinder_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(cylinder_instance), cylinder_instance, GL_STREAM_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	GLuint box_buffer;
	glGenBuffers(1, &box_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, box_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(box_instance), box_instance, GL_STREAM_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	while(!light_platform_exit())
    	{

	
		float deltatime = (float)(clock() - time)/(float)CLOCKS_PER_SEC;

		time = clock();

		fps_frame_count++;
		float fps_interval_time = (float)(clock() - fps_sample_last)/(float)CLOCKS_PER_SEC;
		if(fps_interval_time > fps_sample_interval || fps_frame_count > 500)
		{
			uint32_t frames_per_sec = (float)fps_frame_count/fps_interval_time;
			fps_frame_count  = 0;
			fps_sample_last = clock();
			
			printf("5s fps average: %u \n", frames_per_sec);

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
		for(uint32_t i = 0; i < sphere_count; i++)
		{
			body[i].position = v3add(body[i].position, v3scl(body[i].velocity, deltatime));
			struct vec3 p = body[i].position;
			sphere_instance[i].translation = m4x4trs(p);
			sphere_instance[i].translation_inv = m4x4inv(&sphere_instance[i].translation, &result); 
		}


	

		for(uint32_t i = 0; i < cylinder_count; i++)
		{
			cylinder_body[i].position = v3add(cylinder_body[i].position, v3scl(cylinder_body[i].velocity, deltatime));
			struct vec3 p = cylinder_body[i].position;
			cylinder_instance[i].translation = m4x4trs(p);
			cylinder_instance[i].translation_inv = m4x4inv(&cylinder_instance[i].translation, &result); 
		}



		for(uint32_t i = 0; i < box_count; i++)
		{
			box_body[i].position = v3add(box_body[i].position, v3scl(box_body[i].velocity, deltatime));
			struct vec3 p = box_body[i].position;
			box_instance[i].translation = m4x4trs(p);
			box_instance[i].translation_inv = m4x4inv(&box_instance[i].translation, &result); 
		}
		uint32_t width, height;
		light_platform_resolution(&width, &height);
		

		light_scene_update(&scene, width, height, deltatime);
		

		glBindTexture(GL_TEXTURE_2D, composed_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);



		glBindImageTexture(0, scene.framebuffer.color_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(1, scene.framebuffer.position_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(2, scene.framebuffer.normal_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(3, composed_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);


		glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphere_buffer);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(sphere_instance), sphere_instance);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sphere_buffer);
	

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, cylinder_buffer);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(cylinder_instance), cylinder_instance);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cylinder_buffer);
	

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, box_buffer);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(box_instance), box_instance);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, box_buffer);
	

		/* implicit start */	
		glUseProgram(implicit_instance.compute_program);
		glDispatchCompute(width, height, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		/* implcit end */

		glUseProgram(light_instance.compute_program);
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
		light_surface_render(&quad_surface, composed_texture);




		light_platform_update();
    	}
	
	printf( "exiting");
	light_platform_deinitialize();
		
	return (result);
}
