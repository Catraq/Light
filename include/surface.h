#ifndef LIGHT_SURFACE_H
#define LIGHT_SURFACE_H

#include "platform.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct light_surface
{
	GLuint vertex_array;
	GLuint vertex_buffer;
	GLuint element_buffer;
	GLuint draw_count;
};

struct light_surface_textured
{
	struct light_surface surface;
	GLuint program;
};

int light_surface_initialize(
		struct light_surface *surface
);

int light_surface_textured_initialize(
		struct light_surface_textured *surface
);

void light_surface_deinitialize(
		struct light_surface *surface
);

void light_surface_textured_deinitialize(
		struct light_surface_textured *surface
);


void light_surface_render(
		struct light_surface *light_surface
);

void light_surface_render_instanced(
		struct light_surface *light_surface,
	       	uint32_t instance_count
);

void light_surface_textured_render(
		struct light_surface_textured *light_surface,
	       	GLuint texture
);


#endif 

