#ifndef CAMERA_H
#define CAMERA_H

#include <string.h>
#include <stdint.h>

#include <GL/glew.h>

#include "math/vec.h"
#include "math/mat4x4.h"


struct camera_view_state
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

void camera_initialize(struct camera_view_state *view_state, GLuint buffer_base_index);
void camera_buffer_bind(struct camera_view_state *view_state, GLuint program);
void camera_view_projection(struct camera_view_state *view_state, int width, int height);
struct mat4x4 camera_view_matrix(struct camera_view_state *camera_view);

#endif
