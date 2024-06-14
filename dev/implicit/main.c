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

struct light_scene_object
{
	struct vec3 scale;
	struct vec3 rotation;
	struct vec3 position;
};

GLuint light_shader_compute_create(const GLchar **source, GLint *length, uint32_t count)
{
	GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
	if(compute_shader == 0)
	{
		fprintf(stderr, "glCreateShader(GL_COMPUTE_SHADER) failed.");
		return -1;
	}

	glShaderSource(compute_shader, count, source , length);
	glCompileShader(compute_shader);
	
	GLint compiled = GL_FALSE;
	glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE){

		GLchar log[8192];
		GLsizei length;
		glGetShaderInfoLog(compute_shader, 8192, &length, log);
		if( length != 0 )
		{
			fprintf(stderr, " ---- Compilation of shader failed.  ---- \n %s \n", log);
		}

		glDeleteShader(compute_shader);

		return 0;
	}
	
	GLuint compute_program = glCreateProgram();
	glAttachShader(compute_program, compute_shader);
	glLinkProgram(compute_program);
	
	glDeleteShader(compute_shader);
	
	GLint linked = GL_FALSE;
	glGetProgramiv(compute_program, GL_LINK_STATUS, &linked);
	if(linked == GL_FALSE) {
		GLsizei length;
		GLchar log[8192];
		glGetProgramInfoLog(compute_program, 8192, &length, log);
		fprintf(stderr, " ---- Linking of program failed ---- \n %s \n", log);

		glDeleteProgram(compute_program);
		return 0;
	}


	return compute_program;
}



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
	struct nhgui_object_text_list gui_object_list;
	struct nhgui_glfw_frame gui_frame = nhgui_frame_create(platform.window);
	
	gui_object_list = (struct nhgui_object_text_list){
		.char_scroll_per_sec = 1,
		.text_color = {.x = 1.0, .y = 1.0, .z = 1.0},	
		.field_color = {.x = 0.0, .y = 0.0, .z = 0.0},	
		.selected_text_color = {.x = 0.0, .y = 0.0, .z = 0.0},	
		.selected_field_color = {.x = 1.0, .y = 1.0, .z = 1.0},	
	
	};

	
	/*create a quad as render surface */	
	struct light_surface_textured quad_surface;
	result = light_surface_textured_initialize(&quad_surface);
	if(result <0)
	{
		printf("light_surface_textured_initialize(): failed. \n");
		exit(EXIT_FAILURE);	
	}
	CHECK_GL_ERROR();
	
	struct light_scene_instance scene;
	result = light_scene_initialize(&scene, &platform);
	if(result < 0)
	{
		printf("light_scene_initialize(): failed. \n");
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

	struct light_scene_state_instance state_instance;
	result = light_scene_state_initialize(&state_instance, state_build);
	if(result < 0){
		printf("light_scene_state_initialize(): failed. \n");
		exit(EXIT_FAILURE);
	}

	
	const uint32_t light_count = 1;
	struct light_scene_light_light_instance light_instance[light_count];
	for(uint32_t i = 0; i < light_count; i++)
	{
		light_instance[i].position = (struct vec3){.x=0.0f, 10.0f, 10.0};
	}

	light_instance[0].color = (struct vec3){.x=1.0, .y=0.1f, .z=1.0f};

	light_scene_implicit_commit_light(&state_instance, light_instance, light_count);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	

	
	uint32_t object_node_size = 3;
	struct light_scene_implicit_object object_node[object_node_size];

	{
		struct vec3 p0 = {.x = 0.0, .y = 0.0, .z = 10.0};
		object_node[0].translation = m4x4trs(p0);
		object_node[0].translation_inv = m4x4inv(&object_node[0].translation, &result); 
		object_node[0].object_index = 0;

		struct vec3 p1 = {.x = 3.0, .y = 0.0, .z = 10.0};
		object_node[1].translation = m4x4trs(p1);
		object_node[1].translation_inv = m4x4inv(&object_node[1].translation, &result); 
		object_node[1].object_index = 1;

		struct vec3 p2 = {.x = 6.0, .y = 0.0, .z = 10.0};
		object_node[2].translation = m4x4trs(p2);
		object_node[2].translation_inv = m4x4inv(&object_node[2].translation, &result); 
		object_node[2].object_index = 2;

		light_scene_implicit_commit_objects(
				&state_instance,
			       	object_node, 
				object_node_size
		);

	}



	uint32_t object_count = object_node_size;
	const uint32_t object_size = 10;


	const uint32_t object_name_size = 32;
	char object_name[object_size][object_name_size];
	uint32_t object_name_length[object_size];
	char *object_name_ptr[object_size];

	for(uint32_t i = 0; i < object_count; i++){
		result = snprintf(object_name[i], object_name_size, "Object(%u)", i);
		if(result < 0){
			fprintf(stderr, "snprintf failed. Check buffer size. \n");
			return -1;	
		}
		object_name_length[i] = result;
		object_name_ptr[i] = &object_name[i];	
	}


	light_scene_state_bind(&state_instance);

	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	float 		fps_time;
	struct timeval 	fps_time_last;
	struct timeval 	fps_time_curr;

	gettimeofday(&fps_time_last, NULL);
	fps_time_curr = fps_time_last;

		
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
		light_scene_state_dispatch(&state_instance, &scene.framebuffer, width, height, deltatime);



		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		struct nhgui_input gui_input = nhgui_glfw_frame_begin(
				&gui_frame,
			       	platform.window
		);
	
		float menu_width_mm = 50;
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

		result = nhgui_object_text_list(
			&gui_object_list,
			&gui_context, 
			object_name_ptr,
			object_name_length,
			object_count, 
			&gui_font, 
			&gui_list_attribute,
			&gui_input, 
			result
		);
		
		result = nhgui_result_dec_y(result);

		if(gui_object_list.selected > 0)
		{
			
		
		}



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
