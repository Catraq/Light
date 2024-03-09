#ifndef LIGHT_CAMERA_H
#define LIGHT_CAMERA_H

#include <string.h>
#include <stdint.h>

#include <GL/glew.h>

#include "math/vec.h"
#include "math/mat4x4.h"


struct light_camera_view_state
{
	float near;
	float far;
	float fov;

	struct mat4x4 projection;
	struct vec3 position;
	struct vec3 rotation;

	GLuint camera_buffer;
	GLuint buffer_base_index;
};

void light_camera_initialize(struct light_camera_view_state *view_state, GLuint buffer_base_index);
int light_camera_buffer_bind(struct light_camera_view_state *view_state, GLuint program);
void light_camera_view_matrix(struct light_camera_view_state *view_state, uint32_t width, uint32_t height);

#endif
