#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include "platform.h"

struct light_framebuffer
{
	GLuint framebuffer;
	
	GLuint depth_texture;
	GLuint normal_texture;
	GLuint position_texture;
	GLuint color_texture;
	GLuint composed_texture;
};


int light_framebuffer_initialize(struct light_framebuffer *framebuffer, int width, int height);
void light_framebuffer_deinitialize(struct light_framebuffer *framebuffer);
void light_framebuffer_resize(struct light_framebuffer *framebuffer, int width, int height);

#endif 
