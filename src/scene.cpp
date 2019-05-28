#include "scene.h"



struct frame_info_update_result
{
	struct vec2 mouse_delta;
};

static struct frame_info_update_result frame_info_update(struct frame_info *info)
{
	struct frame_info_update_result result = {};
	
	struct vec2 mouse;
	platform_mouse(&mouse);
	platform_resolution(&info->width, &info->height);
	
	const float half_width_f  = (float)info->width/2.0f;
	const float half_height_f = (float)info->height/2.0f;
	
	struct vec2 tmp;
	tmp.x = -(half_width_f/info->width -  mouse.x/info->width );
	tmp.y =  (half_height_f/info->height - mouse.y/info->height );
	
	result.mouse_delta = v2sub(info->mouse, tmp); 
	result.mouse_delta = v2scl(result.mouse_delta, 2.0f); 
	info->mouse = tmp;
	
	return result;
}


static void scene_view_initialize(struct camera_input_state *input_state, struct camera_view_state *view_state, uint32_t width, uint32_t height)
{
	//Camera attributes 
	const float fov = 3.14f/3.0f;
	const float far = 1000.0f;
	const float close = 1.0f;
	float ratio = ((float)width / (float)height); 

	view_state->near = close;
	view_state->far = far;
	view_state->fov = fov;
	
	//Setup camera. 
	camera_view_projection(view_state, width, height);
	view_state->position = (vec3){0.0f, 0.0f, 5.0f};
	view_state->rotation = (vec3){0.0f, 0.0f, 0.0f};

	input_state->speed = 200.0f;
	
}


static mat4x4 scene_update_view(struct camera_input_state *camera_input, struct camera_view_state *camera_view, struct frame_info_update_result *frame_info_result, const float& deltatime)
{
	uint8_t action_count = 0;
	enum camera_input_action actions[camera_input_action::length];
	float multiplier[camera_input_action::length];
	struct vec2 delta_rotation = {};	
	
	if(platform_key(GLFW_KEY_W) == PLATFORM_PRESS){
		actions[action_count] = camera_input_action::forward;
		multiplier[action_count] = 1.0f;
		action_count++;
	}
	
	else if(platform_key(GLFW_KEY_S) == PLATFORM_PRESS){
		actions[action_count] = camera_input_action::backward;
		multiplier[action_count] = 1.0f;
		action_count++;
	}
	else if(platform_key(GLFW_KEY_A) == PLATFORM_PRESS){
		actions[action_count] = camera_input_action::left;
		multiplier[action_count] = 1.0f;
		action_count++;
	}
	else if(platform_key(GLFW_KEY_D) == PLATFORM_PRESS){
		actions[action_count] = camera_input_action::right;
		multiplier[action_count] = 1.0f;
		action_count++;
	}
	
	if(platform_mouse_key(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
		delta_rotation = frame_info_result->mouse_delta;
	}
	
	camera_input_update(camera_input, camera_view, actions, multiplier, action_count, delta_rotation, deltatime);
	return camera_view_matrix(camera_view);
}


int scene_initialize(struct scene_instance *instance)
{
	int result = 0;

	vertex_buffer_initialize(&instance->vertex_buffer);
	physic_initialize(&instance->physic);
	renderer_initialize(&instance->renderer);

	
	frame_info_update(&instance->frame_info);
	scene_view_initialize(&instance->input_state, &instance->view_state, instance->frame_info.width, instance->frame_info.height);
	
	return result;
}


int scene_render(struct scene_instance *scene, const float deltatime)
{
	struct frame_info_update_result frame_result = frame_info_update(&scene->frame_info);

	camera_view_projection(&scene->view_state, scene->frame_info.width, scene->frame_info.height);
	struct mat4x4 view = scene_update_view(&scene->input_state, &scene->view_state, &frame_result, deltatime);


	renderer_render_begin(&scene->renderer, &view, scene->frame_info.width, scene->frame_info.height);

	return 1;

}


