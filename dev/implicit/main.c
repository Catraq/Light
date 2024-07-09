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
#include "scene_gui.h"

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

float phy_sphere(struct vec3 p)
{
	return v3len(p) - 1;
}

float phy_cylinder(struct vec3 p)
{
	float r = 1.0;
	float h = 1.0;
	float len_xz = sqrtf(p.x*p.x + p.z*p.z);
	float d_x = len_xz - r;
	float d_y = abs(p.y) - h;

	float m_1 = d_x < d_y ? d_y : d_x;
	float m_2 = m_1 < 0 ? m_1 : 0;
	
	d_x = d_x < 0 ? 0 : d_x;
	d_y = d_y < 0 ? 0 : d_y;
	float d_len = sqrtf(d_x*d_x + d_y*d_y);

	return m_2 + d_len;
}

float phy_F_sphere_sphere(struct vec3 p, struct mat4x4 o1, struct mat4x4 o2)
{
	struct vec4 p4 = {.x=p.x, .y=p.y, .z=p.z, .w = 1};
	struct vec4 p1 = m4x4mulv4(o1, p4);
	struct vec4 p2 = m4x4mulv4(o2, p4);
	struct vec3 c1 = {.x=p1.x, .y=p1.y, .z=p1.z};
	struct vec3 c2 = {.x=p2.x, .y=p2.y, .z=p2.z};

	return phy_sphere(c1) + phy_sphere(c2);	
}

struct vec3 phy_grad_sphere_sphere(struct vec3 p, struct mat4x4 o1 , struct mat4x4 o2)
{
	float stepsize = 0.00001;
	struct vec3 grad = {
		1.0/(2*stepsize)*(phy_F_sphere_sphere((struct vec3){.x=p.x+stepsize, .y=p.y, .z=p.z}, o1, o2) - phy_F_sphere_sphere((struct vec3){.x=p.x-stepsize, .y=p.y, .z=p.z}, o1, o2)),
		1.0/(2*stepsize)*(phy_F_sphere_sphere((struct vec3){.x=p.x, .y=p.y+stepsize, .z=p.z}, o1, o2) - phy_F_sphere_sphere((struct vec3){.x=p.x, .y=p.y-stepsize, .z=p.z}, o1, o2)),
		1.0/(2*stepsize)*(phy_F_sphere_sphere((struct vec3){.x=p.x, .y=p.y, .z=p.z+stepsize}, o1, o2) - phy_F_sphere_sphere((struct vec3){.x=p.x, .y=p.y, .z=p.z-stepsize}, o1, o2)),

	};
	return grad;
}

float phy_F_cylinder_cylinder(struct vec3 p, struct mat4x4 o1, struct mat4x4 o2)
{
	struct vec4 p4 = {.x=p.x, .y=p.y, .z=p.z, .w = 1};
	struct vec4 p1 = m4x4mulv4(o1, p4);
	struct vec4 p2 = m4x4mulv4(o2, p4);
	struct vec3 c1 = {.x=p1.x, .y=p1.y, .z=p1.z};
	struct vec3 c2 = {.x=p2.x, .y=p2.y, .z=p2.z};

	return phy_cylinder(c1) + phy_cylinder(c2);	
}

struct vec3 phy_grad_cylinder_cylinder(struct vec3 p, struct mat4x4 o1 , struct mat4x4 o2)
{
	float stepsize = 0.00001;
	struct vec3 grad = {
		1.0/(2*stepsize)*(phy_F_cylinder_cylinder((struct vec3){.x=p.x+stepsize, .y=p.y, .z=p.z}, o1, o2) - phy_F_cylinder_cylinder((struct vec3){.x=p.x-stepsize, .y=p.y, .z=p.z}, o1, o2)),
		1.0/(2*stepsize)*(phy_F_cylinder_cylinder((struct vec3){.x=p.x, .y=p.y+stepsize, .z=p.z}, o1, o2) - phy_F_cylinder_cylinder((struct vec3){.x=p.x, .y=p.y-stepsize, .z=p.z}, o1, o2)),
		1.0/(2*stepsize)*(phy_F_cylinder_cylinder((struct vec3){.x=p.x, .y=p.y, .z=p.z+stepsize}, o1, o2) - phy_F_cylinder_cylinder((struct vec3){.x=p.x, .y=p.y, .z=p.z-stepsize}, o1, o2)),

	};


	float i = phy_F_cylinder_cylinder((struct vec3){.x=p.x, .y=p.y, .z=p.z+stepsize}, o1, o2);
       	float j = phy_F_cylinder_cylinder((struct vec3){.x=p.x, .y=p.y, .z=p.z-stepsize}, o1, o2);
	printf("-> %f %f \n", i, j);
	return grad;
}


