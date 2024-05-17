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
	       	struct light_scene_implicit_object_instance *object_instance, uint32_t object_instance_count, 
	       	struct light_scene_implicit_object_node *object_node, uint32_t object_node_count
);

void light_scene_implicit_commit_sphere(
	       	struct light_scene_state_instance  *instance,
	       	struct light_scene_implicit_sphere_instance *sphere,
	       	uint32_t sphere_count
);


void light_scene_implicit_commit_cylinder(
	       	struct light_scene_state_instance  *instance,
	       	struct light_scene_implicit_cylinder_instance *cylinder,
	       	uint32_t cylinder_count
);


void light_scene_implicit_commit_box(
	       	struct light_scene_state_instance  *instance,
	       	struct light_scene_implicit_box_instance *box,
	       	uint32_t box_count
);
	
void light_scene_implicit_commit_light(
	       	struct light_scene_state_instance  *instance,
	       	struct light_scene_light_light_instance *light,
	       	uint32_t light_count
);



#endif


