#ifndef SCENE_INSTANCE_H
#define SCENE_INSTANCE_H


#include "math/vec.h"
#include "math/mat4x4.h"

#include "camera.h"
#include "camera_input.h"

#include "platform.h"

#include "model/raw_model.h"
#include "vertex_buffer.h"
#include "physic.h"
#include "renderer.h"

struct frame_info
{
	struct vec2 mouse;
	uint32_t width;
	uint32_t height;
};

#endif 

