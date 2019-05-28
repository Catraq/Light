#include "camera.h"


void camera_view_projection(struct camera_view_state *view_state, int width, int height)
{
	float ratio = ((float)width / (float)height); 
	struct mat4x4 projection = m4x4pers(ratio, view_state->fov, view_state->near, view_state->far);
	memcpy(&view_state->projection, &projection, sizeof(struct mat4x4));
}

void camera_view_update(struct camera_view_state *camera_view, struct vec3 position_delta, struct vec3 rotation_delta)
{
	camera_view->rotation = v3add(camera_view->rotation, rotation_delta);
	camera_view->position = v3add(camera_view->position, position_delta);
}


struct mat4x4 camera_view_matrix(struct camera_view_state *camera_view)
{
	struct mat4x4 rotation = m4x4rote(camera_view->rotation);
	struct mat4x4 position = m4x4trs(camera_view->position);
	struct mat4x4 view = m4x4mul(rotation, position);

	return m4x4mul(camera_view->projection, view);
}

