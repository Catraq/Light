#include "scene.h"

#include "error.h"

void light_scene_object_commit(
		struct light_scene_state_instance *instance,
		struct light_scene_object *object,
		uint32_t object_count
)
{
	struct light_scene_implicit_object object_node[object_count];
	
	for(uint32_t i = 0; i < object_count; i++)
	{

		struct mat4x4 TRS = m4x4mul(
				m4x4rote(object[i].rotation),
				m4x4scl(object[i].scale)
		);

		TRS = m4x4mul(m4x4trs(object[i].position), TRS);
		
		int dummy = 0;
		object_node[i].object_index = object[i].object_index;
		object_node[i].translation = TRS;
		object_node[i].translation_inv = m4x4inv(&TRS, &dummy); 

	}

	light_scene_implicit_commit_objects(
			instance,
			object_node, 
			object_count
	);
}



int light_scene_initialize(
		struct light_scene_instance *instance,
	       	struct light_platform *platform,
		struct light_scene_state_build state_build 
)
{
	int result = 0;

	
	struct light_camera_view_state *view_state = &instance->view_state;
	//Camera attributes 
	const float fov = 3.14f/2.0f;

	view_state->fov = fov;
	view_state->near = 0.1f;
	view_state->far = 100.0f;
	
	//Setup camera. 
	view_state->position = (struct vec3){0.0f, 0.0f, 0.0f};
	view_state->rotation = (struct vec3){0.0f, 0.0f, 0.0f};

	result = light_camera_initialize(view_state);
	CHECK_GL_ERROR();
	if(result < 0){
		fprintf(stderr, "light_camera_initialize() failed. \n");
		return -1;	
	}


	/* Initialize framebuffer */
	uint32_t frame_width = 512, frame_height = 512;
	result = light_framebuffer_initialize(&instance->framebuffer, frame_width, frame_height);
	CHECK_GL_ERROR();
	if(result < 0)
	{
		light_camera_deinitialize(view_state);
		fprintf(stderr, "light_framebuffer_initialize() failed. \n");
		return -1;	
	}
	
	
	result = light_scene_state_initialize(&instance->state_instance, state_build);
	CHECK_GL_ERROR();
	if(result < 0){

		light_camera_deinitialize(view_state);
		light_framebuffer_deinitialize(&instance->framebuffer);

		printf("light_scene_state_initialize(): failed. \n");
		return -1;	
	}

	return 0;
}

void light_scene_deinitialize(struct light_scene_instance *instance)
{

	
	light_camera_initialize(&instance->view_state);
	light_framebuffer_deinitialize(&instance->framebuffer);
	light_scene_state_deinitialize(&instance->state_instance);
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










