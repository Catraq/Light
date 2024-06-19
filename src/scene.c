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

void 
light_scene_object_nhgui_edit_initialize(struct light_scene_object_nhgui_edit *edit)
{
	
	edit->gui_object_list = (struct nhgui_object_text_list){
		.char_scroll_per_sec = 1,
		.text_color = {.x = 1.0, .y = 1.0, .z = 1.0},	
		.field_color = {.x = 0.0, .y = 0.0, .z = 0.0},	
		.selected_text_color = {.x = 0.0, .y = 0.0, .z = 0.0},	
		.selected_field_color = {.x = 1.0, .y = 1.0, .z = 1.0},	
	
	};
}

struct nhgui_result
light_scene_object_nhgui_edit(
		struct light_scene_object_nhgui_edit *gui_edit,
		const struct nhgui_context *gui_context,
		const struct nhgui_object_font *gui_font,
		struct nhgui_input *gui_input,
		struct nhgui_result gui_result,
		const float gui_menu_width_mm,
		const float gui_font_mm,
		struct light_scene_object *objects,
		const uint32_t object_count
)
{

	const uint32_t object_name_size = 32;
	char object_name[object_count][object_name_size];
	uint32_t object_name_length[object_count];
	char *object_name_ptr[object_count];

	for(uint32_t i = 0; i < object_count; i++){
		int result = snprintf(object_name[i], object_name_size, "Object(%u)", i);
		if(result < 0){
			fprintf(stderr, "snprintf failed. Check buffer size. \n");	
		}
		else{
			object_name_length[i] = result;
			object_name_ptr[i] = &object_name[i];	
		}
	}


	struct nhgui_render_attribute gui_list_attribute = {
		.width_mm = gui_menu_width_mm,
		.height_mm = gui_font_mm, 
	};


	gui_result = nhgui_object_text_list(
		&gui_edit->gui_object_list,
		gui_context, 
		object_name_ptr,
		object_name_length,
		object_count, 
		gui_font, 
		&gui_list_attribute,
		gui_input, 
		gui_result
	);
	
	gui_result = nhgui_result_dec_y(gui_result);

	if(gui_edit->gui_object_list.selected > 0)
	{


		if(gui_edit->last_selected_index != gui_edit->gui_object_list.selected_index)
		{
			gui_edit->last_selected_index = gui_edit->gui_object_list.selected_index;

			gui_edit->x_input_field = (struct nhgui_object_input_field_float) {};
			gui_edit->y_input_field = (struct nhgui_object_input_field_float) {};
			gui_edit->z_input_field = (struct nhgui_object_input_field_float) {};
			
			gui_edit->x_scale_input_field = (struct nhgui_object_input_field_float) {};
			gui_edit->y_scale_input_field = (struct nhgui_object_input_field_float) {};
			gui_edit->z_scale_input_field = (struct nhgui_object_input_field_float) {};
				
			gui_edit->x_pos_input_field = (struct nhgui_object_input_field_float) {};
			gui_edit->y_pos_input_field = (struct nhgui_object_input_field_float) {};
			gui_edit->z_pos_input_field = (struct nhgui_object_input_field_float) {};

		}	



		uint32_t object_index = gui_edit->gui_object_list.selected_index;

		struct nhgui_render_attribute gui_input_field_attribute = {
			.height_mm = gui_font_mm,	
			.width_mm = 20, 
		};

		const char position_str[] = "Position(x,y,z):";
		struct nhgui_result gui_f_result = nhgui_object_font_text(
			gui_context, 
			gui_font,
			position_str, 
			sizeof(position_str),
			&gui_input_field_attribute,
			gui_input, 
			gui_result
		);

		gui_result = nhgui_result_inc_x(gui_f_result);

		struct nhgui_result gui_p_result = nhgui_object_input_field_float(
				&gui_edit->x_pos_input_field,
				gui_context,
				gui_font,
				&gui_input_field_attribute,
				gui_input, 
				gui_result,
				&objects[object_index].position.x	
		);
		
		gui_result = nhgui_result_inc_x(gui_p_result);

		gui_result = nhgui_object_input_field_float(
				&gui_edit->y_pos_input_field,
				gui_context,
				gui_font,
				&gui_input_field_attribute, 
				gui_input, 
				gui_result,
				&objects[object_index].position.y	
		);

		gui_result = nhgui_result_inc_x(gui_result);

		gui_result = nhgui_object_input_field_float(
				&gui_edit->z_pos_input_field,
				gui_context,
				gui_font,
				&gui_input_field_attribute, 
				gui_input, 
				gui_result,
				&objects[object_index].position.z	
		);

		gui_result = nhgui_result_dec_y(gui_result);
		gui_result = nhgui_result_rewind_x_to(gui_result, gui_f_result);


		const char angle_str[] = "Angle(x,y,z):";
		gui_result = nhgui_object_font_text(
			gui_context, 
			gui_font,
			angle_str, 
			sizeof(angle_str),
			&gui_input_field_attribute,
			gui_input, 
			gui_result
		);

		gui_result = nhgui_result_rewind_x_to(gui_result, gui_p_result);

		gui_result = nhgui_object_input_field_float(
				&gui_edit->x_input_field,
				gui_context,
				gui_font,
				&gui_input_field_attribute,
				gui_input, 
				gui_result,
				&objects[object_index].rotation.x	
		);
		
		gui_result = nhgui_result_inc_x(gui_result);

		gui_result = nhgui_object_input_field_float(
				&gui_edit->y_input_field,
				gui_context,
				gui_font,
				&gui_input_field_attribute, 
				gui_input, 
				gui_result,
				&objects[object_index].rotation.y	
		);

		gui_result = nhgui_result_inc_x(gui_result);

		gui_result = nhgui_object_input_field_float(
				&gui_edit->z_input_field,
				gui_context,
				gui_font,
				&gui_input_field_attribute, 
				gui_input, 
				gui_result,
				&objects[object_index].rotation.z	
		);

		gui_result = nhgui_result_dec_y(gui_result);
		gui_result = nhgui_result_rewind_x_to(gui_result, gui_f_result);

		const char scale_str[] = "Scale(x,y,z):";
		gui_result = nhgui_object_font_text(
			gui_context, 
			gui_font,
			scale_str, 
			sizeof(scale_str),
			&gui_input_field_attribute,
			gui_input, 
			gui_result
		);

		gui_result = nhgui_result_rewind_x_to(gui_result, gui_p_result);

		gui_result = nhgui_object_input_field_float(
				&gui_edit->x_scale_input_field,
				gui_context,
				gui_font,
				&gui_input_field_attribute,
				gui_input, 
				gui_result,
				&objects[object_index].scale.x
		);
		gui_result = nhgui_result_inc_x(gui_result);

		gui_result = nhgui_object_input_field_float(
				&gui_edit->y_scale_input_field,
				gui_context,
				gui_font,
				&gui_input_field_attribute, 
				gui_input, 
				gui_result,
				&objects[object_index].scale.y	
		);

		gui_result = nhgui_result_inc_x(gui_result);

		gui_result = nhgui_object_input_field_float(
				&gui_edit->z_scale_input_field,
				gui_context,
				gui_font,
				&gui_input_field_attribute, 
				gui_input, 
				gui_result,
				&objects[object_index].scale.z	
		);
	}

	return gui_result;
}




int light_scene_initialize(
		struct light_scene_instance *instance,
	       	struct light_platform *platform,
		struct light_scene_state_build state_build 
)
{
	int result = 0;

	struct light_frame_info frame_info = light_frame_info_update(
			NULL,
			platform
	);


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
		printf(stderr, "light_camera_initialize() failed. \n");
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
	
	
	struct light_scene_state_instance state_instance;
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










