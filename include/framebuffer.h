#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

struct light_framebuffer
{
	GLuint normal_texture;
	GLuint position_texture;
	GLuint color_texture;

	GLuint depth_buffer;
	GLuint framebuffer;
};

void light_framebuffer_initialize(struct light_framebuffer *framebuffer, int width, int height);
void light_framebuffer_resize(struct light_framebuffer *framebuffer, int width, int height);

#endif 
