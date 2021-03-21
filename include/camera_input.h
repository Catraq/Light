#ifndef CAMERA_INPUT_H
#define CAMERA_INPUT_H

#include "platform.h"
#include "camera.h"


#include <joystick.h>
#include <joystick_map.h>

#include "math/mat4x4.h"
#include "math/vec.h"

struct camera_update_state
{
	struct joystick_device device;
	struct joystick_map map;
};

int camera_input_initialize(struct camera_update_state *state);
int camera_input_update(struct camera_update_state *state, struct camera_view_state *camera_view, const float speed, const vec2 mouse_delta, const float deltatime);


#endif //CAMERA_INPUT_H
