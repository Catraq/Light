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


struct light_implicit_instance_build
{
	uint32_t object_count;

	uint32_t object_node_count; 

	uint32_t object_node_max_level;

	uint32_t sphere_count;

	uint32_t cylinder_count;

	uint32_t box_count;

	uint32_t light_count;

};

struct light_implicit_instance
{
	struct light_implicit_instance_build instance_build;

	struct light_surface surface;

	GLuint object_buffer;
	GLuint object_node_buffer;
	GLuint sphere_buffer;
	GLuint cylinder_buffer;
	GLuint box_buffer;
	GLuint light_buffer;

};


int light_implicit_initialize(struct light_scene_instance *scene, struct light_implicit_instance *instance, struct light_implicit_instance_build instance_build)
{
	instance->instance_build = instance_build;

	
	{
		const char *compute_shader_filename = "../data/implicit.txt";
		uint8_t compute_shader_source[8192];
		size_t read = light_file_read_buffer(compute_shader_filename, compute_shader_source, 8192);
		if(read == 0){
			fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", compute_shader_filename);
			return -1;
		}

		uint8_t compute_shader_source_header[512];
		uint32_t gen = snprintf(compute_shader_source_header, 512, 
			"#version 430 core \n" 
			"#define OBJECT_NODE_STACK %u \n"
			"#define OBJECT_NODE_COUNT %u \n"
			"#define OBJECT_COUNT %u \n"
			"#define SPHERE_COUNT %u \n"
			"#define BOX_COUNT %u \n"
			"#define CYLINDER_COUNT %u \n",
		       	instance_build.object_node_max_level,
			instance_build.object_node_count,
			instance_build.object_count,
			instance_build.sphere_count,
			instance_build.box_count,
			instance_build.cylinder_count
		); 

		const GLchar *source_fragment[] =
		{
			compute_shader_source_header,
			compute_shader_source,
		};

		uint32_t length_fragment[] = {
			gen, read
		};

		const GLchar *source_vertex[] = {
			light_surface_vertex_shader_source
		};


		int result = light_surface_initialize_vertex_fragement_source(
				&instance->surface, 
				source_vertex, NULL, 1,
			       	source_fragment, length_fragment, 2
		);

		if(result < 0)
		{
			fprintf(stderr, "light_surface_initialize_vertex_fragement_source() failed. \n");
			return -1;	
		}

	}

	uint32_t sphere_buffer_size = sizeof(struct light_scene_implicit_sphere_instance) * instance_build.sphere_count + 4*sizeof(uint32_t);
	GLuint sphere_buffer;
	glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, sphere_buffer);
	glBufferData(GL_UNIFORM_BUFFER,  sphere_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	uint32_t cylinder_buffer_size = sizeof(struct light_scene_implicit_cylinder_instance) * instance_build.cylinder_count + 4*sizeof(uint32_t);
	GLuint cylinder_buffer;
	glGenBuffers(1, &cylinder_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, cylinder_buffer);
	glBufferData(GL_UNIFORM_BUFFER, cylinder_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	uint32_t box_buffer_size = sizeof(struct light_scene_implicit_box_instance) * instance_build.box_count + 4*sizeof(uint32_t);	
	GLuint box_buffer;
	glGenBuffers(1, &box_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, box_buffer);
	glBufferData(GL_UNIFORM_BUFFER, box_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	uint32_t light_buffer_size = sizeof(struct light_scene_light_light_instance) * instance_build.light_count + 4*sizeof(uint32_t);
	GLuint light_buffer;
	glGenBuffers(1, &light_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, light_buffer);
	glBufferData(GL_UNIFORM_BUFFER, light_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	/* See shader source for structure of matrix */	
	uint32_t object_buffer_size = sizeof(struct light_scene_implicit_object_instance) * instance_build.object_count + 4*sizeof(uint32_t);
	GLuint object_buffer;
	glGenBuffers(1, &object_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, object_buffer);
	glBufferData(GL_UNIFORM_BUFFER, object_buffer_size, NULL, GL_STATIC_DRAW);
	

	uint32_t object_node_buffer_size = sizeof(struct light_scene_implicit_object_node) * instance_build.object_node_count + 4*sizeof(uint32_t);	
	GLuint object_node_buffer;
	glGenBuffers(1, &object_node_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, object_node_buffer);
	glBufferData(GL_UNIFORM_BUFFER, object_node_buffer_size,  NULL, GL_STATIC_DRAW);
	
	instance->object_buffer 	= object_buffer;
	instance->object_node_buffer 	= object_node_buffer;
	instance->sphere_buffer		= sphere_buffer;
	instance->cylinder_buffer	= cylinder_buffer;
	instance->box_buffer		= box_buffer;
	instance->light_buffer		= light_buffer;

	return 0;
}

void light_implicit_commit_objects(struct light_implicit_instance *instance,
	       	struct light_scene_implicit_object_instance *object_instance, uint32_t object_instance_count, 
	       	struct light_scene_implicit_object_node *object_node, uint32_t object_node_count)
{


	glBindBuffer(GL_UNIFORM_BUFFER, instance->object_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_implicit_object_instance)*object_instance_count, object_instance);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_implicit_object_instance)*object_instance_count, sizeof(uint32_t), &object_instance_count);


	glBindBuffer(GL_UNIFORM_BUFFER, instance->object_node_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_implicit_object_node)*object_node_count, object_node);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_implicit_object_node)*object_node_count, sizeof(uint32_t), &object_node_count);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void light_implicit_commit_sphere(struct light_implicit_instance *instance, struct light_scene_implicit_sphere_instance *sphere, uint32_t sphere_count)
{
	glBindBuffer(GL_UNIFORM_BUFFER, instance->sphere_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_implicit_sphere_instance) * sphere_count, sphere);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_implicit_sphere_instance) * sphere_count, sizeof(uint32_t),  &sphere_count);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void light_implicit_commit_cylinder(struct light_implicit_instance *instance, struct light_scene_implicit_cylinder_instance *cylinder, uint32_t cylinder_count)
{
	glBindBuffer(GL_UNIFORM_BUFFER, instance->cylinder_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_implicit_cylinder_instance) * cylinder_count, cylinder);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_implicit_cylinder_instance) * cylinder_count, sizeof(uint32_t), &cylinder_count);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void light_implicit_commit_box(struct light_implicit_instance *instance, struct light_scene_implicit_box_instance *box, uint32_t box_count)
{
	glBindBuffer(GL_UNIFORM_BUFFER, instance->box_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_implicit_box_instance) * box_count, box);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_implicit_box_instance) * box_count, sizeof(uint32_t), &box_count);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

	
