#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "render_framebuffer.h"

void framebuffer_initialize(struct render_framebuffer *framebuffer, int width, int height)
{
	GLuint frame, normal_texture, position_texture, depth_buffer;
	glGenFramebuffers(1, &frame);
	glBindFramebuffer(GL_FRAMEBUFFER, frame);

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

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, normal_texture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, position_texture, 0);

	
	glGenRenderbuffers(1, &depth_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

	GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, draw_buffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		fprintf(stderr, "Error: glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE \n");
	}

	framebuffer->normal_texture 	= normal_texture;
	framebuffer->position_texture 	= position_texture;
	framebuffer->depth_buffer 	= depth_buffer;
	framebuffer->framebuffer 	= frame;
}

void framebuffer_resize(struct render_framebuffer *framebuffer, int width, int height)
{
	glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->depth_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

	glBindTexture(GL_TEXTURE_2D, framebuffer->normal_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, framebuffer->position_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

}


