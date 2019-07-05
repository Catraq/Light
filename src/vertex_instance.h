#ifndef VERTEX_INSTANCE_H
#define VERTEX_INSTANCE_H

#include "vertex_buffer.h"


struct vertex_instance
{
	GLuint vertex_array;

	GLuint instance_buffer;

	GLuint indice_count;
	GLuint indice_offset;
};


void vertex_instance_initialize(struct vertex_instance *instance, struct vertex_buffer *buffer, struct vertex_buffer_handler *handler);
void vertex_instance_draw(struct vertex_instance *instance, GLuint instance_count);
void vertex_instance_update(GLuint buffer, void *src, GLuint src_len);


#endif //VERTEX_INSTANCE_H


