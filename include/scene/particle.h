#ifndef SCENE_PARTICLE_H
#define SCENE_PARTICLE_H

#include "platform.h"

#include "surface.h"
#include "math/vec.h"

#include "state.h"

#include <stdint.h>

struct light_scene_particle_emitter_normal
{
	struct vec3 position;
	float lifetime_expected;
	struct vec3 velocity_expected;
	float lifetime_variance;	
	struct vec3 velocity_variance;
	uint32_t emitter_offset;
	uint32_t emitter_count;
	uint32_t padding[3];
};


void light_scene_particle_simulate(struct light_scene_state_instance* instance, struct light_scene_state_build *build, float deltatime);

int light_scene_particle_initialize(struct light_scene_state_instance* instance, struct light_scene_state_build* build);
void light_scene_particle_deinitialize(struct light_scene_particle_instance* instance);


#endif 
