#ifndef SCENE_INSTANCE_H
#define SCENE_INSTANCE_H

#include <stdio.h>

#include "math/vec.h"
#include "math/mat4x4.h"

#include "camera.h"
#include "camera_input.h"
#include "framebuffer.h"
#include "frame.h"

#include "scene/state.h"
#include "scene/implicit.h"
#include "scene/object.h"

#include "platform.h"

#include "nhgui.h"

/* 
 * Scene instance 
 */
struct light_scene_instance 
{
	struct light_framebuffer framebuffer;

	struct light_frame_info frame_info;

	struct light_camera_view_state view_state;

	struct light_camera_update_state update_state;

	struct light_scene_state_instance state_instance;
};

struct light_scene_object
{
	uint32_t object_index;

	struct vec3 position;
	struct vec3 rotation;
	struct vec3 scale;
};


void 
light_scene_object_commit(
		struct light_scene_state_instance *instance,
		struct light_scene_object *object,
		uint32_t object_count
);

int
light_scene_initialize(
		struct light_scene_instance *instance,
		struct light_platform *platform,
		struct light_scene_state_build state_build 
);

void 
light_scene_deinitialize(
		struct light_scene_instance *instance
);

int 
light_scene_bind(
		struct light_scene_instance *instance,
		struct light_platform *platform, 
	       	uint32_t width, uint32_t height,
	       	const float deltatime
);


#endif 

