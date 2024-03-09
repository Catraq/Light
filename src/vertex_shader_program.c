#include "vertex_shader_program.h"

		
#include <GL/glew.h>

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


