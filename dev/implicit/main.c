#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <sys/time.h>

#include "nhgui.h"
#include "nhgui_glfw.h"

#include "math/vec.h"
#include "math/mat4x4.h"
#include "platform.h"

#include "scene.h"
#include "scene/state.h"
#include "scene/object.h"
#include "scene/implicit.h"
#include "scene/particle.h"
#include "scene/particle_emitter.h"

#include "misc/file.h"


#include "camera.h"
#include "camera_input.h"

#include "framebuffer.h"
#include "frame.h"
#include "surface.h"
#include "error.h"

#include "physic/physic.c"
#include "physic/physic_rope.c"




int main(int args, char *argv[])
{
	int result;

	
	/* opengl platform initialization */
	struct light_platform platform;
	result = light_platform_initialize(&platform);
	if(result < 0)
	{
		fprintf(stderr, "error: could not initialize platform.\n");
		exit(EXIT_FAILURE);
	}



	struct nhgui_context gui_context;
	result = nhgui_context_initialize(
			&gui_context,
			platform.screen_resolution_x_pixel, 	
			platform.screen_resolution_y_pixel, 	
			platform.screen_width_mm,
			platform.screen_height_mm
	);

	if(result < 0)
	{
		fprintf(stderr, "nhgui_context_initialize() failed. \n");
		exit(EXIT_FAILURE);
	}


	glfwSetInputMode(platform.window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
	glfwSetCharCallback(platform.window, nhgui_glfw_char_callback);

	struct nhgui_object_font gui_font = {};
	struct nhgui_render_attribute gui_font_attribute = {
		.height_mm = 10,	
	};
	const char *font_filename = "../data/UbuntuMono-R.ttf";

	{
		struct nhgui_object_font_freetype font_freetype;
		result = nhgui_object_font_freetype_initialize(&font_freetype);
		if(result < 0){
			fprintf(stderr, "nhgui_object_font_freetype_initialize() failed. \n");
			return -1;
		}
		
		result = nhgui_object_font_freetype_characters_initialize(
				&font_freetype,
				&gui_context,
				&gui_font_attribute,
				&gui_font, 
				font_filename
		);

		if(result < 0)
		{
			fprintf(stderr, "nhgui_object_font_freetype_characters_initialize() failed. \n");
			return -1;
		}

		nhgui_object_font_freetype_deinitialize(&font_freetype);
	}
	
	struct nhgui_window gui_window = {};
	struct light_scene_object_nhgui_edit object_nhgui_edit = {};
	struct nhgui_glfw_frame gui_frame = nhgui_frame_create(platform.window);


	light_scene_object_nhgui_edit_initialize(&object_nhgui_edit);

	/*create a quad as render surface */	
	struct light_surface_textured quad_surface;
	result = light_surface_textured_initialize(&quad_surface);
	if(result <0)
	{
		printf("light_surface_textured_initialize(): failed. \n");
		exit(EXIT_FAILURE);	
	}
	CHECK_GL_ERROR();


	struct light_scene_state_build state_build = {

		.implicit_build = {
			.object_node_count = 10,
			.light_count = 1,
		},
		.particle_build = {
			.emitter_count = 8,
			.emitter_particle_count = 64,	
		},
		.particle_emitter_build  = {
			.emitter_normal_count = 2,
		},
	};

	struct light_scene_instance scene;
	result = light_scene_initialize(
			&scene,
		       	&platform,
			state_build
	);

	if(result < 0)
	{
		printf("light_scene_initialize(): failed. \n");
		exit(EXIT_FAILURE);
	}


	
	const uint32_t light_count = 1;
	struct light_scene_light_light_instance light_instance[light_count];
	for(uint32_t i = 0; i < light_count; i++)
	{
		light_instance[i].position = (struct vec3){.x=0.0f, 10.0f, 10.0};
	}

	light_instance[0].color = (struct vec3){.x=1.0, .y=0.1f, .z=1.0f};

	light_scene_implicit_commit_light(&scene.state_instance, light_instance, light_count);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	


	const uint32_t object_size = 10;
	const uint32_t object_count = 3;

	struct light_scene_object objects[object_size];
	memset(objects, 0, sizeof(objects));

	objects[0].position = (struct vec3){.x=0, .y=0, .z=10};
	objects[0].scale= (struct vec3){.x=1, .y=1, .z=1};
	objects[0].object_index = 0;

	objects[1].position = (struct vec3){.x=3, .y=0, .z=10};
	objects[1].scale= (struct vec3){.x=1, .y=1, .z=1};
	objects[1].object_index = 1;

	objects[2].position = (struct vec3){.x=6, .y=0, .z=10};
	objects[2].scale= (struct vec3){.x=1, .y=1, .z=1};
	objects[2].object_index = 2;

	light_scene_object_commit(&scene.state_instance, objects, object_count);




	light_scene_state_bind(&scene.state_instance);

	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	float 		fps_time;
	struct timeval 	fps_time_last;
	struct timeval 	fps_time_curr;

	gettimeofday(&fps_time_last, NULL);
	fps_time_curr = fps_time_last;

	float test = 0.0f;	

	while(!light_platform_exit(&platform))
    	{
		struct timeval fps_time_curr_tmp;
		gettimeofday(&fps_time_curr_tmp, NULL);	
		float deltatime = fps_time_curr_tmp.tv_sec - fps_time_curr.tv_sec + (float)(fps_time_curr_tmp.tv_usec - fps_time_curr.tv_usec)/(1000.0f*1000.0f);
		fps_time_curr = fps_time_curr_tmp;
		fps_time = fps_time + deltatime;

		fps_frame_count++;
		if(fps_time> fps_sample_interval)
		{
			uint32_t frames_per_sec = (float)fps_frame_count/fps_time;
			gettimeofday(&fps_time_last, NULL);

			printf("5s fps average(%u frames): %u \n", fps_frame_count, frames_per_sec);

			fps_time = 0.0f;
			fps_frame_count  = 0;

		}

		uint32_t width, height;	
		light_platform_window_resolution(&platform, &width, &height);

		light_scene_bind(&scene, &platform, width, height, deltatime);
		light_scene_state_dispatch(&scene.state_instance, &scene.framebuffer, width, height, deltatime);



		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		struct nhgui_input gui_input = nhgui_glfw_frame_begin(
				&gui_frame,
			       	platform.window
		);
	
		float menu_width_mm = 80;
		float menu_font_mm = 3;

		float pixel_per_mm = (float)gui_context.screen_resolution_y/(float)gui_context.screen_height_mm;
		uint32_t width_offset_pixel = menu_width_mm * pixel_per_mm;
		width_offset_pixel = width_offset_pixel > gui_input.width_pixel ? gui_input.width_pixel : width_offset_pixel;
		glViewport(width_offset_pixel, 0, width-width_offset_pixel, height);
		light_surface_textured_render(&quad_surface, scene.framebuffer.color_texture);

		
		
		/* GUI code */

		glViewport(0, 0, width, height);

		/* Make the window 100 mm by window height */	
		struct nhgui_render_attribute gui_window_attribute = {
			.width_mm = menu_width_mm,
			.height_mm = gui_context.screen_height_mm * (float)gui_input.height_pixel/(float)gui_context.screen_resolution_y,
		};

		struct nhgui_render_attribute gui_list_attribute = {
			.width_mm = menu_width_mm,
			.height_mm = menu_font_mm, 
		};

		
		/* Start point of the rendering */
		struct nhgui_result result = {
			.y_mm = gui_context.screen_height_mm * (float)gui_input.height_pixel/(float)gui_context.screen_resolution_y,
			.y_min_mm = gui_context.screen_height_mm * (float)gui_input.height_pixel/(float)gui_context.screen_resolution_y
		};

		result = nhgui_window_begin(
			&gui_window,
			&gui_context,
			&gui_window_attribute,
			&gui_input,
			result
		);

		
		result = light_scene_object_nhgui_edit(
				&object_nhgui_edit,
				&gui_context,
				&gui_font,
				&gui_input,
				result, 	
				menu_width_mm,
				menu_font_mm,
				objects,
				object_count
		);


		light_scene_object_commit(
				&scene.state_instance,
			       	objects, 
				object_count
		);




		nhgui_window_end(
			&gui_window,
			&gui_context,
			&gui_window_attribute,
			&gui_input,
			result	
		);



		nhgui_glfw_frame_end(
				&gui_frame, 
				&gui_input
		);	


		light_platform_update(&platform);
		glError("main loop:");
    	}
		
	light_scene_deinitialize(&scene);




	printf( "exiting");
	light_platform_deinitialize(&platform);
		
	return (result);
}
