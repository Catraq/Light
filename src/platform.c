#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_EGL
#include <GLFW/glfw3native.h>

#if 0
#include <CL/cl.h>
#endif 

#include <stdio.h>

#include "platform.h"



int light_platform_initialize(
		struct light_platform *platform
)
{
	/* Initialize openGL */	
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint( GLFW_SAMPLES, 16 );

	platform->window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!platform->window)
	{
		fprintf(stderr,"Failed to init GLFW \n");
		glfwTerminate();
		return -1;
	}

	int width_mm, height_mm;
	glfwGetMonitorPhysicalSize(glfwGetPrimaryMonitor(), &width_mm, &height_mm);
	platform->screen_width_mm = width_mm;
	platform->screen_height_mm = height_mm;

	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	platform->screen_resolution_x_pixel = mode->width;
	platform->screen_resolution_y_pixel = mode->height;


	glfwMakeContextCurrent(platform->window);
	
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
	
	glfwSwapBuffers(platform->window);
	glfwPollEvents();

	return 0;
}

void light_platform_deinitialize(
		struct light_platform *platform
)
{
    glfwDestroyWindow(platform->window);
    glfwTerminate();
}

int light_platform_exit(
		struct light_platform *platform
)
{
	return glfwWindowShouldClose(platform->window);
}


void light_platform_window_resolution(
		struct light_platform *platform,
		uint32_t *width,
	      	uint32_t *height
)
{
	int x,y;
	glfwGetFramebufferSize(platform->window, &x, &y);
	*width = (uint32_t)x;
	*height = (uint32_t)y;
}


void light_platform_mouse(
		struct light_platform *platform, 
		struct vec2 *coord
)
{
	double x,y;
	glfwGetCursorPos(platform->window, &x, &y);
	coord->x = (float)x;
	coord->y = (float)y;
}

int32_t light_platform_mouse_key(
		struct light_platform *platform,
		uint8_t key
)
{
	return glfwGetMouseButton(platform->window, GLFW_MOUSE_BUTTON_LEFT);
}

int32_t light_platform_key(
		struct light_platform *platform,
		uint8_t key
)
{
	return (int32_t)glfwGetKey(platform->window, ( int )key );
}

void light_platform_update(
		struct light_platform *platform
)
{
	glfwSwapBuffers(platform->window);
	glfwPollEvents();
}


