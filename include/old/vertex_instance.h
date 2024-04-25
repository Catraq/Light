#ifndef VERTEX_INSTANCE_H
#define VERTEX_INSTANCE_H

#include "vertex_buffer.h"


struct light_vertex_instance
{
	GLuint vertex_array;

	GLuint instance_buffer;

	GLuint indice_count;
	GLuint indice_offset;
};


void light_vertex_instance_init(struct light_vertex_instance *instance, struct light_vertex_buffer *buffer, struct light_vertex_buffer_handler *handler);

void light_vertex_instance_deinit(struct light_vertex_instance *instance);
void light_vertex_instance_draw(struct light_vertex_instance *instance, GLuint instance_count);
void light_vertex_instance_allocate(struct light_vertex_instance *instance , void *src, GLuint src_len);
void light_vertex_instance_commit(struct light_vertex_instance *instance , void *src, GLuint src_len);


#endif //VERTEX_INSTANCE_H


