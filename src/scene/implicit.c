#include "scene/implicit.h"

#include "error.h"

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



int light_scene_implicit_initialize(struct light_scene_state_instance *instance)
{
	
	{
		const char *compute_shader_filename = "../data/implicit.txt";
		uint8_t compute_shader_source[2*8192];
		size_t read = light_file_read_buffer(compute_shader_filename, compute_shader_source, 2*8192);
		if(read == 0){
			fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", compute_shader_filename);
			return -1;
		}

		uint8_t compute_shader_source_header[512];
		uint32_t gen = snprintf(compute_shader_source_header, 512, 
			"#version 430 core \n" 
			"#define EMITTER_COUNT %u \n"
			"#define EMITTER_PARTICLE_COUNT %u \n"
			"#define OBJECT_NODE_COUNT %u \n"
			"#define LIGHT_COUNT %u \n",
			instance->build.particle_build.emitter_count,
			instance->build.particle_build.emitter_particle_count,
			instance->build.implicit_build.object_node_count,
			instance->build.implicit_build.light_count

		); 

		const GLchar *source_fragment[] =
		{
			compute_shader_source_header,
			compute_shader_source,
		};

		uint32_t length_fragment[] = {
			gen, read
		};

		const GLchar *source_vertex[] = {
			light_vertex_shader_source	
		};

		instance->implicit_instance.program = light_shader_vertex_create(
				source_vertex, NULL, 1,
			       	source_fragment, length_fragment, 2
		);

		if(instance->implicit_instance.program  == 0){
			fprintf(stderr, "light_shader_vertex_create(): failed \n");
			return -1;
		}

	}

	uint32_t light_buffer_size = sizeof(struct light_scene_light_light_instance) * instance->build.implicit_build.light_count + 4*sizeof(uint32_t);
	GLuint light_buffer;
	glGenBuffers(1, &light_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, light_buffer);
	glBufferData(GL_UNIFORM_BUFFER, light_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	
	uint32_t object_node_buffer_size = sizeof(struct light_scene_implicit_object) * instance->build.implicit_build.object_node_count + 4*sizeof(uint32_t);	
	GLuint object_node_buffer;
	glGenBuffers(1, &object_node_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, object_node_buffer);
	glBufferData(GL_UNIFORM_BUFFER, object_node_buffer_size,  NULL, GL_STATIC_DRAW);
	
	instance->implicit_instance.object_node_buffer = object_node_buffer;
	instance->implicit_instance.light_buffer = light_buffer;

	return 0;
}

void light_scene_implicit_commit_objects(
	struct light_scene_state_instance *instance,
	struct light_scene_implicit_object *object_node,
       	uint32_t object_node_count
)
{
	
	glBindBuffer(
		GL_UNIFORM_BUFFER,
		instance->implicit_instance.object_node_buffer
	);

	glBufferSubData(
		GL_UNIFORM_BUFFER, 
		0,
		sizeof(struct light_scene_implicit_object)*object_node_count, 
		object_node
	);

	glBufferSubData(
		GL_UNIFORM_BUFFER, 
		sizeof(struct light_scene_implicit_object)*instance->build.implicit_build.object_node_count, 
		sizeof(uint32_t), 
		&object_node_count
	);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	CHECK_GL_ERROR();
}

void light_scene_implicit_commit_light(
	struct light_scene_state_instance  *instance,
	struct light_scene_light_light_instance *light,
	uint32_t light_count
)
{
	glBindBuffer(GL_UNIFORM_BUFFER, instance->implicit_instance.light_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_light_light_instance) * light_count, light);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_light_light_instance) * instance->build.implicit_build.light_count, sizeof(uint32_t),  &light_count);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void light_scene_implicit_dispatch(
	struct light_scene_state_instance  *instance,
	uint32_t width, uint32_t height
)
{
	glUseProgram(instance->implicit_instance.program);
	light_surface_render(&instance->surface);

}

void light_scene_implicit_deinitialize(
	struct light_scene_state_instance  *instance
)
{
	glDeleteProgram(instance->implicit_instance.program);
	glDeleteBuffers(1, &instance->implicit_instance.light_buffer);
	glDeleteBuffers(1, &instance->implicit_instance.object_node_buffer);
}
