#ifndef LIGHT_SCENE_IMPLICIT_H
#define LIGHT_SCENE_IMPLICIT_H

#include "platform.h"

#include "misc/file.h"

#include "config.h"
#include "shader.h"
#include "scene.h"
#include "object.h"
#include "surface.h"
#include "state.h"

int light_scene_implicit_initialize(
	       	struct light_scene_state_instance  *instance
);

void light_scene_implicit_deinitialize(
	       	struct light_scene_state_instance  *instance
);

void
light_scene_implicit_compute_inerita(
		struct light_scene_state_instance *instance,
		struct light_scene_implicit_object *objects, 
		uint32_t object_count,
		struct mat3x3 *object_inerita_without_mass, 
		uint32_t samples

);

void light_scene_implicit_dispatch_render(
	       	struct light_scene_state_instance  *instance,
	       	uint32_t width, uint32_t height
);

uint32_t light_scene_implicit_dispatch_physic(
	       	struct light_scene_state_instance  *instance,
	       	struct light_scene_implicit_collision *collision,
		const uint32_t collision_count
);

void light_scene_implicit_commit_objects(
	       	struct light_scene_state_instance  *instance,
	       	struct light_scene_implicit_object *object_node,
		uint32_t object_node_count
);
	
void light_scene_implicit_commit_light(
	       	struct light_scene_state_instance  *instance,
	       	struct light_scene_light_light_instance *light,
	       	uint32_t light_count
);



#endif


