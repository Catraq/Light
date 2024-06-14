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
 * Scene instance 
 */
struct light_scene_instance 
{


	struct light_framebuffer framebuffer;

	struct light_frame_info frame_info;

	struct light_camera_view_state view_state;

	struct light_camera_update_state update_state;

};


int light_scene_initialize(
		struct light_scene_instance *instance,
		struct light_platform *platform
);

void light_scene_deinitialize(struct light_scene_instance *instance);

int light_scene_bind(
		struct light_scene_instance *instance,
		struct light_platform *platform, 
	       	uint32_t width, uint32_t height,
	       	const float deltatime
);


#endif 

