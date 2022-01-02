#include <limits.h>
#include <stdio.h>

#include "camera_input.h"

#define CAMERA_INPUT_AXIS 4
#define CAMERA_INPUT_BUTTON 2

#define APPLICATION_INPUT_AXIS 4
#define APPLICATION_INPUT_BUTTON 2

int camera_input_initialize(struct camera_update_state *state)
{

	struct joystick_input_requirement  joystick_input_req = {
		.requirement_axis_count_min 	= CAMERA_INPUT_AXIS,
		.requirement_button_count_min 	=  CAMERA_INPUT_BUTTON,
		.requirement_axis_count_max 	= JOYSTICK_AXIS_MAX,
		.requirement_button_count_max 	= JOYSTICK_AXIS_MAX,
	};

	
	const size_t input_attrib_list_count = 8; 
	struct joystick_input_attrib input_attrib_list[input_attrib_list_count];
	memset(&input_attrib_list, 0, sizeof input_attrib_list);

	size_t number_of_joysticks = joystick_device_identify_by_requirement(&joystick_input_req, input_attrib_list, input_attrib_list_count);
	if(number_of_joysticks == 0){
		fprintf(stderr, "Could not find any joystick device \n");		
		return 0;
	}


	const char *joystick_device_path = (const char *)input_attrib_list[0].joystick_device_path;
	joystick_input_attrib_print(&input_attrib_list[0], stdout);

	int result = joystick_device_open(&state->device, &joystick_input_req, joystick_device_path);
	if(result < 0)
	{
		fprintf(stderr, "Could not open joystick device \n");		
		return 0;	
	}


	/* Determined by the input joystick HW. */	
	const uint32_t linear_inputs = joystick_device_axis_count(&state->device);

	/* Determined by the application requirement. */
	const uint32_t outputs = APPLICATION_INPUT_AXIS; 


	joystick_map_create(&state->map, linear_inputs, outputs);
	uint32_t camera_index_to_input[CAMERA_INPUT_AXIS] = {0, 1, 2, 5};


	//TODO: MAP by controller. 
	for(uint32_t i = 0; i < CAMERA_INPUT_AXIS; i++)
	{	
		uint32_t output_channel_count = APPLICATION_INPUT_AXIS;
		float output_channels[APPLICATION_INPUT_AXIS] = {0.0f, 0.0f, 0.0f, 0.0f};
		output_channels[i] = 1.0f;


		uint32_t input_index = camera_index_to_input[i];


		joystick_map_transform(&state->map, input_index, output_channels, output_channel_count);
	}

	return 0;
}


int camera_input_update(struct camera_update_state *state, struct camera_view_state *camera_view, const float speed,  const struct vec2 mouse_delta, const float deltatime)
{
	struct vec3 delta = {0.0f,0.0f,0.0f}; 
	const float speed_dt = deltatime * speed;

#if 0
	int result = joystick_device_poll(&state->device);
	if(result < 0)
	{
		/* Disconnected */	
	
	}
	else if(result > 0)
	{
		float output[APPLICATION_INPUT_AXIS];
		uint32_t output_count = APPLICATION_INPUT_AXIS;
		joystick_map_translate(&state->map, &state->device, output, output_count);


		struct vec3 camera_pos_delta = {output[3], output[2], 0};
		struct vec3 camera_rot_delta = v3scl((struct vec3){output[1], output[0], 0.0f}, speed_dt);
		camera_view->rotation = v3add(camera_view->rotation, camera_rot_delta); 
		
		float rx = camera_pos_delta.x;	
		const float r_deadzone = 0.5;
		if(rx < -r_deadzone || rx > r_deadzone){
			struct vec3 direction = camera_view->rotation;
			struct vec3 result = {0.0f, 0.0f, 0.0f};
			result.x = -sinf(direction.y) * cosf(direction.x);
			result.z = -cosf(direction.y) * cosf(direction.x);
			result.y = -sinf(direction.x);

			result = v3norm(result);
			result = v3scl(result,  -rx*speed_dt);
			
			camera_view->position = v3add(camera_view->position, result);
		}

	}
#endif 
			
	if(platform_key(GLFW_KEY_W) == PLATFORM_PRESS){

		struct vec3 direction = camera_view->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = -sinf(direction.y) * cosf(direction.x);
		result.z = -cosf(direction.y) * cosf(direction.x);
		result.y = -sinf(direction.x);

		result = v3norm(result);
		result = v3scl(result,  speed_dt);
		camera_view->position = v3add(camera_view->position, result);
	}
	
	if(platform_key(GLFW_KEY_S) == PLATFORM_PRESS){

		struct vec3 direction = camera_view->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = sinf(direction.y) * cosf(direction.x);
		result.z = cosf(direction.y) * cosf(direction.x);
		result.y = sinf(direction.x);

		result = v3norm(result);
		result = v3scl(result,  speed_dt);
		camera_view->position = v3add(camera_view->position, result);

	}
	if(platform_key(GLFW_KEY_A) == PLATFORM_PRESS){

		struct vec3 direction = camera_view->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = cosf(direction.y);
		result.z = -sinf(direction.y);

		result = v3norm(result);
		result = v3scl(result,  speed_dt);
		camera_view->position = v3add(camera_view->position, result);
	}
	if(platform_key(GLFW_KEY_D) == PLATFORM_PRESS){

		struct vec3 direction = camera_view->rotation;
		struct vec3 result = {0.0f, 0.0f, 0.0f};
		result.x = -cosf(direction.y);
		result.z = sinf(direction.y);

		result = v3norm(result);
		result = v3scl(result,  speed_dt);
		camera_view->position = v3add(camera_view->position, result);
	}
	
	if(platform_mouse_key(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
		struct vec3 delta = {mouse_delta.y, mouse_delta.x, 0.0f};
		camera_view->rotation = v3add(camera_view->rotation, delta);
	}
	
	return 0;
}
