#ifndef PLATFORM_H
#define PLATFORM_H


#define PLATFORM_PRESS GLFW_PRESS

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "math/vec.h"



int light_platform_initialize(void);
int light_platform_exit(void);
void light_platform_deinitialize(void);
void light_platform_resolution(uint32_t *width,  uint32_t *height);
void light_platform_mouse(struct vec2 *coord);
int32_t light_platform_key(uint8_t key);
int32_t light_platform_mouse_key(uint8_t key);
void light_platform_update(void);


#endif //PLATFORM_H
