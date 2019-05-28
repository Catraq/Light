#include "vertex_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vertex_buffer_initialize(struct vertex_buffer *buffer)
{
	buffer->vertices = 0;
	buffer->indices = 0;
	buffer->vertex_count = 0;
	buffer->indice_count = 0;
	
	
	glGenBuffers(1, &buffer->vertexbuffer);
	glGenBuffers(1, &buffer->indicebuffer);
	
}

void vertex_buffer_deinitialize(struct vertex_buffer *buffer)
{
	glDeleteBuffers(1, &buffer->vertexbuffer);
	glDeleteBuffers(1, &buffer->indicebuffer);
	
	free(buffer->vertices);
	free(buffer->indices);
	
	buffer->vertex_count = 0;
	buffer->indice_count = 0;
}



void vertex_buffer_commit(struct vertex_buffer *buffer)
{
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, buffer->vertex_count * sizeof(struct vertex), buffer->vertices, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->indicebuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer->indice_count * sizeof(uint32_t), buffer->indices, GL_STATIC_DRAW);
	
}

/*
	Maybe rename?
*/

struct vertex_buffer_handler vertex_buffer_push(struct vertex_buffer *buffer, struct vertex *vertices, uint32_t vertex_count, uint32_t *indices, uint32_t indice_count)
{
	//Set up vertex buffer handler.  
	vertex_buffer_handler handler; 
	handler.indice_offset = buffer->indice_count;
	handler.indice_count = indice_count;

	handler.vertex_offset = buffer->vertex_count;
	handler.vertex_count = vertex_count;
	

	
	//Calculate new size
	uint32_t new_vertex_count = buffer->vertex_count + vertex_count;
	uint32_t new_indice_count = buffer->indice_count + indice_count;

	//Allocate new memory
	vertex   *new_vertices = (struct vertex*)malloc(sizeof(vertex) * new_vertex_count);
	uint32_t *new_indices =  (uint32_t*)malloc(sizeof(uint32_t) * new_indice_count);
	
	
	const uint32_t vertice_copy_size = (sizeof(struct vertex) * buffer->vertex_count);
	const uint32_t indice_copy_size  = (sizeof(uint32_t) * buffer->indice_count);
	
	//Copy old to begining
	memcpy(new_vertices, buffer->vertices, vertice_copy_size);
	memcpy(new_indices,  buffer->indices,  indice_copy_size);
	
	
	struct vertex* new_vertices_append_dest = new_vertices + handler.vertex_offset;
	uint32_t* new_indices_append_dest = new_indices + handler.indice_offset;
	
	
	//Append data to the buffer. 
	memcpy(new_vertices_append_dest, vertices, sizeof(struct vertex) * vertex_count);
	memcpy(new_indices_append_dest,  indices,  sizeof(uint32_t) * indice_count);
	
	
	//Free old
	free(buffer->vertices);
	free(buffer->indices);

	//set new values
	buffer->vertices     = new_vertices;
	buffer->indices      = new_indices;
	buffer->vertex_count = new_vertex_count;
	buffer->indice_count = new_indice_count;
	
	
	return handler;
}

void vertex_buffer_handler_printf(struct vertex_buffer *buffer, struct vertex_buffer_handler *handler)
{
	for( uint32_t i = handler->vertex_offset; i < handler->vertex_offset + handler->vertex_count; i++)
	{
		printf( " X: %f Y: %f z: %f \n", 
			buffer->vertices[ i ].position.x,
			buffer->vertices[ i ].position.y,
			buffer->vertices[ i ].position.z
		);
	}
	
}

