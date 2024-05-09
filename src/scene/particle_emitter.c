#include "scene/particle_emitter.h"



void light_scene_particle_emitter_deinitialize(struct light_scene_state_instance *instance)
{
	glDeleteBuffers(1, &instance->particle_emitter_instance.emitter_normal_buffer);
}


int light_scene_particle_emitter_initialize(struct light_scene_state_instance *instance, struct light_scene_state_build *build)
{
	
	
	uint32_t particle_emitter_buffer_size = sizeof(struct light_scene_particle_emitter_normal)*build->particle_emitter_build.emitter_normal_count + 4 * sizeof(uint32_t);
	glGenBuffers(1, &instance->particle_emitter_instance.emitter_normal_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, instance->particle_emitter_instance.emitter_normal_buffer);
	glBufferData(GL_UNIFORM_BUFFER, particle_emitter_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	uint32_t particle_lookup_buffer_size = build->particle_build.emitter_count * sizeof(uint32_t);
	glGenBuffers(1, &instance->particle_emitter_instance.emitter_lookup_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, instance->particle_emitter_instance.emitter_lookup_buffer);
	glBufferData(GL_UNIFORM_BUFFER, particle_lookup_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	return 0;
}

void light_scene_particle_emitter_commit_normal(
		struct light_scene_state_instance *instance, 
		struct light_scene_state_build *build,
		struct light_scene_particle_emitter_normal *normal,
		uint32_t normal_count)
{

	uint32_t lookup_index[build->particle_build.emitter_count];
	for(uint32_t i = 0; i < normal_count; i++)
	{
		for(uint32_t j = 0; j < normal[i].emitter_count; j++)
		{
			lookup_index[normal[i].emitter_offset + j] = i;	
		}	
	}
	for(uint32_t i = 0; i < build->particle_build.emitter_count; i++){
		printf("%u\n", lookup_index[i]);
	}


	uint32_t particle_lookup_buffer_size = build->particle_build.emitter_count * sizeof(uint32_t);
	glBindBuffer(GL_UNIFORM_BUFFER, instance->particle_emitter_instance.emitter_lookup_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, particle_lookup_buffer_size,  lookup_index);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	uint32_t particle_emitter_buffer_size = sizeof(struct light_scene_particle_emitter_normal) * normal_count;  
	glBindBuffer(GL_UNIFORM_BUFFER, instance->particle_emitter_instance.emitter_normal_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, particle_emitter_buffer_size, normal);
	glBufferSubData(GL_UNIFORM_BUFFER, particle_emitter_buffer_size, sizeof(uint32_t), &normal_count);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


