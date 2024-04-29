#include "error.h"

void glError(const char *msg)
{
	GLenum error_code = GL_NO_ERROR;
	while((error_code = glGetError()) != GL_NO_ERROR)
	{
		fprintf(stderr, msg);
		switch(error_code)
		{
			case GL_INVALID_ENUM:
				fprintf(stderr, "GL_INVALID_ENUM\n");	
				break;
			case GL_INVALID_VALUE:
				fprintf(stderr, "GL_INVALID_VALUE\n");	
				break;
			case GL_INVALID_OPERATION:
				fprintf(stderr, "GL_INVALID_OPERATION\n");	
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				fprintf(stderr, "GL_INVALID_FRAMEBUFFER_OPERATION\n");	
				break;
			case GL_OUT_OF_MEMORY:
				fprintf(stderr, "GL_OUT_OF_MEMORY\n");	
				break;
			case GL_STACK_UNDERFLOW:
				fprintf(stderr, "GL_STACK_UNDERFLOW\n");	
				break;
			case GL_STACK_OVERFLOW:
				fprintf(stderr, "GL_STACK_OVERFLOW\n");	
				break;
		
		}	
	}
}


