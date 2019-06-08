#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include <GL/glew.h>
#include <stdint.h>

#include "model/raw_model.h"

struct vertex_buffer
{
	GLuint vertexbuffer;
	GLuint indicebuffer;
	
	uint32_t vertex_offset;
	uint32_t indice_offset;
};

struct vertex_buffer_handler
{
	uint32_t vertex_offset;
	uint32_t vertex_count;

	uint32_t indice_offset;
	uint32_t indice_count;
};

struct vertex_handle
{
	struct vertex_buffer *buffer;
	struct vertex_buffer_handler *handle;

};


void vertex_buffer_initialize(struct vertex_buffer *buffer, uint32_t vertex_buffer_size, uint32_t indice_buffer_size);
void vertex_buffer_deinitialize(struct vertex_buffer *buffer);
void vertex_buffer_attribute_pointer(struct vertex_buffer *buffer, struct vertex_buffer_handler *handler);
struct vertex_buffer_handler vertex_buffer_push(struct vertex_buffer *buffer, struct vertex *vertices, uint32_t vertex_count, uint32_t *indices, uint32_t indice_count);
void vertex_buffer_handler_printf(struct vertex_buffer *buffer, struct vertex_buffer_handler *handler);

#endif
