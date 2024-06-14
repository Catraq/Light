#ifndef LIGHT_SCENE_IMPLICIT_H
#define LIGHT_SCENE_IMPLICIT_H

#include "platform.h"

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

void light_scene_implicit_dispatch(
	       	struct light_scene_state_instance  *instance,
	       	uint32_t width, uint32_t height
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


