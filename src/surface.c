#include "surface.h"	

const GLchar light_surface_vertex_shader_source[] = 
{
	"#version 430 core 					\n"
	" layout ( location = 0 ) in vec2 r_position; 		\n"
	" out vec2 fragcoord;					\n"
	" void main(){						\n"
	"	fragcoord = (r_position + vec2(1))/2;		\n"
	" 	gl_Position = vec4(r_position, 0.0, 1.0);       \n"
	"} 							\n"
};


const GLchar light_surface_fragment_shader_source[] = {
	"#version 430 core 				\n"
	"uniform sampler2D in_texture;			\n"
	"out vec4 fcolor; 				\n"
	"in vec2 fragcoord;					\n"
	"void main(){					\n"
	"	fcolor = texture(in_texture, fragcoord);\n"
	"}						\n"
};

const struct vec2 light_surface_quad_vertices[] = {
	{-1.0f, 1.0f}, 
	{1.0f, 1.0f},
	{1.0, -1.0f},
	{-1.0f, -1.0f}
};	

const GLuint light_surface_quad_vertices_count = sizeof(light_surface_quad_vertices)/sizeof(light_surface_quad_vertices[0]);

const GLuint light_surface_quad_indices[] = {
	1, 0, 2,
	2, 0, 3
};

const GLuint light_surface_quad_indices_count = sizeof(light_surface_quad_indices)/sizeof(light_surface_quad_indices[0]);

int light_surface_initialize(struct light_surface *surface)
{

	GLuint light_surface_vertex_array, light_surface_vertex_buffer, light_surface_element_buffer;

	glGenVertexArrays(1, &light_surface_vertex_array);
	glBindVertexArray(light_surface_vertex_array);

	glGenBuffers(1, &light_surface_element_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_surface_element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(light_surface_quad_indices), light_surface_quad_indices, GL_STATIC_DRAW); 


	glGenBuffers(1, &light_surface_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, light_surface_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(light_surface_quad_vertices), light_surface_quad_vertices, GL_STATIC_DRAW); 

	const GLuint light_surface_vertex_index = 0;
	glEnableVertexAttribArray(light_surface_vertex_index);
	glVertexAttribPointer(light_surface_vertex_index, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	surface->vertex_array = light_surface_vertex_array;
	surface->element_buffer = light_surface_element_buffer;
	surface->vertex_buffer = light_surface_vertex_buffer;
	surface->draw_count = light_surface_quad_indices_count;

	return 0;

}


int light_surface_textured_initialize(
		struct light_surface_textured *surface
)
{
	int result = 0;
	result = light_surface_initialize(&surface->surface);	
	if(result < 0)
	{
		fprintf(stderr, "light_surface_initialize() failed. \n");
		return -1;
	}

	
	GLchar *vertex_source[] = {
		light_surface_vertex_shader_source
	};
	
	GLchar *fragment_source[] = {
		light_surface_fragment_shader_source
	};


	surface->program = light_shader_vertex_create(
			vertex_source, NULL, 1,
		       	fragment_source, NULL, 1
	);

	if(surface->program == 0){
		light_surface_deinitialize(&surface->surface);
		fprintf(stderr, "light_shader_vertex_create() failed. \n");
		return -1;
	}


	return 0;
}


void light_surface_deinitialize(struct light_surface *surface)
{
	glDeleteVertexArrays(1, &surface->vertex_array);
	glDeleteBuffers(1, &surface->element_buffer);
	glDeleteBuffers(1, &surface->vertex_buffer);
}

void light_surface_textured_deinitialize(struct light_surface_textured *surface)
{
	light_surface_deinitialize(&surface->surface);
	glDeleteProgram(surface->program);
}



void light_surface_render(struct light_surface *light_surface)
{
	glBindVertexArray(light_surface->vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_surface->element_buffer);
	glDrawElements(GL_TRIANGLE_STRIP, light_surface->draw_count, GL_UNSIGNED_INT, 0);	
	glBindVertexArray(0);

}

void light_surface_render_instanced(struct light_surface *light_surface, uint32_t instance_count)
{
	glBindVertexArray(light_surface->vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_surface->element_buffer);
	glDrawElementsInstanced(GL_TRIANGLE_STRIP, light_surface->draw_count, GL_UNSIGNED_INT, 0, instance_count);	
	glBindVertexArray(0);
}



void light_surface_textured_render(struct light_surface_textured *light_surface, GLuint texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
		
	glUseProgram(light_surface->program);
	glBindVertexArray(light_surface->surface.vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_surface->surface.element_buffer);
	glDrawElements(GL_TRIANGLE_STRIP, light_surface->surface.draw_count, GL_UNSIGNED_INT, 0);	
	glBindVertexArray(0);

}

