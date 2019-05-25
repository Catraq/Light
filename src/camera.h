#ifndef CAMERA_H
#define CAMERA_H

#include <string.h>

#include "math/vec.h"
#include "math/mat4x4.h"

/*
		Is the camera really in need of quaternion?

*/

	
struct camera_view_state
{
	mat4x4 projection;
	vec3   position;
	vec3   rotation;
};

void camera_view_projection(camera_view_state *camera_view, mat4x4 projection )
{
	memcpy(&camera_view->projection, &projection, sizeof( mat4x4 ));
}

void camera_view_update(camera_view_state *camera_view, vec3 position_delta, vec3 rotation_delta)
{
	camera_view->rotation = v3add(camera_view->rotation, rotation_delta);
	camera_view->position = v3add(camera_view->position, position_delta);
}


mat4x4 camera_view_matrix(camera_view_state *camera_view)
{
	mat4x4 rotation = m4x4rote(camera_view->rotation);
	mat4x4 position = m4x4trs(camera_view->position);
	mat4x4 view = m4x4mul(rotation, position);

	return m4x4mul(camera_view->projection, view);
}
#endif //CAMERA_H
