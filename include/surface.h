#ifndef LIGHT_SURFACE_H
#define LIGHT_SURFACE_H

#include "platform.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct light_surface
{
	GLuint program;
	GLuint vertex_array;
	GLuint vertex_buffer;
	GLuint element_buffer;
	GLuint draw_count;
};

int light_surface_initialize(
		struct light_surface *surface
);

int light_surface_initialize_vertex_fragement_source(
		struct light_surface *surface,
	       	const char **vertex_source, uint32_t *vertex_source_length,  uint32_t vertex_source_count,
	       	const char **fragment_source, uint32_t *fragment_source_length, uint32_t fragment_source_count
);

void light_surface_deinitialize(
		struct light_surface *surface
);


void light_surface_render(
		struct light_surface *light_surface
);

void light_surface_render_textured(
		struct light_surface *light_surface,
	       	GLuint texture
);


void light_surface_render_instanced(
		struct light_surface *light_surface,
	       	uint32_t instance_count
);

#endif 

