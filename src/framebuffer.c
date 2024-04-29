#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "framebuffer.h"

#include "error.h"

int light_framebuffer_initialize(struct light_framebuffer *framebuffer, int width, int height)
{
	GLuint color_texture, normal_texture, position_texture, composed_texture;

	glGenTextures(1, &normal_texture);
	glBindTexture(GL_TEXTURE_2D, normal_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	glGenTextures(1, &position_texture);
	glBindTexture(GL_TEXTURE_2D, position_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1, &color_texture);
	glBindTexture(GL_TEXTURE_2D, color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1, &composed_texture);
	glBindTexture(GL_TEXTURE_2D, composed_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	
	GLuint frame_buffer;
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	CHECK_GL_ERROR();
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, normal_texture, 0);	

	CHECK_GL_ERROR();
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, position_texture, 0);	

	CHECK_GL_ERROR();
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, color_texture, 0);	

	GLenum buffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
	};

	CHECK_GL_ERROR();
	glDrawBuffers(3, buffers);

	
	framebuffer->framebuffer 	= frame_buffer;
	framebuffer->normal_texture 	= normal_texture;
	framebuffer->position_texture 	= position_texture;
	framebuffer->color_texture 	= color_texture;
	framebuffer->composed_texture 	= composed_texture;

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "glCheckFramebufferStatus(GL_FRAMEBUFFER) failed. \n ");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		light_framebuffer_deinitialize(framebuffer);
		return -1;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return 0;
}

void light_framebuffer_deinitialize(struct light_framebuffer *framebuffer)
{
	glDeleteFramebuffers(1, &framebuffer->framebuffer);
	glDeleteTextures(1, &framebuffer->normal_texture);
	glDeleteTextures(1, &framebuffer->position_texture);
	glDeleteTextures(1, &framebuffer->color_texture);
	glDeleteTextures(1, &framebuffer->composed_texture);
}


void light_framebuffer_resize(struct light_framebuffer *framebuffer, int width, int height)
{
	glBindTexture(GL_TEXTURE_2D, framebuffer->normal_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, framebuffer->position_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, framebuffer->color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, framebuffer->composed_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);



}


