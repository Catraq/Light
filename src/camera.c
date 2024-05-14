#include "camera.h"
	
struct glsl_view_buffer{
		
	struct mat4x4 view;
	struct mat4x4 view_inv;

	uint32_t width;
	uint32_t height;
	uint32_t dummy[2];

	struct mat4x4 proj;	
};


void light_camera_initialize(struct light_camera_view_state *view_state)
{

	glGenBuffers(1, &view_state->camera_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, view_state->camera_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(struct glsl_view_buffer), 0, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


int light_camera_buffer_bind(struct light_camera_view_state *view_state)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, view_state->camera_buffer);
	return 0;
}


void light_camera_view_matrix(struct light_camera_view_state *view_state, uint32_t width, uint32_t height)
{

	struct mat4x4 proj = m4x4pers((float)width/(float)height, view_state->fov, view_state->near, view_state->far);

	/* Translate camera */
	struct mat4x4 rotation = m4x4rote(view_state->rotation);
	struct mat4x4 position = m4x4trs(view_state->position);
	struct mat4x4 view = m4x4mul(rotation, position);

	/* Upload to shader program buffer */
	{
		glBindBuffer(GL_UNIFORM_BUFFER, view_state->camera_buffer);
		struct glsl_view_buffer *view_buffer = (struct glsl_view_buffer *)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);

		int result = 0;
		view_buffer->proj = proj;
		view_buffer->view = view;
		view_buffer->view_inv = m4x4inv(&view, &result);
		view_buffer->width = width;
		view_buffer->height = height;

		glUnmapBuffer(GL_UNIFORM_BUFFER);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

