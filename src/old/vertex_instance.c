#include "vertex_instance.h"

void light_vertex_instance_init(struct light_vertex_instance *instance, struct light_vertex_buffer *buffer, struct light_vertex_buffer_handler *handler)
{
	glGenBuffers(1, &instance->instance_buffer);
	glGenVertexArrays(1, &instance->vertex_array);

	glBindVertexArray(instance->vertex_array);
	light_vertex_buffer_attribute_pointer(buffer, handler);
	glBindVertexArray(0);


	instance->indice_count = handler->indice_count;
	instance->indice_offset = handler->indice_offset;

}

void light_vertex_instance_deinit(struct light_vertex_instance *instance)
{
	glDeleteBuffers(1, &instance->instance_buffer);
	glDeleteVertexArrays(1, &instance->vertex_array);
}

void light_vertex_instance_draw(struct light_vertex_instance *instance, GLuint instance_count)
{
	GLuint indice_count 	= instance->indice_count;
	GLuint indice_offset 	= instance->indice_offset;
	GLuint vertexarray 	= instance->vertex_array;

	glBindVertexArray(vertexarray);
	glDrawElementsInstanced(GL_TRIANGLES, 
		  indice_count,
		  GL_UNSIGNED_INT, 
		  (const void*)(indice_offset), 
		  instance_count
	);
}

void light_vertex_instance_commit(struct light_vertex_instance *instance , void *src, GLuint src_len)
{
	glBindBuffer(GL_ARRAY_BUFFER, instance->instance_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, src_len, src);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void light_vertex_instance_allocate(struct light_vertex_instance *instance, void *src, GLuint src_len)
{
	glBindBuffer(GL_ARRAY_BUFFER, instance->instance_buffer);
	glBufferData(GL_ARRAY_BUFFER, src_len, src, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}



