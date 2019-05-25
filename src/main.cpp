#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include "platform.h"
#include "vertex_buffer.h"
#include "physic.h"
#include "renderer.h"


#include "camera.h"
#include "camera_input.h"

#include "model/raw_model.h"

#include "math/mat4x4.h"

#define DEBUG 1
#define DEBUG_PHYSIC 0
/*
	TODO: 
		Use a single memory block.

		 Copy to buffer in render_instance  
		 Support other data types than physic_body? 
			
		Obj loader.	But can't handle large files. 
			
		Input:
			Create general solution for input handling and input configuration. 
			Etc hide player forward/backward keys/joysticks behind interface.  
			reading zoom values and forward, backward keys. 
		
		Collisions 				
		Screen to scene raycasting(picking)  					
		View frustum
		
		Cleanup
		Check/fix mem leaks 
				
				
		Resource table for loading(filename -> ptr ) 
	
*/


struct frame_info
{
	vec2  				mouse;
	uint32_t 			width;
	uint32_t 			height;
};


struct frame_info_update_result
{
	vec2 mouse_delta;
};

//TODO: This should go in some kind of input system
struct frame_info_update_result frame_info_update( frame_info *info )
{
	struct frame_info_update_result result;
	
	struct vec2 mouse;
	platform_mouse(&mouse);
	platform_resolution(&info->width, &info->height);
	
	const float half_width_f  = (float)info->width/2.0f;
	const float half_height_f = (float)info->height/2.0f;
	
	struct vec2 tmp;
	tmp.x = -(half_width_f/info->width -  mouse.x/info->width );
	tmp.y =  (half_height_f/info->height - mouse.y/info->height );
	
	result.mouse_delta = v2sub(info->mouse, tmp); 
	result.mouse_delta = v2scl(result.mouse_delta, 2.0f); 
	info->mouse = tmp;
	
	return (result);
}


void scene_view_initialize(camera_input_state *input_state, camera_view_state *view_state, uint32_t width, uint32_t height)
{
	//Camera attributes 
	const float fov = 3.14f/3.0f;
	const float far = 1000.0f;
	const float close = 1.0f;
	float 		ratio = ((float)width / (float)height); 
	
	//Setup camera. 
	camera_view_projection( view_state, m4x4pers( ratio, fov, close, far ) );
	view_state->position = (vec3){ 0.0f, 0.0f, 5.0f };
	view_state->rotation = (vec3){ 0.0f, 0.0f, 0.0f };

	input_state->speed = 20.0f;
	input_state->friction = 0.001f;
	
}


mat4x4 scene_update_view(camera_input_state *camera_input, camera_view_state *camera_view, frame_info_update_result *frame_info_result, const float& deltatime )
{
	//Buffer for all actions 	
	uint8_t action_count= 0;
	enum camera_input_action actions[camera_input_action::length];
	float multiplier[camera_input_action::length];
	struct vec2 delta_rotation = {};	
	
	//TODO: They keys used for movement should not come from paltform. 
	if(platform_key('W') == PLATFORM_PRESS){
		actions[action_count] 		= camera_input_action::forward;
		multiplier[action_count] 	= 1.0f;
		action_count++;
	}
	
	else if(platform_key('S') == PLATFORM_PRESS){
		actions[action_count] 		= camera_input_action::backward;
		multiplier[action_count] 	= 1.0f;
		action_count++;
	}
	
	else if(platform_key(32) == PLATFORM_PRESS){
		actions[action_count] 		= camera_input_action::stop;
		multiplier[action_count] 	= 1.0f;
		action_count++;
	}

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
		delta_rotation = frame_info_result->mouse_delta;
	}
	
	
	camera_input_update(camera_input, camera_view, actions, multiplier,	action_count, delta_rotation, deltatime);
	return camera_view_matrix(camera_view);
}


struct engine_core
{	
	/* Physic instance */
	physic physic;

	/* Render instance */
	renderer renderer;

	/* Vertexbuffer shared between renderer and physic */
	vertex_buffer vertex_buffer;
};

int engine_initialize(struct engine_core *core)
{
	int result = 0;

	physic_initialize(&core->physic);
	renderer_initialize(&core->renderer);
	vertex_buffer_initialize(&core->vertex_buffer);

	return(result);
}

