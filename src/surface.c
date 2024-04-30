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

GLint light_create_program(const char *vertex_source, const char *fragment_source)
{
	
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	
	
	const GLchar *vsource = (const GLchar *)vertex_source;
	const GLchar *fsource = (const GLchar *)fragment_source;

	glShaderSource(vertex_shader, 1, &vsource, 0);
	glShaderSource(fragment_shader, 1, &fsource, 0);
	
	glCompileShader(vertex_shader);
	glCompileShader(fragment_shader);

	
	GLint compiled = GL_FALSE;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE){
		GLchar log[255];
		GLsizei length;
		glGetShaderInfoLog(vertex_shader, 255, &length, (GLchar*)&log);
		if( length != 0 )
		{
			printf(" ---- Vertexshader compile log ---- \n %s \n", (GLchar*)&log);
		}

		/* Cleanup and return */	
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return -1;
	}

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE){
		GLchar log[255];
		GLsizei length;
		glGetShaderInfoLog(fragment_shader, 255, &length, (GLchar*)&log);
		if( length != 0 )
		{
			printf(" ---- Fragmentshader compile log ---- \n %s \n", (GLchar*)&log);
		}

		/* Cleanup and return */	
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return -1;
	}
	
	
	GLuint program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	
	glLinkProgram(program);
	
	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	
	GLint linked = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if(linked == GL_FALSE) {
		GLsizei length;
		GLchar log[255];
		glGetProgramInfoLog(program, 255, &length, (GLchar*)&log);
		printf(" ---- Program Link log ---- \n %s \n", (GLchar*)&log);

		glDeleteProgram(program);
		return -1;
	}

	
	return program;
}


GLuint light_shader_vertex_create(const char **vertex_source, uint32_t *vertex_source_length,  uint32_t vertex_source_count, const char **fragment_source, uint32_t *fragment_source_length, uint32_t fragment_source_count)
{
	
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	
	
	glShaderSource(vertex_shader, vertex_source_count, vertex_source, vertex_source_length);
	glShaderSource(fragment_shader, fragment_source_count, fragment_source, fragment_source_length);

	glCompileShader(vertex_shader);
	glCompileShader(fragment_shader);

	
	GLint compiled = GL_FALSE;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE){
		GLchar log[8192];
		GLsizei length;
		glGetShaderInfoLog(vertex_shader, 8192, &length, log);
		if( length != 0 )
		{
			printf(" ---- Vertexshader compile log ---- \n %s \n", log);
		}

		/* Cleanup and return */	
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return 0;
	}

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE){
		GLchar log[8192];
		GLsizei length;
		glGetShaderInfoLog(fragment_shader, 8192, &length, log);
		if( length != 0 )
		{
			printf(" ---- Fragmentshader compile log ---- \n %s \n", log);
		}

		/* Cleanup and return */	
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return 0;
	}
	
	
	GLuint program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	
	glLinkProgram(program);
	
	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	
	GLint linked = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if(linked == GL_FALSE) {
		GLsizei length;
		GLchar log[8192];
		glGetProgramInfoLog(program, 8192, &length, log);
		printf(" ---- Program Link log ---- \n %s \n", log);

		glDeleteProgram(program);
		return 0;
	}

	
	return program;
}

int light_surface_initialize(struct light_surface *surface)
{

	GLuint light_surface_vertex_array, light_surface_vertex_buffer, light_surface_element_buffer;


	GLint light_surface_program = light_create_program(light_surface_vertex_shader_source, light_surface_fragment_shader_source);
	if(light_surface_program < 0){
		fprintf(stderr, "create_program(): failed \n");
		return -1;
	}

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
	
	const GLchar *uniform_name = "in_texture";	
	GLint shader_sampler_location = glGetUniformLocation(light_surface_program, uniform_name);
	if(shader_sampler_location == -1){
		fprintf(stderr, "glGetUniformLocation(): could not find %s .\n", uniform_name);
		return -1;	
	}

	glUseProgram(light_surface_program);
	glUniform1i(shader_sampler_location, 0);


	surface->program = light_surface_program;
	surface->vertex_array = light_surface_vertex_array;
	surface->element_buffer = light_surface_element_buffer;
	surface->vertex_buffer = light_surface_vertex_buffer;
	surface->draw_count = light_surface_quad_indices_count;

	return 0;

}

int light_surface_initialize_vertex_fragement_source(struct light_surface *surface, const char **vertex_source, uint32_t *vertex_source_length,  uint32_t vertex_source_count, const char **fragment_source, uint32_t *fragment_source_length, uint32_t fragment_source_count)
{

	GLuint light_surface_vertex_array, light_surface_vertex_buffer, light_surface_element_buffer;

	GLint light_surface_program = light_shader_vertex_create(vertex_source, vertex_source_length, vertex_source_count, fragment_source, fragment_source_length, fragment_source_count);
	if(light_surface_program == 0){
		fprintf(stderr, "light_shader_vertex_create(): failed \n");
		return -1;
	}

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
	
	surface->program = light_surface_program;
	surface->vertex_array = light_surface_vertex_array;
	surface->element_buffer = light_surface_element_buffer;
	surface->vertex_buffer = light_surface_vertex_buffer;
	surface->draw_count = light_surface_quad_indices_count;

	return 0;

}

void light_surface_deinitialize(struct light_surface *surface)
{
	
	glDeleteVertexArrays(1, &surface->vertex_array);
	glDeleteBuffers(1, &surface->element_buffer);
	glDeleteBuffers(1, &surface->vertex_buffer);
	glDeleteProgram(surface->program);
}


void light_surface_render(struct light_surface *light_surface)
{
	glUseProgram(light_surface->program);
	glBindVertexArray(light_surface->vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_surface->element_buffer);
	glDrawElements(GL_TRIANGLE_STRIP, light_surface->draw_count, GL_UNSIGNED_INT, 0);	
	glBindVertexArray(0);

}


void light_surface_render_textured(struct light_surface *light_surface, GLuint texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
		
	glUseProgram(light_surface->program);
	glBindVertexArray(light_surface->vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_surface->element_buffer);
	glDrawElements(GL_TRIANGLE_STRIP, light_surface->draw_count, GL_UNSIGNED_INT, 0);	
	glBindVertexArray(0);

}