void
light_scene_object_physic_simulate(
		struct light_scene_object *objects,
	       	uint32_t object_count, 
		const float deltatime
)
{
	struct mat4x4 translations[object_count];
	for(uint32_t i = 0; i < object_count; i++)
	{
		struct mat4x4 TRS = m4x4mul(
			m4x4rote(objects[i].rotation),
			m4x4scl(objects[i].scale)
		);

		TRS = m4x4mul(
			m4x4trs(objects[i].position),
			TRS
		);
		
		int dummy = 0;

		translations[i] = m4x4inv(&TRS, &dummy);
	}

	for(uint32_t i = 0; i < object_count; i++)
	{
		for(uint32_t j = i+1; j < object_count; j++)
			
		{
			uint32_t collision = 0;
			if(objects[i].object_index == 0 && objects[j].object_index == 0)
			{
				float alpha = 1.00;
				struct vec3 minimum = {.x=0, .y=0, .z=0};
				for(uint32_t k = 0; k < 32; k++)
				{
					struct vec3 grad = phy_grad_cylinder_cylinder(minimum, translations[i], translations[j]);
					minimum = v3sub(minimum, v3scl(grad, alpha));	
					float d = phy_F_cylinder_cylinder(minimum, translations[i], translations[j]);
					if(d < 0.0){
						collision = 1;
						break;	
					}
					printf("%f %f %f \n", minimum.x, minimum.y, minimum.z);
				}
			}
			else if(objects[i].object_index == 2 && objects[j].object_index == 2)
			{
				float alpha = 1.00;
				struct vec3 minimum = {.x=0, .y=0, .z=0};
				for(uint32_t k = 0; k < 32; k++)
				{
					struct vec3 grad = phy_grad_sphere_sphere(minimum, translations[i], translations[j]);
					minimum = v3sub(minimum, v3scl(grad, alpha));	
					float d = phy_F_cylinder_cylinder(minimum, translations[i], translations[j]);
					if(d < 0.0){
						collision = 1;
						break;	
					}
				}
				printf("%u %u %f %f %f \n", i,j, minimum.x, minimum.y, minimum.z);
			}
			if(collision > 0)
			{

				struct vec3 n = v3norm(v3sub(objects[i].position, objects[j].position));
				struct vec3 v_rel = v3sub(objects[i].velocity, objects[j].velocity);
				
				float e = 1.0f;	
				float v_j = -(1.0f+e) * v3dot(v_rel, n);
				float J = v_j / (1.0/objects[i].mass + 1.0/objects[j].mass);
				
				
				objects[i].velocity = v3add(objects[i].velocity, v3scl(n, 1.0/objects[i].mass * J));
				objects[j].velocity = v3sub(objects[j].velocity, v3scl(n, 1.0/objects[j].mass * J));

			}
		}	
	}

	for(uint32_t i = 0; i < object_count; i++){
		objects[i].position = v3add(objects[i].position, v3scl(objects[i].velocity, deltatime));
	
	}
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
		light_instance[i].position = (struct vec3){.x=0.0f, 10.0f, -20.0};
	}

	light_instance[0].color = (struct vec3){.x=1.0, .y=0.1f, .z=1.0f};

	light_scene_implicit_commit_light(&scene.state_instance, light_instance, light_count);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	const uint32_t object_size = 10;
	const uint32_t object_count = 4;

	struct light_scene_object objects[object_size];
	memset(objects, 0, sizeof(objects));


#if 0	
	uint32_t implicit_function_name_count = light_scene_object_implicit_name_count(&scene);	
	for(uint32_t i = 0; i < implicit_function_name_count; i++)
	{	
		const char *implicit_function_name = light_scene_object_implicit_name(&scene, i);
		printf("Object(%u) with implicit function %s \n", i, implicit_function_name);
		
		objects[i].position = (struct vec3){.x=i*5.00, .y=0, .z=10};
		objects[i].scale= (struct vec3){.x=1, .y=1, .z=1};
		objects[i].rotation= (struct vec3){.z=0.0, .x=3.14/2, .y=0};
		objects[i].object_index = 2;

		objects[i].mass = 10.0f;
		objects[i].velocity = (struct vec3){.x=(1.0-1.0f*i)*0.2f, .y=0, .z=0};

	}
#endif 
	objects[0].position = (struct vec3){.x=0, .y=-4, .z=10};
	objects[0].scale= (struct vec3){.x=10, .y=1, .z=10};
	objects[0].object_index = 1;
	objects[0].mass = 1000000.0f;
	
	objects[1].position = (struct vec3){.x=0, .y=0, .z=10};
	objects[1].scale= (struct vec3){.x=1, .y=1, .z=1};
	objects[1].object_index = 1;
	objects[1].mass = 1.0f;
	
	objects[2].position = (struct vec3){.x=0, .y=4, .z=10};
	objects[2].scale= (struct vec3){.x=1, .y=1, .z=1};
	objects[2].object_index = 0;
	objects[2].mass = 1.0f;
	
	objects[3].position = (struct vec3){.x=0, .y=8, .z=10};
	objects[3].scale= (struct vec3){.x=1, .y=1, .z=1};
	objects[3].object_index = 2;
	objects[3].mass = 1.0f;



	light_scene_object_commit(&scene.state_instance, objects, object_count);

	light_scene_state_bind(&scene.state_instance);

	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	float 		fps_time;
	struct timeval 	fps_time_last;
	struct timeval 	fps_time_curr;
	float 		deltatime;

	gettimeofday(&fps_time_last, NULL);
	fps_time_curr = fps_time_last;


	while(!light_platform_exit(&platform))
    	{
		struct timeval fps_time_curr_tmp;
		gettimeofday(&fps_time_curr_tmp, NULL);	
		float last_deltatime = deltatime;
		deltatime = fps_time_curr_tmp.tv_sec - fps_time_curr.tv_sec + (float)(fps_time_curr_tmp.tv_usec - fps_time_curr.tv_usec)/(1000.0f*1000.0f);
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

	
		struct vec3 gravity = (struct vec3){.x=0, .y=-9.82, .z=0};
		for(uint32_t i = 1; i < object_count; i++){
			objects[i].velocity = v3add(objects[i].velocity, v3scl(gravity, deltatime));
		}

		light_scene_update(
				&scene, 
				&platform, 
				width, height, deltatime,
				objects, 
				object_count
				);


	

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
				&scene, 
				&gui_context,
				&gui_font,
				&gui_input,
				result, 	
				menu_width_mm,
				menu_font_mm,
				objects,
				object_count
		);

#if 0
		light_scene_object_physic_simulate(
				objects, 
				object_count,
				deltatime
		);
#endif 

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
