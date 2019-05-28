#include "camera_input.h"


struct vec3 update_camera_rotation(struct vec2 delta)
{
	struct vec3 result = {0.0f, 0.0f, 0.0f};
	result.y = delta.x;
	result.x = delta.y;
	result = v3scl(result, 2.0f);
	return result;
}




struct vec3 camera_input_update_action_up(struct vec3 direction)
{
	//TO remove warnings
	direction;
	
	struct vec3 result = {0,0,0};
	return result;
}


struct vec3 camera_input_update_action_down(struct vec3 direction)
{
	//TO remove warnings
	direction;
	
	struct vec3 result = {0,0,0};
	return result;
}


struct vec3 camera_input_update_action_forward(struct vec3 direction)
{
	struct vec3 result = {0.0f, 0.0f, 0.0f};
	result.x = -sinf(direction.y) * cosf(direction.x);
	result.z = -cosf(direction.y) * cosf(direction.x);
	result.y = -sinf(direction.x);
	return result;
}

struct vec3 camera_input_update_action_backward(struct vec3 direction)
{
	struct vec3 result = {0.0f, 0.0f, 0.0f};
	result.x = sinf(direction.y) * cosf(direction.x);
	result.z = cosf(direction.y) * cosf(direction.x);
	result.y = sinf(direction.x);
	return result;
}

struct vec3 camera_input_update_action_left(struct vec3 direction)
{
	struct vec3 result = {0.0f, 0.0f, 0.0f};

	result.x = cosf(direction.y);
	result.z = -sinf(direction.y);

	return result;
}

struct vec3 camera_input_update_action_right(struct vec3 direction)
{
	struct vec3 result = {0.0f, 0.0f, 0.0f};

	result.x = -cosf(direction.y);
	result.z = sinf(direction.y);

	return result;
}

struct camera_input_result camera_input_update_action(const camera_input_action *input_action, const float *input_multiplier, const uint8_t input_action_count, struct vec3 direction)
{
	struct camera_input_result result = {0.0f,0.0f,0.0f};

	if(input_action_count == 0){
		result.direction = {0.0f, 0.0f, 0.0f};
		return result;
	}
	
	for(uint8_t i = 0; i < input_action_count; i++)
	{
		const float multiplier = input_multiplier[i];
		const camera_input_action action = input_action[i];
		switch(action)
		{

			case up:
				{
					struct vec3 to_add_up = camera_input_update_action_up(direction);
					result.direction = v3add(result.direction, to_add_up);
					result.direction = v3scl(result.direction, multiplier);
				}
				break;
				
			case down:
				{
					struct vec3 to_add_down = camera_input_update_action_down(direction);
					result.direction = v3add(result.direction, to_add_down);
					result.direction = v3scl(result.direction, multiplier);
				}
				break;
			case right:
				{
					struct vec3 to_add_down = camera_input_update_action_right(direction);
					result.direction = v3add(result.direction, to_add_down);
					result.direction = v3scl(result.direction, multiplier);
				}
				break;
			case left:
				{
					struct vec3 to_add_down = camera_input_update_action_left(direction);
					result.direction = v3add(result.direction, to_add_down);
					result.direction = v3scl(result.direction, multiplier);
				}
				break;
				
			case forward:
				{
					struct vec3 to_add_forward = camera_input_update_action_forward(direction);
					result.direction = v3add(result.direction ,to_add_forward);
					result.direction = v3scl(result.direction, multiplier);
				}
				break;
				
			case backward:
				{
					struct vec3 to_add_backward = camera_input_update_action_backward(direction);
					result.direction = v3add(result.direction, to_add_backward);
					result.direction = v3scl(result.direction, multiplier);
				}
				break;
			break;
		}
	}

	result.direction = v3norm(result.direction); 
	return result;
}




void camera_input_update(struct camera_input_state *camera_input, 
						  struct camera_view_state *camera_view, 
						  camera_input_action *input_action, 
						  const float *input_multiplier,
						  uint8_t input_count,  
						  const vec2 mouse_delta, 
						  const float deltatime  )
{
	struct vec3 delta = {0.0f,0.0f,0.0f}; 
	struct vec3 delta_rotation = {mouse_delta.y, mouse_delta.x, 0.0f};
	struct vec3 direction = {0.0f,0.0f,0.0f};
	struct camera_input_result input_result = {};

	const float c = deltatime * camera_input->speed;
		
	input_result = camera_input_update_action(input_action, input_multiplier, input_count, camera_view->rotation);

	direction = input_result.direction;
	direction = v3scl(direction, c);	
	camera_view_update(camera_view, direction, delta_rotation);
}




#
