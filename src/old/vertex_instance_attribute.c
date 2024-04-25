






void light_translation_attribute_pointer(GLuint location, GLsizei stride, GLuint offset)
{	

	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(offset));
	glVertexAttribDivisor(location, 1);
	location++;
	
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(sizeof(struct vec4) + offset));
	glVertexAttribDivisor(location, 1);
	location++;

	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(2*sizeof(struct vec4) + offset));
	glVertexAttribDivisor(location, 1);
	location++;	
	
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(3*sizeof(struct vec4) + offset));
	glVertexAttribDivisor(location, 1);

}


typedef enum 
{
	VERTEX_INSTANCE_ATTRIBUTE_MAT4X4,
}vertex_instance_attribute_type_t;

int light_vertex_instance_attribute(struct light_vertex_instance *instance, GLuint instance_buffer_offset, GLuint program, const char *attribute_name, vertex_instance_attribute_type_t attribute_type)
{
	assert(instance);
	assert(attribute_name);


	GLint translation_index = glGetAttribLocation(program, attribute_name);
	if(translation_index == -1){
		fprintf(stderr, "glGetAttribLocation(): could not find %s .\n", attribute_name);
		return -1;	
	}


	glBindVertexArray(instance->vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, instance->instance_buffer);
	switch(attribute_type)
	{
		case VERTEX_INSTANCE_ATTRIBUTE_MAT4X4:
			light_translation_attribute_pointer(translation_index, sizeof(struct mat4x4), instance_buffer_offset);
			break;
		default:
			assert(0);	
	};

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return 0;
}

