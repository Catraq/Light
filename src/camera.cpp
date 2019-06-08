#include "camera.h"


void camera_initialize(struct camera_view_state *view_state, GLuint buffer_base_index)
{

	glGenBuffers(1, &view_state->camera_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, view_state->camera_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(struct mat4x4), 0, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	view_state->buffer_base_index = buffer_base_index;	
	glBindBufferBase(GL_UNIFORM_BUFFER, buffer_base_index, view_state->camera_buffer);
}


void camera_buffer_bind(struct camera_view_state *view_state, GLuint program)
{

	GLuint block_index = glGetUniformBlockIndex(program, "scene");
	glUniformBlockBinding(program, block_index, view_state->buffer_base_index);
}

void camera_view_projection(struct camera_view_state *view_state, int width, int height)
{
	float ratio = ((float)width / (float)height); 
	view_state->projection = m4x4pers(ratio, view_state->fov, view_state->near, view_state->far);
}

struct mat4x4 camera_view_matrix(struct camera_view_state *camera_view)
{
	struct mat4x4 rotation = m4x4rote(camera_view->rotation);
	struct mat4x4 position = m4x4trs(camera_view->position);
	struct mat4x4 view = m4x4mul(rotation, position);

	view = m4x4mul(camera_view->projection, view);
	{
		glBindBuffer(GL_UNIFORM_BUFFER, camera_view->camera_buffer);
		struct mat4x4 *view_dest = (struct mat4x4 *)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		memcpy(view_dest, &view, sizeof(struct mat4x4));
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	return view;
}