void light_implicit_commit_light(struct light_implicit_instance *instance, struct light_scene_light_light_instance *light, uint32_t light_count)
{
	glBindBuffer(GL_UNIFORM_BUFFER, instance->light_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_light_light_instance) * light_count, light);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_light_light_instance) * light_count, sizeof(uint32_t),  &light_count);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void light_implicit_dispatch(struct light_implicit_instance *instance, uint32_t width, uint32_t height)
{


	glBindBufferBase(GL_UNIFORM_BUFFER, 0, instance->sphere_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, instance->cylinder_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, instance->box_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, instance->light_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, instance->object_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, instance->object_node_buffer);

	light_surface_render(&instance->surface);

}

void light_implicit_deinitialize(struct light_implicit_instance *instance)
{
	
	glDeleteBuffers(1, &instance->sphere_buffer);
	glDeleteBuffers(1, &instance->cylinder_buffer);
	glDeleteBuffers(1, &instance->box_buffer);
	glDeleteBuffers(1, &instance->light_buffer);
	
	glDeleteBuffers(1, &instance->object_buffer);
	glDeleteBuffers(1, &instance->object_node_buffer);
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

	GLuint compute_program = light_shader_compute_create(&source, &length, 1);
	if(compute_program == 0)
	{
		fprintf(stderr, "light_shader_compute_create() failed.");
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
	CHECK_GL_ERROR();
	
	struct light_scene_instance scene;
	result = light_scene_initialize(&scene);
	if(result < 0)
	{
		printf("light_scene_initialize(): failed. \n");
		exit(EXIT_FAILURE);
	}

	CHECK_GL_ERROR();

	struct light_implicit_instance implicit_instance;
	{
		struct light_implicit_instance_build build = {
			.object_count = 1,
			.object_node_count = 4,
			.object_node_max_level = 5,	
			.sphere_count 	= 2,
			.box_count 	= 1,
			.cylinder_count = 1,
			.light_count 	= 3,
		};

		result = light_implicit_initialize(&scene, &implicit_instance, build);
		if(result < 0)
		{
			printf("light_implicit_init(): failed. \n");
			exit(EXIT_FAILURE);
		}
	}

	struct light_light_instance _light_instance;
	result = light_light_initialize(&scene, &_light_instance);
	if(result < 0)
	{
		printf("light_light_init(): failed. \n");
		exit(EXIT_FAILURE);
	}



	const uint32_t sphere_count = 2;
	struct light_scene_implicit_sphere_instance sphere_instance[sphere_count];
	
	struct light_physic_particle *body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * sphere_count); 
	for(uint32_t i = 0; i < sphere_count; i++)
	{
		{
			sphere_instance[i].radius = 1.0f;
			sphere_instance[i].color = (struct vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f};

			body[i] = (struct light_physic_particle){
				.position = (struct vec3){.x = 0.0f, .y = -1.0*i, .z = 3.0f},
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

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	float 		fps_time;
	struct timeval 	fps_time_last;
	struct timeval 	fps_time_curr;

	gettimeofday(&fps_time_last, NULL);
	fps_time_curr = fps_time_last;

		
	light_implicit_commit_sphere(&implicit_instance, sphere_instance, sphere_count);
	light_implicit_commit_cylinder(&implicit_instance, cylinder_instance, cylinder_count);
	light_implicit_commit_box(&implicit_instance, box_instance, box_count);
	light_implicit_commit_light(&implicit_instance, light_instance, light_count);

	

	struct light_scene_implicit_object_instance object_instance[1];
	struct light_scene_implicit_object_node object_node[4];


	{
		struct vec3 p1 = {.x = 0.0, .y = 1.0, .z = 0};
		object_node[0].translation = m4x4trs(p1);
		object_node[0].translation_inv = m4x4inv(&object_node[0].translation, &result); 
		object_node[0].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_node[0].object_index = 0;
		object_node[0].node_index = 2;

		struct vec3 p2 = {.x = 0.0, .y = -1.0, .z = 0};
		object_node[1].translation = m4x4trs(p2);
		object_node[1].translation_inv = m4x4inv(&object_node[1].translation, &result); 
		object_node[1].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_node[1].object_index = 1;
		object_node[1].node_index = 3;

		struct vec3 p3 = {.x = 1.0, .y = 0.0, .z = 0};
		object_node[2].translation = m4x4trs(p3);
		object_node[2].translation_inv = m4x4inv(&object_node[2].translation, &result); 
		object_node[2].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_node[2].object_index = 0;

		struct vec3 p4 = {.x = -1.0, .y = 0.0, .z = 0};
		object_node[3].translation = m4x4trs(p4);
		object_node[3].translation_inv = m4x4inv(&object_node[3].translation, &result); 
		object_node[3].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_node[3].object_index = 1;

	}


	{
		struct vec3 p = {.x = 0.0, .y = 0.0, .z = 10};
		object_instance[0].translation = m4x4trs(p);
		object_instance[0].translation_inv = m4x4inv(&object_instance[0].translation, &result); 
		object_instance[0].index_type = LIGHT_SCENE_IMPLICIT_UNION;
		object_instance[0].index_left = 0;
		object_instance[0].index_right = 1;
		object_instance[0].levels = 2;
	}

	light_implicit_commit_objects(&implicit_instance, object_instance, 1, object_node, 4);

	float t = 0.0;
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
		
		t += deltatime;
		{
			struct vec3 p = {.x = 0.0, .y = cosf(t), .z = 10};
			object_instance[0].translation = m4x4trs(p);
			object_instance[0].translation_inv = m4x4inv(&object_instance[0].translation, &result); 
		}

		light_implicit_commit_objects(&implicit_instance, object_instance, 1, object_node, 4);
			
		
		/* copy the physic simulation into the rendering buffer */
		
		uint32_t width, height;
		width = 512; height = 512;
		light_scene_bind(&scene, width, height, deltatime);
#if 1	
		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, scene.framebuffer.framebuffer);
		light_implicit_dispatch(&implicit_instance, width, height);
#endif 

#if 0
		glUseProgram(_light_instance.compute_program);
		glDispatchCompute(width, height, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
#endif 
		light_platform_resolution(&width, &height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width/2, height/2);
		glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		light_surface_render_textured(&quad_surface, scene.framebuffer.color_texture);
		
		glViewport(width/2, 0, width/2, height/2);
		light_surface_render_textured(&quad_surface, scene.framebuffer.position_texture);

		glViewport(0, height/2, width/2, height/2);
		light_surface_render_textured(&quad_surface, scene.framebuffer.normal_texture);
#if 0
		glViewport(width/2, height/2, width/2, height/2);
		light_surface_render(&quad_surface, scene.framebuffer.composed_texture);
#endif 


		light_platform_update();
		glError("main loop:");
    	}
		
	light_implicit_deinitialize(&implicit_instance);
	light_light_deinitialize(&_light_instance);
	light_scene_deinitialize(&scene);




	printf( "exiting");
	light_platform_deinitialize();
		
	return (result);
}
