#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_EGL
#include <GLFW/glfw3native.h>

#if 0
#include <CL/cl.h>
#endif 

#include <stdio.h>

#include "platform.h"


GLFWwindow* window;

int platform_initialize(void)
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
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr,"Failed to init Glew \n");
		glfwTerminate();
		return -1;
	}

#if 0	
	/* Init OpenCL */ 
	cl_platform_id platform_id;
	cl_device_id device_id;
	cl_uint device_count;
	cl_uint platform_count;

	cl_int ret = clGetPlatformIDs(1, &platform_id, &platform_count);
	if(ret != CL_SUCCESS){
		printf("CL: Failure \n");
	}
	
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &device_count);
	if(ret != CL_SUCCESS){
		printf("CL: Failure \n");
	}

	const cl_context_properties context_properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)glfwGetEGLContext(window),
		CL_EGL_DISPLAY_KHR, (cl_context_properties)glfwGetEGLDisplay(), 
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id, 
		0
	};

	cl_context context = clCreateContext(context_properties, 1, &device_id, NULL, NULL, &ret);
	if(ret != CL_SUCCESS){
		printf("CL: Failure \n");
	}

	cl_command_queue command_queue = clCreateCommandQueue(cl_context, device_id, 0, &ret);
	if(ret != CL_SUCCESS){
		printf("CL: Failure \n");
	}
#endif 

	/* Clear any errors */
	while( glGetError() != GL_NONE)
	{
		glGetError();
	}
	
	glfwSwapBuffers(window);
	glfwPollEvents();

	return 0;
}

int platform_exit(void)
{
	return glfwWindowShouldClose(window);
}

void platform_deinitialize(void)
{
	
    glfwDestroyWindow(window);
    glfwTerminate();
}

void platform_resolution(uint32_t *width,  uint32_t *height)
{
	int x,y;
	glfwGetFramebufferSize(window, &x, &y);
	*width = (uint32_t)x;
	*height = (uint32_t)y;
}


void platform_mouse(struct vec2 *coord)
{
	double x,y;
	glfwGetCursorPos(window, &x, &y);
	coord->x = (float)x;
	coord->y = (float)y;
}

int32_t platform_mouse_key(uint8_t key)
{
	return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
}

int32_t platform_key(uint8_t key)
{
	return (int32_t)glfwGetKey( window, ( int )key );
}

void platform_update(void)
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}


