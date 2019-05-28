#ifndef CAMERA_H
#define CAMERA_H

#include <string.h>
#include <stdint.h>


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
};

void camera_view_projection(struct camera_view_state *view_state, int width, int height);
void camera_view_update(struct camera_view_state *camera_view, struct vec3 position_delta, struct vec3 rotation_delta);
struct mat4x4 camera_view_matrix(struct camera_view_state *camera_view);

#endif
