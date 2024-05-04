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


void light_scene_particle_deinitialize(struct light_scene_particle_instance* instance) 
{
	light_surface_deinitialize(&instance->surface);

	glDeleteBuffers(1, &instance->emitter_buffer);

	glDeleteTextures(2, instance->position);
	glDeleteTextures(2, instance->velocity);
	glDeleteTextures(2, instance->acceleration);
	glDeleteFramebuffers(2, instance->framebuffer);
	
}

int light_scene_particle_initialize(struct light_scene_state_instance* instance, struct light_scene_state_build* build) 
{

	const char *particle_shader_filename = "../data/particle.txt";
	uint8_t particle_shader_source[2*8192];
	size_t read = light_file_read_buffer(particle_shader_filename, particle_shader_source, 2*8192);
	if(read == 0){
		fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", particle_shader_filename);
		return -1;
	}

	uint8_t particle_shader_source_header[512];
	uint32_t gen = snprintf(particle_shader_source_header, 512, 
		"#version 430 core \n" 
		"#define EMITTER_COUNT %u \n"
		"#define EMITTER_INSTANCE_COUNT %u \n"
		"#define EMITTER_PARTICLE_COUNT %u \n"
		"#define OBJECT_NODE_STACK %u \n"
		"#define OBJECT_NODE_COUNT %u \n"
		"#define OBJECT_COUNT %u \n"
		"#define SPHERE_COUNT %u \n"
		"#define BOX_COUNT %u \n"
		"#define CYLINDER_COUNT %u \n"
		"#define LIGHT_COUNT %u \n",
		build->particle_build.emitter_count,
		build->particle_build.emitter_instance_count,
		build->particle_build.emitter_particle_count,
		build->implicit_build.object_node_max_level,
		build->implicit_build.object_node_count,
		build->implicit_build.object_count,
		build->implicit_build.sphere_count,
		build->implicit_build.box_count,
		build->implicit_build.cylinder_count,
		build->implicit_build.light_count

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


	int result = light_surface_initialize_vertex_fragement_source(
			&instance->particle_instance.surface, 
			source_vertex, NULL, 1,
			source_fragment, length_fragment, 2
	);

	if(result < 0)
	{
		fprintf(stderr, "light_surface_initialize_vertex_fragement_source() failed. \n");
		return -1;	
	}

	const char *uniform_name = "deltatime";
	instance->particle_instance.program_deltatime_location = glGetUniformLocation(instance->particle_instance.surface.program, uniform_name);
	if(instance->particle_instance.program_deltatime_location == -1){
		light_surface_deinitialize(&instance->particle_instance.surface);
		fprintf(stderr, "glGetUniformLocation(%s) failed. \n", uniform_name);
		return -1;
	} 



	uint32_t emitter_buffer_size = sizeof(struct light_scene_particle_emitter_instance) * build->particle_build.emitter_instance_count + 4*sizeof(uint32_t);
	glGenBuffers(1, &instance->particle_instance.emitter_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, instance->particle_instance.emitter_buffer);
	glBufferData(GL_UNIFORM_BUFFER, emitter_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	uint32_t width = build->particle_build.emitter_particle_count;
	uint32_t height = build->particle_build.emitter_count;


	for(uint32_t i = 0; i < 2; i++)
	{
		glGenTextures(1, &instance->particle_instance.position[i]);
		glBindTexture(GL_TEXTURE_2D, instance->particle_instance.position[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


		glGenTextures(1, &instance->particle_instance.velocity[i]);
		glBindTexture(GL_TEXTURE_2D, instance->particle_instance.velocity[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glGenTextures(1, &instance->particle_instance.acceleration[i]);
		glBindTexture(GL_TEXTURE_2D, instance->particle_instance.acceleration[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		
		glGenFramebuffers(1, &instance->particle_instance.framebuffer[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, instance->particle_instance.framebuffer[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, instance->particle_instance.position[i], 0);	
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, instance->particle_instance.velocity[i], 0);	
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, instance->particle_instance.acceleration[i], 0);	

		GLenum buffers[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
		};

		glDrawBuffers(3, buffers);
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

void light_scene_particle_simulate(struct light_scene_state_instance* instance, struct light_scene_state_build *build, float deltatime)
{

	uint32_t width = build->particle_build.emitter_particle_count;
	uint32_t height = build->particle_build.emitter_count;

	glViewport(0, 0, width, height);

	
	uint32_t index = instance->particle_instance.buffer_index;
	glBindFramebuffer(GL_FRAMEBUFFER, instance->particle_instance.framebuffer[(index+1)%2]);

	struct light_surface *surface = &instance->particle_instance.surface;

	glUseProgram(surface->program);
	glUniform1f(instance->particle_instance.program_deltatime_location, deltatime);

	glBindVertexArray(surface->vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surface->element_buffer);
	glDrawElements(GL_TRIANGLE_STRIP, surface->draw_count, GL_UNSIGNED_INT, 0);	
	glBindVertexArray(0);

	instance->particle_instance.buffer_index = (instance->particle_instance.buffer_index+1)%2; 

}


