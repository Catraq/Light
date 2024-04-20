

void light_scene_view_initialize(struct light_camera_view_state *view_state, GLuint buffer_base_index, uint32_t width, uint32_t height)
{

	light_camera_initialize(view_state, buffer_base_index);

	//Camera attributes 
	const float fov = 3.14f/3.0f;

	view_state->fov = fov;
	
	//Setup camera. 
	view_state->position = (struct vec3){0.0f, 0.0f, 0.0f};
	view_state->rotation = (struct vec3){0.0f, 0.0f, 0.0f};

}


struct light_scene_instance_build
{
	uint32_t sphere_count;

	uint32_t cylinder_count;

	uint32_t box_count;
};

struct light_scene_instance 
{
	struct light_framebuffer framebuffer;

	struct light_frame_info frame_info;

	struct light_camera_view_state view_state;

	struct light_camera_update_state update_state;
};

int light_scene_initialize(struct light_scene_instance *instance)
{
	int result = 0;

	/* Platform Joystick initialization */
	result = light_camera_input_initialize(&instance->update_state);
	if(result < 0){
		fprintf(stderr, "Error: could not initialize camera input.\n");
		return -1;	
	}


	/* Opengl camera uniform block location. */
	const GLuint uniform_buffer_block_location = 0;

	struct light_frame_info frame_info = light_frame_info_update(NULL);
	light_scene_view_initialize(&instance->view_state, uniform_buffer_block_location, frame_info.width, frame_info.height);

	/* Initialize framebuffer */
	uint32_t frame_width = 512, frame_height = 512;
	light_framebuffer_initialize(&instance->framebuffer, frame_width, frame_height);
	
	return 0;
}

int light_scene_bind(struct light_scene_instance *instance, GLint program)
{
	
	return light_camera_buffer_bind(&instance->view_state, program);
}

int light_scene_update(struct light_scene_instance *instance, uint32_t width, uint32_t height, const float deltatime)
{	
	struct light_frame_info frame_result = light_frame_info_update(&instance->frame_info);
	struct vec2 delta_mouse = light_frame_info_mouse_delta(&frame_result, &instance->frame_info);
	instance->frame_info = frame_result; 

	light_camera_input_update(&instance->update_state, &instance->view_state, 10.0f, delta_mouse, deltatime);

	light_camera_view_matrix(&instance->view_state, width, height);

	/* Resize and clear framebuffer */	
	light_framebuffer_resize(&instance->framebuffer, width, height);
	
	glViewport(0,0, width, height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	return 0;
}










