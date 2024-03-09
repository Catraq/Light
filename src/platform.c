#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_EGL
#include <GLFW/glfw3native.h>

#if 0
#include <CL/cl.h>
#endif 

#include <stdio.h>

#include "platform.h"


static GLFWwindow* window;

int light_platform_initialize(void)
{
	window = 0;
	
	/* Initialize openGL */	
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint( GLFW_SAMPLES, 16 );

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		fprintf(stderr,"Failed to init GLFW \n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	
	/* Disable vertical sync */
	glfwSwapInterval(0);

  	
       	/* Enable Version 3.3 */
	glewExperimental = 1;
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr,"Failed to init Glew \n");
		glfwTerminate();
		return -1;
	}


	/* Clear any errors */
	while( glGetError() != GL_NONE)
	{
		glGetError();
	}
	
	glfwSwapBuffers(window);
	glfwPollEvents();

	return 0;
}

int light_platform_exit(void)
{
	return glfwWindowShouldClose(window);
}

void light_platform_deinitialize(void)
{
	
    glfwDestroyWindow(window);
    glfwTerminate();
}

void light_platform_resolution(uint32_t *width,  uint32_t *height)
{
	int x,y;
	glfwGetFramebufferSize(window, &x, &y);
	*width = (uint32_t)x;
	*height = (uint32_t)y;
}


void light_platform_mouse(struct vec2 *coord)
{
	double x,y;
	glfwGetCursorPos(window, &x, &y);
	coord->x = (float)x;
	coord->y = (float)y;
}

int32_t light_platform_mouse_key(uint8_t key)
{
	return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
}

int32_t light_platform_key(uint8_t key)
{
	return (int32_t)glfwGetKey( window, ( int )key );
}

void light_platform_update(void)
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}


