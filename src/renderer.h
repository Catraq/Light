#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>

#include "math/vec.h"
#include "math/mat4x4.h"


struct render_instance
{
	GLuint indice_offset;
	
	GLuint vertex_count;
	GLuint vertexarray;
	
	GLuint instance_count;
	GLuint instancebuffer;
	
	struct render_instance *next;
};

	

struct renderer
{
	/* Shader program */
	GLuint program;

	/* Scene uniform buffer */
	GLuint scenebuffer;
	
};
 	

void renderer_initialize(struct renderer *render);
void renderer_deinitialize(struct renderer *render);


void renderer_render_begin(renderer *render, mat4x4 *view, uint32_t width,  uint32_t height);

void renderer_instance_initialize(struct renderer *render, struct vertex_handle *handle, struct render_instance *instance);
void renderer_instance_renderer(struct renderer *render, struct render_instance *instance);


//void renderer_instance_dwrite_instances(render_instance *instance, physic_body *bodies, uint32_t count);
#endif //RENDERER_H
