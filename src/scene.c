#include "scene.h"

#include "error.h"

static void light_scene_view_initialize(struct light_camera_view_state *view_state, uint32_t width, uint32_t height)
{

	light_camera_initialize(view_state);

	//Camera attributes 
	const float fov = 3.14f/2.0f;

	view_state->fov = fov;
	view_state->near = 0.1f;
	view_state->far = 100.0f;
	
	//Setup camera. 
	view_state->position = (struct vec3){0.0f, 0.0f, 0.0f};
	view_state->rotation = (struct vec3){0.0f, 0.0f, 0.0f};

}



void light_scene_deinitialize(struct light_scene_instance *instance)
{

	
	light_framebuffer_deinitialize(&instance->framebuffer);

}


int light_scene_initialize(
		struct light_scene_instance *instance,
	       	struct light_platform *platform
)
{
	int result = 0;


	/* Platform Joystick initialization */
	result = light_camera_input_initialize(&instance->update_state);
	if(result < 0){
		fprintf(stderr, "Error: could not initialize camera input.\n");
		return -1;	
	}


	struct light_frame_info frame_info = light_frame_info_update(
			NULL,
			platform
	);

	light_scene_view_initialize(&instance->view_state, frame_info.width, frame_info.height);
	CHECK_GL_ERROR();

	/* Initialize framebuffer */
	uint32_t frame_width = 512, frame_height = 512;
	result = light_framebuffer_initialize(&instance->framebuffer, frame_width, frame_height);
	CHECK_GL_ERROR();

	if(result < 0)
	{
		fprintf(stderr, "light_framebuffer_initialize() failed. \n");
		return -1;	
	}

	return 0;
}

int light_scene_bind(
		struct light_scene_instance *instance,
	       	struct light_platform *platform,
	       	uint32_t width, uint32_t height,
	       	const float deltatime)
{	
	struct light_frame_info frame_result = light_frame_info_update(
			&instance->frame_info,
			platform
	);

	struct vec2 delta_mouse = light_frame_info_mouse_delta(&frame_result, &instance->frame_info);
	instance->frame_info = frame_result; 

	light_camera_input_update(
			&instance->update_state,
		       	&instance->view_state,
		       	platform,
		       	10.0f,
		       	delta_mouse, 
			deltatime
	);

	CHECK_GL_ERROR();

	light_camera_view_matrix(&instance->view_state, width, height);
	CHECK_GL_ERROR();

	/* Resize and clear framebuffer */	
	light_framebuffer_resize(&instance->framebuffer, width, height);
	CHECK_GL_ERROR();

	glViewport(0,0, width, height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	light_camera_buffer_bind(&instance->view_state);
	CHECK_GL_ERROR();

	return 0;
}










