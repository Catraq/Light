#include "vertex_instance.h"

void vertex_instance_initialize(struct vertex_instance *instance, struct vertex_buffer *buffer, struct vertex_buffer_handler *handler)
{
	glGenBuffers(1, &instance->instance_buffer);
	glGenVertexArrays(1, &instance->vertex_array);

	glBindVertexArray(instance->vertex_array);
	vertex_buffer_attribute_pointer(buffer, handler);
	glBindVertexArray(0);


	instance->indice_count = handler->indice_count;
	instance->indice_offset = handler->indice_offset;

}
void vertex_instance_draw(struct vertex_instance *instance, GLuint instance_count)
{
	GLuint indice_count 	= instance->indice_count;
	GLuint indice_offset 	= instance->indice_offset;
	GLuint vertexarray 	= instance->vertex_array;
	GLuint instance_buffer 	= instance->instance_buffer;

	glBindVertexArray(vertexarray);
	glDrawElementsInstanced(GL_TRIANGLES, 
		  indice_count,
		  GL_UNSIGNED_INT, 
		  (const void*)(indice_offset), 
		  instance_count
	);
}

void vertex_instance_update(GLuint buffer, void *src, GLuint src_len)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, src_len, src, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}



