#ifndef CAMERA_INPUT_H
#define CAMERA_INPUT_H

#include "platform.h"
#include "camera.h"

#include <joystick_ps3.h>

#include "math/mat4x4.h"
#include "math/vec.h"

struct camera_update_state
{
	struct joystick_ps3_context ps3;	
};

int camera_input_initialize(struct camera_update_state *state, const char *device_path);
struct mat4x4 camera_input_update(struct camera_update_state *state, struct camera_view_state *camera_view, const float speed, const vec2 mouse_delta, const float deltatime);


#endif //CAMERA_INPUT_H
