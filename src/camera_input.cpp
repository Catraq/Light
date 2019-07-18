#include <limits.h>
#include <stdio.h>

#include "camera_input.h"

int camera_input_initialize(struct camera_update_state *state, const char *device_path)
{
	if(device_path != NULL){
	int result = joystick_ps3_initialize(&state->ps3, device_path, 0);	
	if(result < 0)
		return -1;
	}

	return 0;
}


struct mat4x4 camera_input_update(struct camera_update_state *state, struct camera_view_state *camera_view, const float speed,  const vec2 mouse_delta, const float deltatime)
{
	struct vec3 delta = {0.0f,0.0f,0.0f}; 
	const float c = deltatime * speed;

	/* If result = 1, then there is data */
	struct joystick_ps3 input;
	const uint32_t timeout_usec = 10;
	memset(&input, 0, sizeof input);
	int result = joystick_ps3_input(&state->ps3, &input, timeout_usec);
	if(result > 0)
	{
		float lx = (float)input.axis[JOYSTICK_PS3_AXIS_LEFT_X]/(float)SHRT_MAX;
		float ly = (float)input.axis[JOYSTICK_PS3_AXIS_LEFT_Y]/(float)SHRT_MAX;

		float rx = (float)input.axis[JOYSTICK_PS3_AXIS_RIGHT_X]/(float)SHRT_MAX;
		float ry = (float)input.axis[JOYSTICK_PS3_AXIS_RIGHT_Y]/(float)SHRT_MAX;

	
		struct vec3 ldelta = {ly, lx, 0.0f};
		ldelta = v3scl(ldelta, 70.0f * deltatime);
		camera_view->rotation = v3add(camera_view->rotation, ldelta); 
		
		const float r_deadzone = 0.05;
		if(rx < -r_deadzone || rx > r_deadzone){
			struct vec3 direction = camera_view->rotation;
			struct vec3 result = {0.0f, 0.0f, 0.0f};
			result.x = -sinf(direction.y) * cosf(direction.x);
			result.z = -cosf(direction.y) * cosf(direction.x);
			result.y = -sinf(direction.x);

			result = v3norm(result);
			result = v3scl(result,  -rx*c);
			camera_view->position = v3add(camera_view->position, result);

		}
	}
			
	if(platform_key(GLFW_KEY_W) == PLATFORM_PRESS){

		struct vec3 direction = camera_view->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = -sinf(direction.y) * cosf(direction.x);
		result.z = -cosf(direction.y) * cosf(direction.x);
		result.y = -sinf(direction.x);

		result = v3norm(result);
		result = v3scl(result,  c);
		camera_view->position = v3add(camera_view->position, result);
	}
	
	if(platform_key(GLFW_KEY_S) == PLATFORM_PRESS){

		struct vec3 direction = camera_view->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = sinf(direction.y) * cosf(direction.x);
		result.z = cosf(direction.y) * cosf(direction.x);
		result.y = sinf(direction.x);

		result = v3norm(result);
		result = v3scl(result,  c);
		camera_view->position = v3add(camera_view->position, result);

	}
	if(platform_key(GLFW_KEY_A) == PLATFORM_PRESS){

		struct vec3 direction = camera_view->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = cosf(direction.y);
		result.z = -sinf(direction.y);

		result = v3norm(result);
		result = v3scl(result,  c);
		camera_view->position = v3add(camera_view->position, result);
	}
	if(platform_key(GLFW_KEY_D) == PLATFORM_PRESS){

		struct vec3 direction = camera_view->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = -cosf(direction.y);
		result.z = sinf(direction.y);

		result = v3norm(result);
		result = v3scl(result,  c);
		camera_view->position = v3add(camera_view->position, result);
	}
	
	if(platform_mouse_key(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
		vec3 delta = {mouse_delta.y, mouse_delta.x, 0.0f};
		camera_view->rotation = v3add(camera_view->rotation, delta);
	}

	return camera_view_matrix(camera_view);
}

