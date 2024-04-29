#ifndef LIGHT_ERROR_H
#define LIGHT_ERROR_H 

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION __FILE__ ":" S2(__LINE__) ":"

#define CHECK_GL_ERROR() glError(LOCATION)

#include <stdio.h>

#include "platform.h"

void glError(const char *msg);

#endif 
