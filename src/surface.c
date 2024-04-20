	

const GLchar light_surface_vertex_shader_source[] = 
{
	"#version 430 core 					\n"
	" layout ( location = 0 ) in vec2 r_position; 		\n"
	" out vec2 f_uv;					\n"
	" void main(){						\n"
	"	f_uv = (r_position + vec2(1))/2;		\n"
	" 	gl_Position = vec4(r_position, 0.0, 1.0);       \n"
	"} 							\n"
};


const GLchar light_surface_fragment_shader_source[] = {
	"#version 430 core 				\n"
	"uniform sampler2D in_texture;			\n"
	"out vec4 fcolor; 				\n"
	"in vec2 f_uv;					\n"
	"void main(){					\n"
	"	fcolor = texture(in_texture, f_uv);	\n"
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



struct light_surface
{
	GLuint program;
	GLuint vertex_array;
	GLuint vertex_buffer;
	GLuint element_buffer;
	GLuint draw_count;
};

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

int light_surface_initialize_vertex_fragement_source(struct light_surface *surface, const char *vertex_shader_source, const char *fragment_shader_source)
{

	GLuint light_surface_vertex_array, light_surface_vertex_buffer, light_surface_element_buffer;


	GLint light_surface_program = light_create_program(vertex_shader_source, fragment_shader_source);
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



void light_surface_render(struct light_surface *light_surface, GLuint texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
		
	glUseProgram(light_surface->program);
	glBindVertexArray(light_surface->vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_surface->element_buffer);
	glDrawElements(GL_TRIANGLE_STRIP, light_surface->draw_count, GL_UNSIGNED_INT, 0);	
	glBindVertexArray(0);

}

