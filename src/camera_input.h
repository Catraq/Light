#ifndef CAMERA_INPUT_H
#define CAMERA_INPUT_H

#include "math/vec.h"
#include "math/mat4x4.h"

#include "camera.h"


enum camera_input_action
{
	up,
	down,
	forward, 
	backward,
	stop,
	length
};

struct camera_input_state
{
	float 		speed;
	float 		friction;
	vec3 			velocity;
	vec2 			mouse;
};


struct vec3 update_camera_rotation(vec2 delta)
{
	
	struct vec3 result = {0.0f, 0.0f, 0.0f};
	result.y = delta.x ;
	result.x = delta.y ;
	result = v3scl(result, 2.0f );
	return ( result );
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
	result.x = -sinf( direction.y ) * cosf(direction.x);
	result.z = -cosf( direction.y ) * cosf(direction.x);
	result.y = -sinf( direction.x );
	return (result);
}


struct vec3 camera_input_update_action_backward( struct vec3 direction)
{
	struct vec3 result = {0.0f, 0.0f, 0.0f};
	result.x = sinf( direction.y ) * cosf(direction.x);
	result.z = cosf( direction.y ) * cosf(direction.x);
	result.y = sinf( direction.x );

	return (result);
}

struct camera_input_result
{
	vec3 direction;
	uint8_t stop : 1;
};

struct camera_input_result camera_input_update_action( const camera_input_action *input_action, const float *input_multiplier, const uint8_t input_action_count, struct vec3 direction, struct vec3 velocity)
{
	struct camera_input_result result = {{0.0f,0.0f,0.0f}, 0};

	for(uint8_t i = 0; i < input_action_count; i++)
	{
		
		const float multiplier = input_multiplier[i];
	
		/*	
			Fetch action and do whatever 
		*/
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
			case stop:
				result.stop = 1;
			break;
		}
	}
	
	return (result);
}




void camera_input_update(camera_input_state *camera_input, 
						  camera_view_state *camera_view, 
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
	/*
		V = velocity, D = Direction , S = speed, f = friction, d = deltatime 
		V = d(D - V*f)
	*/
	const float c = deltatime * camera_input->speed;
	const float f = camera_input->friction;
		
	input_result = camera_input_update_action(input_action, input_multiplier, input_count, camera_view->rotation, camera_input->velocity);

	if(!input_result.stop)
	{
		direction = input_result.direction;
		direction = v3scl(direction, c);	
		
		direction = v3sub(direction, camera_input->velocity);
		direction = v3scl(direction, f);

		direction = v3add(direction, camera_input->velocity); 
		camera_view_update(camera_view, direction, delta_rotation);
		camera_input->velocity = direction;
	}
	else
	{
			camera_input->velocity = {};
	}
}




#endif //CAMERA_INPUT_H
