#include "math/vec.h"
#include "platform.h"

struct frame_info
{
	struct vec2 delta;
	struct vec2 mouse;

	uint32_t resize;
	uint32_t width;
	uint32_t height;
};

struct frame_info frame_info_update(struct frame_info *prev)
{
	struct frame_info result = {};
	
	struct vec2 mouse;
	platform_mouse(&mouse);

	uint32_t width, height;
	platform_resolution(&width, &height);

	result.mouse = mouse;
	
	if(prev != NULL)
		result.resize = (width != prev->width || height != prev->height);

	result.width = width;
	result.height = height;
	
	return result;
}

struct vec2 frame_info_mouse_delta(struct frame_info *curr, struct frame_info *prev)
{

	struct vec2 np = {prev->mouse.x/prev->width, -prev->mouse.y/prev->height};
	struct vec2 nc = {curr->mouse.x/curr->width, -curr->mouse.y/curr->height};
	struct vec2 delta = v2sub(np, nc); 
	
	return delta;

}


