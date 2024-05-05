#include "scene/implicit.h"

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



int light_scene_implicit_initialize(struct light_scene_state_instance  *instance, struct light_scene_state_build *build)
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
			"#define OBJECT_NODE_STACK %u \n"
			"#define OBJECT_NODE_COUNT %u \n"
			"#define OBJECT_COUNT %u \n"
			"#define SPHERE_COUNT %u \n"
			"#define BOX_COUNT %u \n"
			"#define CYLINDER_COUNT %u \n"
			"#define LIGHT_COUNT %u \n",
			build->particle_build.emitter_count,
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
			compute_shader_source_header,
			compute_shader_source,
		};

		uint32_t length_fragment[] = {
			gen, read
		};

		const GLchar *source_vertex[] = {
			light_vertex_shader_source	
		};


		int result = light_surface_initialize_vertex_fragement_source(
				&instance->implicit_instance.surface, 
				source_vertex, NULL, 1,
			       	source_fragment, length_fragment, 2
		);

		if(result < 0)
		{
			fprintf(stderr, "light_surface_initialize_vertex_fragement_source() failed. \n");
			return -1;	
		}

	}

	uint32_t sphere_buffer_size = sizeof(struct light_scene_implicit_sphere_instance) * build->implicit_build.sphere_count + 4*sizeof(uint32_t);
	GLuint sphere_buffer;
	glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, sphere_buffer);
	glBufferData(GL_UNIFORM_BUFFER,  sphere_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	uint32_t cylinder_buffer_size = sizeof(struct light_scene_implicit_cylinder_instance) * build->implicit_build.cylinder_count + 4*sizeof(uint32_t);
	GLuint cylinder_buffer;
	glGenBuffers(1, &cylinder_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, cylinder_buffer);
	glBufferData(GL_UNIFORM_BUFFER, cylinder_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	uint32_t box_buffer_size = sizeof(struct light_scene_implicit_box_instance) * build->implicit_build.box_count + 4*sizeof(uint32_t);	
	GLuint box_buffer;
	glGenBuffers(1, &box_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, box_buffer);
	glBufferData(GL_UNIFORM_BUFFER, box_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	uint32_t light_buffer_size = sizeof(struct light_scene_light_light_instance) * build->implicit_build.light_count + 4*sizeof(uint32_t);
	GLuint light_buffer;
	glGenBuffers(1, &light_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, light_buffer);
	glBufferData(GL_UNIFORM_BUFFER, light_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	/* See shader source for structure of matrix */	
	uint32_t object_buffer_size = sizeof(struct light_scene_implicit_object_instance) * build->implicit_build.object_count + 4*sizeof(uint32_t);
	GLuint object_buffer;
	glGenBuffers(1, &object_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, object_buffer);
	glBufferData(GL_UNIFORM_BUFFER, object_buffer_size, NULL, GL_STATIC_DRAW);
	

	uint32_t object_node_buffer_size = sizeof(struct light_scene_implicit_object_node) * build->implicit_build.object_node_count + 4*sizeof(uint32_t);	
	GLuint object_node_buffer;
	glGenBuffers(1, &object_node_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, object_node_buffer);
	glBufferData(GL_UNIFORM_BUFFER, object_node_buffer_size,  NULL, GL_STATIC_DRAW);
	
	instance->implicit_instance.object_buffer 		= object_buffer;
	instance->implicit_instance.object_node_buffer 	= object_node_buffer;
	instance->implicit_instance.sphere_buffer		= sphere_buffer;
	instance->implicit_instance.cylinder_buffer	= cylinder_buffer;
	instance->implicit_instance.box_buffer		= box_buffer;
	instance->implicit_instance.light_buffer		= light_buffer;

	return 0;
}

void light_scene_implicit_commit_objects(
	struct light_scene_state_instance  *instance,
	struct light_scene_state_build *build,
	struct light_scene_implicit_object_instance *object_instance, uint32_t object_instance_count, 
	struct light_scene_implicit_object_node *object_node, uint32_t object_node_count
)
{


	glBindBuffer(GL_UNIFORM_BUFFER, instance->implicit_instance.object_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_implicit_object_instance)*object_instance_count, object_instance);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_implicit_object_instance)*build->implicit_build.object_count, sizeof(uint32_t), &object_instance_count);


	glBindBuffer(GL_UNIFORM_BUFFER, instance->implicit_instance.object_node_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_implicit_object_node)*object_node_count, object_node);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_implicit_object_node)*build->implicit_build.object_node_count, sizeof(uint32_t), &object_node_count);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void light_scene_implicit_commit_sphere(
	struct light_scene_state_instance  *instance,
	struct light_scene_state_build  *build,
	struct light_scene_implicit_sphere_instance *sphere,
	uint32_t sphere_count
)
{
	glBindBuffer(GL_UNIFORM_BUFFER, instance->implicit_instance.sphere_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_implicit_sphere_instance) * sphere_count, sphere);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_implicit_sphere_instance) * build->implicit_build.sphere_count, sizeof(uint32_t),  &sphere_count);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void light_scene_implicit_commit_cylinder(
	struct light_scene_state_instance  *instance,
	struct light_scene_state_build *build,
	struct light_scene_implicit_cylinder_instance *cylinder,
	uint32_t cylinder_count
)
{
	glBindBuffer(GL_UNIFORM_BUFFER, instance->implicit_instance.cylinder_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_implicit_cylinder_instance) * cylinder_count, cylinder);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_implicit_cylinder_instance) * build->implicit_build.cylinder_count, sizeof(uint32_t), &cylinder_count);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void light_scene_implicit_commit_box(
	struct light_scene_state_instance  *instance,
	struct light_scene_state_build  *build,
	struct light_scene_implicit_box_instance *box,
	uint32_t box_count
)
{
	glBindBuffer(GL_UNIFORM_BUFFER, instance->implicit_instance.box_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_implicit_box_instance) * box_count, box);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_implicit_box_instance) * build->implicit_build.box_count, sizeof(uint32_t), &box_count);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void light_scene_implicit_commit_light(
	struct light_scene_state_instance  *instance,
	struct light_scene_state_build  *build,
	struct light_scene_light_light_instance *light,
	uint32_t light_count
)
{
	glBindBuffer(GL_UNIFORM_BUFFER, instance->implicit_instance.light_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct light_scene_light_light_instance) * light_count, light);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(struct light_scene_light_light_instance) * build->implicit_build.light_count, sizeof(uint32_t),  &light_count);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void light_scene_implicit_dispatch(
	struct light_scene_state_instance  *instance,
	uint32_t width, uint32_t height
)
{

	light_surface_render(&instance->implicit_instance.surface);

}

void light_scene_implicit_deinitialize(
	struct light_scene_state_instance  *instance
)
{
	
	glDeleteBuffers(1, &instance->implicit_instance.sphere_buffer);
	glDeleteBuffers(1, &instance->implicit_instance.cylinder_buffer);
	glDeleteBuffers(1, &instance->implicit_instance.box_buffer);
	glDeleteBuffers(1, &instance->implicit_instance.light_buffer);
	
	glDeleteBuffers(1, &instance->implicit_instance.object_buffer);
	glDeleteBuffers(1, &instance->implicit_instance.object_node_buffer);
}