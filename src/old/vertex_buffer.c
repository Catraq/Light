#include "vertex_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void light_vertex_buffer_initialize(struct light_vertex_buffer *buffer, uint32_t vertex_buffer_size, uint32_t indice_buffer_size)
{
	buffer->vertex_offset = 0;
	buffer->indice_offset = 0;
	
	glGenBuffers(1, &buffer->vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &buffer->indicebuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->indicebuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indice_buffer_size, NULL, GL_STATIC_DRAW);

}

void light_vertex_buffer_deinitialize(struct light_vertex_buffer *buffer)
{
	glDeleteBuffers(1, &buffer->vertexbuffer);
	glDeleteBuffers(1, &buffer->indicebuffer);
}





struct light_vertex_buffer_handler light_vertex_buffer_push(struct light_vertex_buffer *buffer, struct vertex *vertices, uint32_t vertex_count, uint32_t *indices, uint32_t indice_count)
{
	//Set up vertex buffer handler.  
	struct light_vertex_buffer_handler handler; 
	handler.indice_offset = buffer->indice_offset;
	handler.indice_count = indice_count;

	handler.vertex_offset = buffer->vertex_offset;
	handler.vertex_count = vertex_count;
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vertexbuffer);
	glBufferSubData(GL_ARRAY_BUFFER, buffer->vertex_offset, sizeof(struct vertex) * vertex_count, vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->indicebuffer);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, buffer->indice_offset, sizeof(GLuint) * indice_count, indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	//Calculate new size
	uint32_t new_vertex_offset = buffer->vertex_offset + vertex_count * sizeof(struct vertex);
	uint32_t new_indice_offset = buffer->indice_offset + indice_count * sizeof(GLuint);

	
	
	//set new values
	buffer->vertex_offset = new_vertex_offset;
	buffer->indice_offset = new_indice_offset;
	
	
	return handler;
}

void light_vertex_buffer_attribute_pointer(struct light_vertex_buffer *buffer, struct light_vertex_buffer_handler *handler)
{
	const GLvoid *global_vertex_offset = (const GLvoid *)(handler->vertex_offset + 0);
	const GLvoid *global_normal_offset = (const GLvoid *)(handler->vertex_offset + sizeof(struct vec3));
			
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->indicebuffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vertexbuffer);
			
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), global_vertex_offset);
			
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), global_normal_offset);

}

