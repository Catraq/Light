#include "scene/particle.h"

static const GLchar light_vertex_shader_source[] = 
{
	"#version 430 core 					\n"
	" layout ( location = 0 ) in vec2 r_position; 		\n"
	" out vec2 fragcoord;					\n"
	" void main(){						\n"
	"	fragcoord = (r_position + vec2(1))/2;		\n"
	" 	gl_Position = vec4(r_position, 0.0, 1.0);       \n"
	"} 							\n"
};


void light_scene_particle_deinitialize(struct light_scene_state_instance* instance) 
{
	glDeleteProgram(instance->particle_instance.program);

	glDeleteTextures(2, instance->particle_instance.position);
	glDeleteTextures(2, instance->particle_instance.velocity);
	glDeleteTextures(2, instance->particle_instance.acceleration);
	glDeleteFramebuffers(2, instance->particle_instance.framebuffer);
	
}

int light_scene_particle_initialize(struct light_scene_state_instance* instance) 
{

	const char *particle_shader_filename = "../data/particle.txt";
	char particle_shader_source[2*8192];
	size_t read = light_file_read_buffer(particle_shader_filename, particle_shader_source, 2*8192);
	if(read == 0){
		fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", particle_shader_filename);
		return -1;
	}

	char particle_shader_source_header[512];
	uint32_t gen = snprintf(particle_shader_source_header, 512, 
		"#version 430 core \n" 
		"#define EMITTER_COUNT %u \n"
		"#define EMITTER_PARTICLE_COUNT %u \n"
		"#define EMITTER_NORMAL_COUNT %u \n"
		"#define OBJECT_NODE_COUNT %u \n"
		"#define LIGHT_COUNT %u \n",
		instance->build.particle_build.emitter_count,
		instance->build.particle_build.emitter_particle_count,
		instance->build.particle_emitter_build.emitter_normal_count,
		instance->build.implicit_build.object_node_count,
		instance->build.implicit_build.light_count

	); 

	const GLchar *source_fragment[] =
	{
		particle_shader_source_header,
		particle_shader_source,
	};

	uint32_t length_fragment[] = {
		gen, read
	};

	const GLchar *source_vertex[] = {
		light_vertex_shader_source	
	};


	instance->particle_instance.program = light_shader_vertex_create(
			source_vertex, NULL, 1,
			source_fragment, length_fragment, 2
	);


	if(instance->particle_instance.program == 0)
	{
		fprintf(stderr, "light_shader_vertex_create() failed. \n");
		return -1;	
	}

	const char *uniform_name = "deltatime";
	instance->particle_instance.program_deltatime_location = glGetUniformLocation(instance->particle_instance.program, uniform_name);
	if(instance->particle_instance.program_deltatime_location == -1){

		glDeleteProgram(instance->particle_instance.program);

		fprintf(stderr, "glGetUniformLocation(%s) failed. \n", uniform_name);
		return -1;
	} 


	uint32_t width = instance->build.particle_build.emitter_particle_count;
	uint32_t height = instance->build.particle_build.emitter_count;


	for(uint32_t i = 0; i < 2; i++)
	{
		glGenTextures(1, &instance->particle_instance.position[i]);
		glBindTexture(GL_TEXTURE_2D, instance->particle_instance.position[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


		glGenTextures(1, &instance->particle_instance.velocity[i]);
		glBindTexture(GL_TEXTURE_2D, instance->particle_instance.velocity[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glGenTextures(1, &instance->particle_instance.acceleration[i]);
		glBindTexture(GL_TEXTURE_2D, instance->particle_instance.acceleration[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glGenTextures(1, &instance->particle_instance.lifetime[i]);
		glBindTexture(GL_TEXTURE_2D, instance->particle_instance.lifetime[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


		
		glGenFramebuffers(1, &instance->particle_instance.framebuffer[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, instance->particle_instance.framebuffer[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, instance->particle_instance.position[i], 0);	
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, instance->particle_instance.velocity[i], 0);	
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, instance->particle_instance.acceleration[i], 0);	
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, instance->particle_instance.lifetime[i], 0);	

		GLenum buffers[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3,
		};

		glDrawBuffers(4, buffers);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, instance->particle_instance.framebuffer[0]);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "glCheckFramebufferStatus(GL_FRAMEBUFFER) failed. \n ");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		light_scene_particle_deinitialize(instance);
		return -1;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, instance->particle_instance.framebuffer[1]);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "glCheckFramebufferStatus(GL_FRAMEBUFFER) failed. \n ");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		light_scene_particle_deinitialize(instance);
		return -1;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	instance->particle_instance.buffer_index = 0; 

	return 0;
}

void light_scene_particle_simulate(struct light_scene_state_instance* instance, float deltatime)
{

	uint32_t index = instance->particle_instance.buffer_index;
	glBindFramebuffer(GL_FRAMEBUFFER, instance->particle_instance.framebuffer[(index+1)%2]);

	glUseProgram(instance->particle_instance.program);
	glUniform1f(instance->particle_instance.program_deltatime_location, deltatime);
	
	light_surface_render(&instance->surface);

	instance->particle_instance.buffer_index = (instance->particle_instance.buffer_index+1)%2; 

}

