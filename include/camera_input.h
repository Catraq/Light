#ifndef CAMERA_INPUT_H
#define CAMERA_INPUT_H

#include "platform.h"
#include "camera.h"


#include "math/mat4x4.h"
#include "math/vec.h"

struct light_camera_update_state
{
	int dummy;
};

int light_camera_input_initialize(struct light_camera_update_state *state);
int light_camera_input_update(struct light_camera_update_state *state, struct light_camera_view_state *camera_view, const float speed, const struct vec2 mouse_delta, const float deltatime);


#endif //CAMERA_INPUT_H
