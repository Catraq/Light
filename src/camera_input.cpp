#include "camera_input.h"


struct mat4x4 camera_input_update(struct camera_view_state *camera_view, const float speed,  const vec2 mouse_delta, const float deltatime)
{
	struct vec3 delta = {0.0f,0.0f,0.0f}; 
	const float c = deltatime * speed;
		
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

