#ifndef SCENE_PARTICLE_H
#define SCENE_PARTICLE_H

#include "platform.h"
#include "math/vec.h"

#include "state.h"

#include <stdint.h>

void light_scene_particle_simulate(
		struct light_scene_state_instance* instance, 
	       	float deltatime
);

int light_scene_particle_initialize(
		struct light_scene_state_instance* instance
);

void light_scene_particle_deinitialize(
		struct light_scene_state_instance* instance
);


#endif 
