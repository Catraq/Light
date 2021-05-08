

void scene_view_initialize(struct camera_view_state *view_state, GLuint buffer_base_index, uint32_t width, uint32_t height)
{

	camera_initialize(view_state, buffer_base_index);

	//Camera attributes 
	const float fov = 3.14f/3.0f;
	const float far = 1000.0f;
	const float close = 1.0f;
	float ratio = ((float)width / (float)height); 

	view_state->near = close;
	view_state->far = far;
	view_state->fov = fov;
	
	//Setup camera. 
	view_state->position = (struct vec3){0.0f, 0.0f, 0.0f};
	view_state->rotation = (struct vec3){0.0f, 0.0f, 0.0f};

}


struct scene_object
{
	GLuint 			object_program;
	uint32_t 		object_instance_count;
	struct vertex_instance 	object_instance;
};

struct scene_instance 
{
	struct frame_info frame_info;
	struct camera_view_state view_state;
	struct camera_update_state update_state;

		
	uint32_t object_count;
	struct scene_object *object;
};

int scene_initialize(struct scene_instance *instance, struct scene_object *object, uint32_t object_count)
{
	int result = 0;

	/* Platform Joystick initialization */
	result = camera_input_initialize(&instance->update_state);
	if(result < 0){
		fprintf(stderr, "Error: could not initialize camera input.\n");
		return -1;	
	}


	/* Initlaize camera. */
	const GLuint uniform_buffer_block_location = 0;

	struct frame_info frame_info = frame_info_update(NULL);
	scene_view_initialize(&instance->view_state, uniform_buffer_block_location, frame_info.width, frame_info.height);

	instance->object = object;
	instance->object_count = object_count;
	
	for(uint32_t i = 0; i < instance->object_count; i++)
	{
		camera_buffer_bind(&instance->view_state, instance->object[i].object_program);
	}
		

	return 0;
}

int scene_render(struct scene_instance *instance, uint32_t width, uint32_t height, const float deltatime)
{	
	struct frame_info frame_result = frame_info_update(&instance->frame_info);
	struct vec2 delta_mouse = frame_info_mouse_delta(&frame_result, &instance->frame_info);
	instance->frame_info = frame_result; 

	camera_input_update(&instance->update_state, &instance->view_state, 10.0f, delta_mouse, deltatime);

	camera_view_matrix(&instance->view_state, width, height);

	glViewport(0,0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	

	for(uint32_t i = 0; i < instance->object_count; i++)
	{
		if(instance->object[i].object_instance_count > 0)
		{
			glUseProgram(instance->object[i].object_program);
			vertex_instance_draw(&instance->object[i].object_instance, instance->object[i].object_instance_count);
		}

	}

	return 0;
}










