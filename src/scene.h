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

struct scene_instance 
{

	struct frame_info frame_info;

	struct camera_view_state view_state;
	struct camera_input_state input_state;

	/* Physic instance */
	struct physic physic;

	/* Render instance */
	struct renderer renderer;

	/* Vertexbuffer shared between renderer and physic */
	struct vertex_buffer vertex_buffer;
};

int scene_initialize(struct scene_instance *instance);
int scene_render(struct scene_instance *instance, const float deltatime);

#endif 

