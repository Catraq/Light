#ifndef RENDER_FRAMEBUFFER_H_
#define RENDER_FRAMEBUFFER_H_

struct render_framebuffer
{
	GLuint normal_texture;
	GLuint position_texture;
	GLuint color_texture;

	GLuint depth_buffer;
	GLuint framebuffer;
};

void framebuffer_initialize(struct render_framebuffer *framebuffer, int width, int height);
void framebuffer_resize(struct render_framebuffer *framebuffer, int width, int height);

#endif 
