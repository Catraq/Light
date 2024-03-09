#include <limits.h>
#include <stdio.h>

#include "camera_input.h"


int light_camera_input_initialize(struct light_camera_update_state *state)
{

	return 0;
}


int light_camera_input_update(struct light_camera_update_state *state, struct light_camera_view_state *view_state, const float speed,  const struct vec2 mouse_delta, const float deltatime)
{
	struct vec3 delta = {0.0f,0.0f,0.0f}; 
	const float speed_dt = deltatime * speed;
			
	if(light_platform_key(GLFW_KEY_W) == PLATFORM_PRESS){

		struct vec3 direction = view_state->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = -sinf(direction.y) * cosf(direction.x);
		result.z = -cosf(direction.y) * cosf(direction.x);
		result.y = -sinf(direction.x);

		result = v3norm(result);
		result = v3scl(result,  speed_dt);
		view_state->position = v3add(view_state->position, result);
	}
	
	if(light_platform_key(GLFW_KEY_S) == PLATFORM_PRESS){

		struct vec3 direction = view_state->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = sinf(direction.y) * cosf(direction.x);
		result.z = cosf(direction.y) * cosf(direction.x);
		result.y = sinf(direction.x);

		result = v3norm(result);
		result = v3scl(result,  speed_dt);
		view_state->position = v3add(view_state->position, result);

	}
	if(light_platform_key(GLFW_KEY_A) == PLATFORM_PRESS){

		struct vec3 direction = view_state->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = cosf(direction.y);
		result.z = -sinf(direction.y);

		result = v3norm(result);
		result = v3scl(result,  speed_dt);
		view_state->position = v3add(view_state->position, result);
	}
	if(light_platform_key(GLFW_KEY_D) == PLATFORM_PRESS){

		struct vec3 direction = view_state->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = -cosf(direction.y);
		result.z = sinf(direction.y);

		result = v3norm(result);
		result = v3scl(result,  speed_dt);
		view_state->position = v3add(view_state->position, result);
	}
	
	if(light_platform_mouse_key(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
		struct vec3 delta = {mouse_delta.y, mouse_delta.x, 0.0f};
		view_state->rotation = v3add(view_state->rotation, delta);
	}
	
	return 0;
}