#include "game.h"


int main()
{
	int result;
	struct engine_core core;

	result = platform_initialize();
	if(result < 0)
	{
		printf("Error: could not initialize platform.\n");
		/* Abort */
	}

	result = engine_initialize(&core);	
    if(result < 0)
	{
		printf("Error: could not initialize engine.\n");
		/* Abort */
	}
	

	struct game game;
	uint32_t required_memory = 0;
	required_memory += game_required_memory();
	uint8_t *game_memory = (uint8_t*)malloc(required_memory); 
	game_create_instance(&game, &core, game_memory);
	

	vertex_buffer_commit(&core.vertex_buffer);
	
	
	
	
	//TODO 
	
	while( glGetError() != GL_NONE)
	{
		printf( "GLerror\n");
	}
	
	frame_info r_frame_info;
	frame_info_update( &r_frame_info );
	camera_view_state player_camera_view;
	camera_input_state camera_input;

	scene_view_initialize(&camera_input, &player_camera_view, r_frame_info.width, r_frame_info.height);
	
	
	clock_t time = clock();
	clock_t physic_time = clock();
	
#ifdef DEBUG 
	clock_t  render_pass_timer = clock();
	double   render_pass_avg = 0;
	
	clock_t  physic_pass_timer = clock();
	double   physic_pass_avg = 0;
	uint32_t dframes;
	
#endif
	
	uint32_t frames = 0;
	
	double avg_frametime = 0.0f;
	physic_update( &core.physic, 0.0f );
	
    while (!glfwWindowShouldClose(window))
    {
		
			float physic_dt = (double)(clock() - physic_time) / (double)CLOCKS_PER_SEC;
			physic_time = clock();
				
			
			frame_info_update_result frame_result = frame_info_update(&r_frame_info);
			mat4x4 view = scene_update_view(&camera_input, &player_camera_view, &frame_result, physic_dt);
		
			#if DEBUG 
			
			#if DEBUG_PHYSIC
			physic_pass_timer = clock();
			physic_update( &core.physic, physic_dt );
			physic_pass_avg += (double)(clock() - physic_pass_timer) / (double)CLOCKS_PER_SEC;
			#endif 
			
			render_pass_timer = clock();
			renderer_instance_dwrite_instances(&game.floor_render_handler, physic_instance_bodies( &core.physic, &game.floorphy), game.floorphy.count );
			renderer_instance_dwrite_instances( &game.spaceship_render_handler, physic_instance_bodies( &core.physic, &game.spaceship_physic_handler ), game.spaceship_physic_handler.count );

			renderer_render(&core.renderer, &view, r_frame_info.width,  r_frame_info.height);
			render_pass_avg += (double)(clock() - render_pass_timer) / (double)CLOCKS_PER_SEC;
		
			
			if( clock() - time > CLOCKS_PER_SEC)
			{
				system("cls");
				printf("frames/s: %u \nFrametime: %f \nPhysictime: %f \nRendertime: %f \n", frames, avg_frametime/( double )frames, physic_pass_avg/dframes, render_pass_avg/dframes );
				
				avg_frametime 		= 0.0f;
				physic_pass_avg 	= 0.0f;
				render_pass_avg 	= 0.0f;
				
				time = clock();
				frames = 0;
				dframes = 0;
			}
			else{
				frames++;
				dframes++;
			}
			
		#else 
			physic_update( &core.physic, physic_dt );
			renderer_instance_dwrite_instances( &floor_render_handler, physic_instance_bodies( &core.physic, &floocore.physic_handler ), floocore.physic_handler.count );
			renderer_instance_dwrite_instances( &spaceship_render_handler, physic_instance_bodies( &core.physic, &spaceship_physic_handler ), spaceship_physic_handler.count );
			renderer_render( &r_renderercore.renderer, &view, r_frame_info.width,  r_frame_info.height );
		#endif 
			
			platform_update( );
    }
	
	vertex_buffer_deinitialize( &core.vertex_buffer );
	printf( "Exiting");
	
	platform_deinitialize();
		
	return (result);
}
