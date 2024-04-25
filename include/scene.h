#ifndef SCENE_INSTANCE_H
#define SCENE_INSTANCE_H

#include <stdio.h>

#include "math/vec.h"
#include "math/mat4x4.h"

#include "camera.h"
#include "camera_input.h"
#include "framebuffer.h"
#include "frame.h"

#include "scene/object.h"

#include "platform.h"

/* 
 * Buffers for each type of object in 
 * the scene. 
 */
struct light_scene_instance_buffer
{
	GLuint sphere_buffer;
	GLuint cylinder_buffer;
	GLuint box_buffer;
	GLuint light_buffer;
};

/* 
 * Set number of objects of each type at scene initialization 
 */
struct light_scene_instance_build
{
	uint32_t sphere_count;

	uint32_t cylinder_count;

	uint32_t box_count;

	uint32_t light_count;
};

/* 
 * Scene instance 
 */
struct light_scene_instance 
{
	struct light_scene_instance_build instance_build;

	struct light_scene_instance_buffer instance_buffer;

	struct light_framebuffer framebuffer;

	struct light_frame_info frame_info;

	struct light_camera_view_state view_state;

	struct light_camera_update_state update_state;

};


int light_scene_initialize(struct light_scene_instance *instance, struct light_scene_instance_build instance_build);
void light_scene_deinitialize(struct light_scene_instance *instance);

void light_scene_buffer_commit_sphere(struct light_scene_instance *scene, struct light_scene_implicit_sphere_instance *sphere, uint32_t sphere_count, uint32_t sphere_offset);
void light_scene_buffer_commit_cylinder(struct light_scene_instance *scene, struct light_scene_implicit_cylinder_instance *cylinder, uint32_t cylinder_count, uint32_t cylinder_offset);
void light_scene_buffer_commit_box(struct light_scene_instance *scene, struct light_scene_implicit_box_instance *box, uint32_t box_count, uint32_t box_offset);
void light_scene_buffer_commit_light(struct light_scene_instance *scene, struct light_scene_light_light_instance *light, uint32_t light_count, uint32_t light_offset);

int light_scene_bind_program(struct light_scene_instance *instance, GLint program);
int light_scene_bind(struct light_scene_instance *instance, uint32_t width, uint32_t height, const float deltatime);


#endif 

