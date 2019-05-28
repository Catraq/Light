#ifndef PLATFORM_H
#define PLATFORM_H


#define PLATFORM_PRESS GLFW_PRESS

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "math/vec.h"



int platform_initialize(void);
int platform_exit(void);
void platform_deinitialize(void);
void platform_resolution(uint32_t *width,  uint32_t *height);
void platform_mouse(struct vec2 *coord);
int32_t platform_key(uint8_t key);
int32_t platform_mouse_key(uint8_t key);
void platform_update(void);


#endif //PLATFORM_H
