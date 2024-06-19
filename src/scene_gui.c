#include "scene_gui.h"


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
			object_name_ptr[i] = object_name[i];	
		}
	}


	struct nhgui_render_attribute gui_list_attribute = {
		.width_mm = gui_menu_width_mm,
		.height_mm = gui_font_mm, 
	};


	gui_result = nhgui_object_text_list(
		&gui_edit->gui_object_list,
		gui_context, 
		(const char **)object_name_ptr,
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


