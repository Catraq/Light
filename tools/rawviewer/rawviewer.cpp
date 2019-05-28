#include <platform.h>

#include <camera.h>
#include <camera_input.h>

#include <physic.h>
#include <renderer.h>

#include <math/vec.h>
#include <math/mat4x4.h>

#include <vertex_buffer.h>



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

frame_info_update_result frame_info_update(frame_info *info)
{
	frame_info_update_result result;
	
	vec2 mouse;
	platform_mouse( &mouse );
	platform_resolution( &info->width, &info->height );
	
	const float half_width_f  = (float)info->width/2.0f;
	const float half_height_f = (float)info->height/2.0f;
	
	vec2 mouse_tmp;
	mouse_tmp.x = -( half_width_f/info->width -  mouse.x/info->width );
	mouse_tmp.y =  ( half_height_f/info->height - mouse.y/info->height );
	
	result.mouse_delta = v2sub(info->mouse, mouse_tmp); 
	info->mouse = mouse_tmp;
	
	
	return ( result );
}

vertex_buffer_handler load_model(  vertex_buffer *buffer, const char *filename )
{
	vertex_buffer_handler model_handler; 
	FILE *fp = fopen( filename, "rb");
	if( fp ){
		
		raw_model *raw_model_data = raw_model_load( fp );
		
		if( raw_model_data )
		{
			uint32_t vertex_count = 0;
			vertex *vertices = raw_model_vertices( raw_model_data, &vertex_count);
			
			uint32_t indice_count = 0;
			uint32_t *indices = raw_model_indices( raw_model_data, &indice_count );
			
			model_handler = vertex_buffer_push( buffer, vertices, vertex_count, indices, indice_count ); 					
			raw_model_release( raw_model_data );
		}
		fclose( fp );
	}
	
	return ( model_handler );
}


int main( int args, char *argv[])
{
	int result = -1;
	if( args != 2 )
	{
		printf("Usage: %s filename", argv[0] );
		return ( result );
	}
	
	const char *filename = argv[1];
	
	result = platform_initialize();
	
	//Init vertex buffer for render engine
	vertex_buffer buffer;
	vertex_buffer_initialize(&buffer);
	vertex_buffer_handler model = load_model( &buffer, filename);
	vertex_buffer_commit( &buffer );
	
	//Init render engine
	renderer r_renderer;
	renderer_initialize(&r_renderer);
	
	//Init render handle for the object
	render_instance model_render_handler;
	{
		renderer_instance_initialize(&r_renderer, &model_render_handler,&buffer, &model);
	}
	
	frame_info r_frame_info;
	frame_info_update( &r_frame_info );
	
	//Camera attributes 
	const float fov = 3.14f/3.0f;
	const float far = 100.0f;
	const float close = 1.0f;
	float 		ratio = ((float)r_frame_info.width / (float)r_frame_info.height); 
	
	
	mat4x4 view;
	{
		//Setup camera. 
		camera_view_state player_camera_view;
		camera_view_projection( &player_camera_view, m4x4pers( ratio, fov, close, far ));
		player_camera_view.position = vec3{ 0.0f, 0.0f, 20.0f };
		player_camera_view.rotation = vec3{ 0.0f, 0.0f, 0.0f };
		view = camera_view_matrix(&player_camera_view);
	}
	
	


	physic_body model_body[1];
	model_body[0].position = vec3{0.0f,0.0f,0.0f};
	model_body[0].rotation = vec3{0.0f,0.0f,0.0f};
	
	
	while( !platform_exit() )
	{
		frame_info_update_result frame_result = frame_info_update( &r_frame_info );
		renderer_render( &r_renderer, &view, r_frame_info.width,  r_frame_info.height );
		
		
		//TODO: This should be done in some other way...
		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ){
			model_body[0].rotation = v3add(model_body[0].rotation, update_camera_rotation(frame_result.mouse_delta));
		}
		
		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ){
			model_body[0].position.z += frame_result.mouse_delta.y*20.0f;
		}
		 
		
		physic_instance_translate_body( &model_body[0] );
		renderer_instance_dwrite_instances( &model_render_handler, model_body, 1 );

		
		platform_update();
	}
	
	vertex_buffer_deinitialize( &buffer );
	platform_deinitialize();
	
	return (result);
}
