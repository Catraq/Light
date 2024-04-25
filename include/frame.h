#ifndef LIGHT_FRAME_H
#define LIGHT_FRAME_H

#include "math/vec.h"
#include "platform.h"

struct light_frame_info
{
	struct vec2 delta;
	struct vec2 mouse;

	uint32_t resize;
	uint32_t width;
	uint32_t height;
};

struct light_frame_info light_frame_info_update(struct light_frame_info *prev);

struct vec2 light_frame_info_mouse_delta(struct light_frame_info *curr, struct light_frame_info *prev);

#endif 

