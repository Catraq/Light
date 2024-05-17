#ifndef SCENE_STATE_H
#define SCENE_STATE_H

#include "surface.h"
#include "framebuffer.h"


struct light_scene_implicit_build
{
	uint32_t object_count;

	uint32_t object_node_count; 

	uint32_t object_node_max_level;

	uint32_t sphere_count;

	uint32_t cylinder_count;

	uint32_t box_count;

	uint32_t light_count;

};

struct light_scene_implicit_instance
{
	GLuint object_buffer;
	GLuint object_node_buffer;
	GLuint sphere_buffer;
	GLuint cylinder_buffer;
	GLuint box_buffer;
	GLuint light_buffer;

	GLuint program;

};





struct light_scene_particle_build
{
	/* Number of emitters */
	uint32_t emitter_count;

	/* Number of particles per emitter */
	uint32_t emitter_particle_count;
};


struct light_scene_particle_instance
{
	
	uint32_t buffer_index;

	GLuint framebuffer[2];
	GLuint position[2];
	GLuint velocity[2];
	GLuint acceleration[2];
	GLuint lifetime[2];
	
	GLuint program;
	GLuint program_deltatime_location;
};


struct light_scene_particle_emitter_build
{
	uint32_t emitter_normal_count;	
};

struct light_scene_particle_emitter_instance
{
	GLuint emitter_normal_buffer;
	GLuint emitter_lookup_buffer;
};


struct light_scene_state_build
{
	struct light_scene_particle_build particle_build;

	struct light_scene_particle_emitter_build particle_emitter_build;

	struct light_scene_implicit_build implicit_build;
};

struct light_scene_state_instance
{
	struct light_scene_state_build build;

	struct light_surface surface;

	struct light_scene_particle_instance particle_instance;

	struct light_scene_particle_emitter_instance particle_emitter_instance;

	struct light_scene_implicit_instance implicit_instance;

};

int light_scene_state_initialize(
		struct light_scene_state_instance *instance,
	       	struct light_scene_state_build build
);

void light_scene_state_bind(
		struct light_scene_state_instance *instance
);

void light_scene_state_dispatch(
		struct light_scene_state_instance *instance,
	       	struct light_framebuffer *framebuffer,
	       	uint32_t width, uint32_t height,
	       	float deltatime
);

#endif 
