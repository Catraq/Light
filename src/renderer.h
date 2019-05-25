#ifndef RENDERER_H
#define RENDERER_H

#include <gl/glew.h>

#include "math/vec.h"
#include "math/mat4x4.h"

#include "physic.h"


struct render_instance
{
	
	GLuint indice_offset;
	
	GLuint vertex_count;
	GLuint vertexarray;
	
	GLuint instance_count;
	GLuint instancebuffer;
	
	struct	render_instance *next;
};
	

struct renderer
{
	/* Shader program */
	GLuint program;

	/* Scene uniform buffer */
	GLuint scenebuffer;
	
	/* First to render */
	struct render_instance *head;
};
 	

void renderer_initialize( renderer *render )
{
	const char vertex_shader_source[] = 
	{
		"#version 330 core 									\n"
		" layout ( location = 0 ) in vec3 r_position; 		\n"
		" layout ( location = 1 ) in vec3 r_normal;   		\n"
		" layout ( location = 2 ) in mat4 r_model; 			\n"
		" out vec3 normal; 									\n"
		" uniform scene{ 									\n"
		"	mat4 view; 										\n"
		" };												\n"
		" void main(){								  					  	\n"
		" 	normal = r_normal;												\n"
		" 	gl_Position = view  * r_model *vec4(r_position , 1.0f );        \n"
		"} 																  	\n"
	};

	const char fragment_shader_source[] = 
	{
		"#version 330 core 				\n"
		"out vec4 fcolor; 				\n"
		"in vec3  normal;				\n"
		"void main(){					\n"
		"	fcolor = vec4(vec3(1,1,1)- normal,0);		\n"
		"}								\n"
	};
		
	render->head = 0;
	
	
	
	render->program 	   = glCreateProgram();
	GLuint vertex_shader   = glCreateShader( GL_VERTEX_SHADER );
	GLuint fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );
	
	const GLchar *vertex_source = (const GLchar *)vertex_shader_source;
	const GLchar *fragment_source = (const GLchar *)fragment_shader_source;
	
	
	glShaderSource( vertex_shader, 1,  &vertex_source, 0  );
	glShaderSource( fragment_shader, 1, &fragment_source, 0  );
	
	glCompileShader( vertex_shader );
	glCompileShader( fragment_shader );
	
	
	GLint v_compiled = GL_FALSE;
	GLint f_compiled = GL_FALSE;
	
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &v_compiled);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &f_compiled);
	
	if( v_compiled == GL_FALSE 
	||  f_compiled == GL_FALSE )
	{
		
		GLchar log[255];
		GLsizei length;
		
		glGetShaderInfoLog( vertex_shader, 255, &length, (GLchar*)&log);
		if( length != 0 )
		{
			printf(" ---- Vertexshader log ---- \n %s \n", (GLchar*)&log);
		}
		
		length = 0;
		glGetShaderInfoLog( fragment_shader, 255, &length, (GLchar*)&log);
		if( length != 0)
		{
			printf(" ---- Fragmentshader log ---- \n %s \n", (GLchar*)&log);
		}
		
	}
	
	
	
	glAttachShader(render->program, vertex_shader);
	glAttachShader(render->program, fragment_shader);
	
	glLinkProgram(render->program);
	
	glDetachShader(render->program, vertex_shader);
	glDetachShader(render->program, fragment_shader);
	
	glDeleteShader( vertex_shader );
	glDeleteShader( fragment_shader );
	
	GLint linked = GL_FALSE;
	glGetProgramiv( render->program, GL_LINK_STATUS, &linked);
	if( linked == GL_FALSE ) {
		GLsizei length;
		GLchar log[255];
		glGetProgramInfoLog( render->program, 255, &length, (GLchar*)&log);
		printf(" ---- Link log ---- \n %s \n", (GLchar*)&log);
	}
	
	
	const GLuint block_location = 0;
	glGenBuffers( 1, &render->scenebuffer);
	glBindBuffer( GL_UNIFORM_BUFFER, render->scenebuffer );
	glBufferData( GL_UNIFORM_BUFFER, sizeof( mat4x4 ), 0, GL_STREAM_DRAW );
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );
	

	glBindBufferBase( GL_UNIFORM_BUFFER, block_location, render->scenebuffer );
	GLuint block_index = glGetUniformBlockIndex(render->program, "scene");
	glUniformBlockBinding( render->program, block_index, block_location );
}

