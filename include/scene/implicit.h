#ifndef LIGHT_SCENE_IMPLICIT_H
#define LIGHT_SCENE_IMPLICIT_H

#include "platform.h"

#include "scene.h"
#include "object.h"
#include "surface.h"

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

int light_implicit_initialize(
		struct light_scene_instance *scene,
	       	struct light_implicit_instance *instance,
	       	struct light_implicit_instance_build instance_build
);

void light_implicit_deinitialize(
		struct light_implicit_instance *instance
);

void light_implicit_dispatch(
		struct light_implicit_instance *instance,
	       	uint32_t width, uint32_t height
);

void light_implicit_commit_objects(
		struct light_implicit_instance *instance,
	       	struct light_scene_implicit_object_instance *object_instance, uint32_t object_instance_count, 
	       	struct light_scene_implicit_object_node *object_node, uint32_t object_node_count
);

void light_implicit_commit_sphere(
		struct light_implicit_instance *instance,
	       	struct light_scene_implicit_sphere_instance *sphere,
	       	uint32_t sphere_count
);


void light_implicit_commit_cylinder(
		struct light_implicit_instance *instance,
	       	struct light_scene_implicit_cylinder_instance *cylinder,
	       	uint32_t cylinder_count
);


void light_implicit_commit_box(
		struct light_implicit_instance *instance,
	       	struct light_scene_implicit_box_instance *box,
	       	uint32_t box_count
);
	
void light_implicit_commit_light(
		struct light_implicit_instance *instance,
	       	struct light_scene_light_light_instance *light,
	       	uint32_t light_count
);



#endif


