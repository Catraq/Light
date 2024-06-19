#ifndef LIGHT_CAMERA_H
#define LIGHT_CAMERA_H

#include <string.h>
#include <stdint.h>

#include <GL/glew.h>

#include "math/vec.h"
#include "math/mat4x4.h"


struct light_camera_view_state
{
	float fov;
	float near;
	float far;

	struct vec3 position;
	struct vec3 rotation;

	GLuint camera_buffer;
};

int light_camera_initialize(
		struct light_camera_view_state *view_state
);

void light_camera_deinitialize(
		struct light_camera_view_state *view_state
);

int light_camera_buffer_bind(struct light_camera_view_state *view_state);
void light_camera_view_matrix(struct light_camera_view_state *view_state, uint32_t width, uint32_t height);

#endif
