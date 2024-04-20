#include "camera.h"
	
struct glsl_view_buffer{
	struct mat4x4 view;
	struct mat4x4 view_inv;
};


void light_camera_initialize(struct light_camera_view_state *view_state, GLuint buffer_base_index)
{

	glGenBuffers(1, &view_state->camera_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, view_state->camera_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(struct glsl_view_buffer), 0, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	view_state->buffer_base_index = buffer_base_index;	
	glBindBufferBase(GL_UNIFORM_BUFFER, buffer_base_index, view_state->camera_buffer);
}


int light_camera_buffer_bind(struct light_camera_view_state *view_state, GLuint program)
{
	GLuint block_index = glGetUniformBlockIndex(program, "scene");
	if(block_index == GL_INVALID_INDEX){
		return -1;
	}

	glUniformBlockBinding(program, block_index, view_state->buffer_base_index);

	return 0;
}


void light_camera_view_matrix(struct light_camera_view_state *view_state, uint32_t width, uint32_t height)
{

	/* Translate camera */
	struct mat4x4 rotation = m4x4rote(view_state->rotation);
	struct mat4x4 position = m4x4trs(view_state->position);
	struct mat4x4 view = m4x4mul(rotation, position);

	/* Upload to shader program buffer */
	{
		glBindBuffer(GL_UNIFORM_BUFFER, view_state->camera_buffer);
		struct glsl_view_buffer *view_buffer = (struct glsl_view_buffer *)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		memcpy(&view_buffer->view, &view, sizeof(struct mat4x4));

		int result = 0;
		view_buffer->view_inv = m4x4inv(&view, &result);

		glUnmapBuffer(GL_UNIFORM_BUFFER);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

