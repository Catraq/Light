#ifndef PLATFORM_H
#define PLATFORM_H


#define PLATFORM_PRESS GLFW_PRESS

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "math/vec.h"

struct light_platform 
{
	GLFWwindow* window;

	uint32_t screen_width_mm;
	uint32_t screen_height_mm;

	uint32_t screen_resolution_x_pixel;
	uint32_t screen_resolution_y_pixel;
};



int light_platform_initialize(
	struct light_platform *platform
);

void light_platform_deinitialize(
	struct light_platform *platform
);

int light_platform_exit(
	struct light_platform *platform
);

void light_platform_window_resolution(
		struct light_platform *platform,
		uint32_t *width,
	      	uint32_t *height
);

void light_platform_mouse(
		struct light_platform *platform,
		struct vec2 *coord
);

int32_t light_platform_key(
		struct light_platform *platform,
		uint8_t key
);

int32_t light_platform_mouse_key(
		struct light_platform *platform,
		uint8_t key
);

void light_platform_update(
		struct light_platform *platform
);


#endif //PLATFORM_H
