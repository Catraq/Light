#ifndef CAMERA_INPUT_H
#define CAMERA_INPUT_H

#include "platform.h"
#include "camera.h"

#include "math/mat4x4.h"
#include "math/vec.h"



struct mat4x4 camera_input_update(struct camera_view_state *camera_view, const float speed, const vec2 mouse_delta, const float deltatime);


#endif //CAMERA_INPUT_H