void renderer_deinitialize( renderer *render )
{
	glDeleteProgram( render->program );
	glDeleteBuffers( 1, &render->scenebuffer );
	render->head = 0;
}

void renderer_render( renderer *render, mat4x4 *view, uint32_t width,  uint32_t height )
{
	glViewport( 0,0 ,width, height);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	
	{
		glBindBuffer( GL_UNIFORM_BUFFER, render->scenebuffer );
		mat4x4 *view_dest = ( mat4x4 *)glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
		memcpy( view_dest, view, sizeof( mat4x4 ) );
		glUnmapBuffer( GL_UNIFORM_BUFFER );
		glBindBuffer( GL_UNIFORM_BUFFER, 0 );
	}

	glUseProgram( render->program );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	
	render_instance *curr = render->head;
	while( curr )
	{
		
		glBindVertexArray( curr->vertexarray );
		
		glDrawElementsInstanced(GL_TRIANGLES, 
			  curr->vertex_count, 
			  GL_UNSIGNED_INT, 
			  (const GLuint*)(curr->indice_offset * sizeof( uint32_t )), 
			  curr->instance_count
		);
		
		glBindVertexArray( 0 );
		
		curr = curr->next;
	}
	
	while( glGetError() != GL_NONE)
	{
		printf( "GLerror\n");
		getchar();
	}
	
}

void renderer_instance_dwrite_instances( render_instance *instance, physic_body *bodies, uint32_t count )
{

	instance->instance_count = count;

	glBindBuffer( GL_ARRAY_BUFFER, instance->instancebuffer );
	glBufferData( GL_ARRAY_BUFFER, count * sizeof( physic_body ), bodies, GL_STREAM_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}
	
void renderer_instance_initialize( renderer *render, render_instance *instance, vertex_buffer *buffer, vertex_buffer_handler *handler)
{
	//Make first in list 		
	instance->next = render->head;
	render->head = instance;
	
	instance->indice_offset = handler->indice_offset; 
	
	//Gen input array 
	glGenVertexArrays( 1, &instance->vertexarray );
	glBindVertexArray( instance->vertexarray );
	
	
	//Set up instanced rendering 
	instance->instance_count = 0;
	
	glGenBuffers( 1, &instance->instancebuffer );
	glBindBuffer( GL_ARRAY_BUFFER, instance->instancebuffer );
	
	
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, sizeof( physic_body ), 0 );
	glVertexAttribDivisor( 2, 1);
	
	glEnableVertexAttribArray( 3 );
	glVertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE, sizeof( physic_body ), ( const GLvoid*)( sizeof( vec4 ) ));
	glVertexAttribDivisor( 3, 1);
	
	glEnableVertexAttribArray( 4 );
	glVertexAttribPointer( 4, 4, GL_FLOAT, GL_FALSE, sizeof( physic_body ), ( const GLvoid*)( 2*sizeof( vec4 ) ));
	glVertexAttribDivisor( 4, 1);
	
	glEnableVertexAttribArray( 5 );
	glVertexAttribPointer( 5, 4, GL_FLOAT, GL_FALSE, sizeof( physic_body ), ( const GLvoid*)( 3*sizeof( vec4 ) ));
	glVertexAttribDivisor( 5, 1);
	
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	
	//Set up render data from physic instance 
	instance->vertex_count = handler->indice_count;
		
	const GLvoid *global_vertex_offset = (const GLvoid *)((handler->vertex_offset + 0) * sizeof( vertex ));
	const GLvoid *global_normal_offset = (const GLvoid *)((handler->vertex_offset + 1) * sizeof( vertex ) + sizeof( vec3 ));
	
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffer->indicebuffer );
	glBindBuffer( GL_ARRAY_BUFFER, 		   buffer->vertexbuffer);
	
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( vertex ), global_vertex_offset);
	
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( vertex ), global_normal_offset);
	
	glBindVertexArray( 0 );
}

#endif //RENDERER_H
