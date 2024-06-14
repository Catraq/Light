#include "frame.h"


struct light_frame_info light_frame_info_update(
		struct light_frame_info *prev,
		struct light_platform *platform
		)
{
	struct light_frame_info result = {};
	
	struct vec2 mouse;
	light_platform_mouse(platform, &mouse);

	uint32_t width, height;
	light_platform_window_resolution(platform, &width, &height);

	result.mouse = mouse;
	
	if(prev != NULL)
		result.resize = (width != prev->width || height != prev->height);

	result.width = width;
	result.height = height;
	
	return result;
}

struct vec2 light_frame_info_mouse_delta(struct light_frame_info *curr, struct light_frame_info *prev)
{

	struct vec2 np = {prev->mouse.x/prev->width, -prev->mouse.y/prev->height};
	struct vec2 nc = {curr->mouse.x/curr->width, -curr->mouse.y/curr->height};
	struct vec2 delta = v2sub(np, nc); 
	
	return delta;

}


