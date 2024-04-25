#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include <GL/glew.h>
#include <stdint.h>

#include "model/raw_model.h"

struct light_vertex_buffer
{
	GLuint vertexbuffer;
	GLuint indicebuffer;
	
	uint32_t vertex_offset;
	uint32_t indice_offset;
};

struct light_vertex_buffer_handler
{
	uint32_t vertex_offset;
	uint32_t vertex_count;

	uint32_t indice_offset;
	uint32_t indice_count;
};

struct light_vertex_handle
{
	struct vertex_buffer *buffer;
	struct vertex_buffer_handler *handle;

};


void light_vertex_buffer_initialize(struct light_vertex_buffer *buffer, uint32_t vertex_buffer_size, uint32_t indice_buffer_size);
void light_vertex_buffer_deinitialize(struct light_vertex_buffer *buffer);
void light_vertex_buffer_attribute_pointer(struct light_vertex_buffer *buffer, struct light_vertex_buffer_handler *handler);
struct light_vertex_buffer_handler light_vertex_buffer_push(struct light_vertex_buffer *buffer, struct vertex *vertices, uint32_t vertex_count, uint32_t *indices, uint32_t indice_count);

#endif
