#ifndef LIGHT_SCENE_GUI_H
#define LIGHT_SCENE_GUI_H

#include "scene.h"
#include "nhgui.h"

struct light_scene_object_node_nhgui_edit
{
	uint32_t last_selected_index;

	struct nhgui_object_text_list gui_object_list;
	struct nhgui_object_text_list gui_function_list;

	struct nhgui_object_input_field_float x_input_field;
	struct nhgui_object_input_field_float y_input_field;
	struct nhgui_object_input_field_float z_input_field;
	
	struct nhgui_object_input_field_float x_scale_input_field;
	struct nhgui_object_input_field_float y_scale_input_field;
	struct nhgui_object_input_field_float z_scale_input_field;
		
	struct nhgui_object_input_field_float x_pos_input_field;
	struct nhgui_object_input_field_float y_pos_input_field;
	struct nhgui_object_input_field_float z_pos_input_field;

};


void 
light_scene_object_node_nhgui_edit_initialize(
		struct light_scene_object_node_nhgui_edit *edit
);

struct nhgui_result
light_scene_object_node_nhgui_edit(
		struct light_scene_object_node_nhgui_edit *gui_edit,
		struct light_scene_instance *scene,
		const struct nhgui_context *gui_context,
		const struct nhgui_object_font *gui_font,
		struct nhgui_input *gui_input,
		struct nhgui_result gui_result,
		const float gui_menu_width_mm,
		const float gui_font_mm,
		struct light_scene_object_node *objects,
		const uint32_t object_count
);

#endif //LIGHT_SCENE_GUI_H
