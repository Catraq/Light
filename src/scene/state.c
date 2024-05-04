#include "scene/state.h"

int light_scene_state_initialize(struct light_scene_state_instance *instance, struct light_scene_state_build *build)
{
	int result = 0;


	result = light_scene_implicit_initialize(instance, build);
	if(result < 0)
	{
		printf("light_implicit_init(): failed. \n");
		return -1;	
	}

	result = light_scene_particle_initialize(instance, build); 
	if(result < 0)
	{
		fprintf(stderr, "light_scene_particle_initialize() failed. \n");
		return -1;
	}

	return 0;
}

void light_scene_state_dispatch(struct light_scene_state_instance *instance, struct light_scene_state_build *build, struct light_framebuffer *framebuffer, uint32_t width, uint32_t height, float deltatime)
{	
	uint32_t index = instance->particle_instance.buffer_index;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, instance->particle_instance.position[index]);
		
	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, instance->particle_instance.velocity[index]);

	glActiveTexture(GL_TEXTURE0+2);
	glBindTexture(GL_TEXTURE_2D, instance->particle_instance.acceleration[index]);



	glBindBufferBase(GL_UNIFORM_BUFFER, 0, instance->implicit_instance.sphere_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, instance->implicit_instance.cylinder_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, instance->implicit_instance.box_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, instance->implicit_instance.light_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, instance->implicit_instance.object_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, instance->implicit_instance.object_node_buffer);


	light_scene_particle_simulate(instance, build, deltatime);

		

	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->framebuffer);
	light_scene_implicit_dispatch(instance, width, height);
}
