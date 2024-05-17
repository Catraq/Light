#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

#include "state.h"
#include "object.h"

int light_scene_particle_emitter_initialize(
	struct light_scene_state_instance *instance
);

void light_scene_particle_emitter_deinitialize(
	struct light_scene_state_instance *instance
);

void light_scene_particle_emitter_commit_normal(
	struct light_scene_state_instance *instance, 
	struct light_scene_particle_emitter_normal *normal,
	uint32_t normal_count
);

#endif 
