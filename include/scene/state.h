#ifndef SCENE_STATE_H
#define SCENE_STATE_H

#include "config.h"
#include "surface.h"
#include "framebuffer.h"

#include "scene/object.h"


struct light_scene_implicit_build
{
	uint32_t object_node_count; 

	uint32_t light_count;

};

struct light_scene_implicit_instance
{
	/* Number of object nodes in the object node buffer. */
	uint32_t object_node_count;
	
	/* GPU buffers */
	GLuint object_node_buffer;
	GLuint light_buffer;
	GLuint collision_pair_buffer;

	/* Used for atmoic counter */
	GLuint collision_pair_counter_buffer;
	
	/* Program used for rendering */
	GLuint render_program;

	/* Program used for physic intersection */
	GLuint physic_program;

	/* Program used for calulcating volumes */
	GLuint volume_program;

	GLint volume_program_samples_uniform;
	GLint volume_program_node_index_uniform;

	/* Program used for calculating inerita */
	GLuint inertia_program;

	GLint inertia_program_samples_uniform;
	GLint inertia_program_node_index_uniform;


	/* Number of implicit functions found */	
	uint32_t implicit_function_name_count;

	/* name of the implicit function */
	char implicit_function_name[LIGHT_IMPLICIT_FUNCTION_MAX_COUNT][255];

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

	GLuint particle_program;

};

int light_scene_state_initialize(
		struct light_scene_state_instance *instance,
	       	struct light_scene_state_build build
);

void light_scene_state_deinitialize(
		struct light_scene_state_instance *instance
);


void light_scene_state_bind(
		struct light_scene_state_instance *instance
);

uint32_t light_scene_state_dispatch(
		struct light_scene_state_instance *instance,
	       	struct light_framebuffer *framebuffer,
	       	uint32_t width, uint32_t height,float deltatime,
		struct light_scene_implicit_collision *collision,
		const uint32_t collision_count
);

#endif 
