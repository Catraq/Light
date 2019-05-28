#ifndef CAMERA_INPUT_H
#define CAMERA_INPUT_H

#include "math/vec.h"

#include "camera.h"

enum camera_input_action
{
	up,
	down,
	right, 
	left,
	forward, 
	backward,
	length
};

struct camera_input_state
{
	float speed;
	struct vec2 mouse;
};

struct camera_input_result
{
	struct vec3 direction;
};


struct vec3 update_camera_rotation(struct vec2 delta);
struct vec3 camera_input_update_action_up(struct vec3 direction);
struct vec3 camera_input_update_action_down(struct vec3 direction);
struct vec3 camera_input_update_action_forward(struct vec3 direction);
struct vec3 camera_input_update_action_right(struct vec3 direction);
struct camera_input_result camera_input_update_action(const camera_input_action *input_action, const float *input_multiplier, const uint8_t input_action_count, struct vec3 direction);


void camera_input_update(struct camera_input_state *camera_input, struct camera_view_state *camera_view, camera_input_action *input_action, const float *input_multiplier, uint8_t input_count, const vec2 mouse_delta, const float deltatime);

#endif //CAMERA_INPUT_H
