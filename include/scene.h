#ifndef SCENE_INSTANCE_H
#define SCENE_INSTANCE_H

#include <stdio.h>

#include "math/vec.h"
#include "math/mat4x4.h"

#include "camera.h"
#include "camera_input.h"
#include "framebuffer.h"
#include "frame.h"

#include "scene/state.h"
#include "scene/object.h"

#include "platform.h"

#include "nhgui.h"

/* 
 * Scene instance 
 */
struct light_scene_instance 
{
	struct light_framebuffer framebuffer;

	struct light_frame_info frame_info;

	struct light_camera_view_state view_state;

	struct light_camera_update_state update_state;

	struct light_scene_state_instance state_instance;
};

struct light_scene_object
{
	uint32_t object_index;

	struct vec3 position;
	struct vec3 rotation;
	struct vec3 scale;
};

struct light_scene_object_nhgui_edit
{
	uint32_t last_selected_index;

	struct nhgui_object_text_list gui_object_list;

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
light_scene_object_commit(
		struct light_scene_state_instance *instance,
		struct light_scene_object *object,
		uint32_t object_count
);

void 
light_scene_object_nhgui_edit_initialize(
		struct light_scene_object_nhgui_edit *edit
);

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
);

int
light_scene_initialize(
		struct light_scene_instance *instance,
		struct light_platform *platform,
		struct light_scene_state_build state_build 
);

void 
light_scene_deinitialize(
		struct light_scene_instance *instance
);

int 
light_scene_bind(
		struct light_scene_instance *instance,
		struct light_platform *platform, 
	       	uint32_t width, uint32_t height,
	       	const float deltatime
);


#endif 

