#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdio.h>
#include <stdint.h>

#include <gl/glew.h>
#include <glfw/glfw3.h>

#include "math/vec.h"

#define PLATFORM_PRESS GLFW_PRESS

GLFWwindow* window;

int platform_initialize(void)
{
	window = 0;
	
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint( GLFW_SAMPLES, 16 );

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
	printf("Failed to init GLFW \n");
			glfwTerminate();
	return -1;
	}



	glfwMakeContextCurrent(window);
	
	/* Disable vertical sync */
	glfwSwapInterval(0);

   
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		printf("Failed to init Glew \n");
		glfwTerminate();
		return -1;
	}
	
	/* Clear any errors */
	while( glGetError() != GL_NONE)
	{
		glGetError();
	}
	
	return 0;
}

int platform_exit(void)
{
	return glfwWindowShouldClose(window);
}

void platform_deinitialize()
{
	
    glfwDestroyWindow(window);
    glfwTerminate();
}

void platform_resolution( uint32_t *width,  uint32_t *height )
{
	int x,y;
	glfwGetFramebufferSize(window, &x, &y);
	*width = (uint32_t)x;
	*height = (uint32_t)y;
}


void platform_mouse( vec2 *coord  )
{
	double x,y;
	glfwGetCursorPos(window, &x, &y);
	coord->x = (float)x;
	coord->y = (float)y;
}

int32_t platform_key( uint8_t key )
{
	return (int32_t)glfwGetKey( window, ( int )key );
}

void platform_update( )
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}


#endif //PLATFORM_H
