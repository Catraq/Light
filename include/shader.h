#ifndef LIGHT_SHADER_H
#define LIGHT_SHADER_H

#include "platform.h"

GLuint light_shader_compute_create(
		const char **compute_source, 
		uint32_t *compute_source_length, 
		uint32_t compute_source_count
);


GLuint light_shader_vertex_create(
		const char **vertex_source, 
		uint32_t *vertex_source_length, 
		uint32_t vertex_source_count, 
		const char **fragment_source,
	       	uint32_t *fragment_source_length,
	       	uint32_t fragment_source_count
);
#endif 
