#include "scene/implicit.h"

#include <dirent.h>

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
		/* Load files in the data directory that starts with 
		 * implicit_ and fuse the different functions in the 
		 * files using a switch statement
		 */

		/*
		 * The file with the implicit function looks like
		 * implicit_name.txt so extract the filename 
		 * name and store the file content in a buffer.
		 * */
	
		const char file_path[] = "../data/";
		uint32_t implicit_function_index = 0;

		/* Make name same size as d_name field.*/
		char implicit_function_name[LIGHT_IMPLICIT_FUNCTION_MAX_COUNT][255];
		memset(implicit_function_name, 0, sizeof(implicit_function_name));

		/* File contents */
		char implicit_function[LIGHT_IMPLICIT_FUNCTION_MAX_COUNT][LIGHT_SHADER_FILE_MAX_SIZE];
		uint32_t implicit_function_length[LIGHT_IMPLICIT_FUNCTION_MAX_COUNT];
		 

		struct dirent *dir = NULL;
		DIR *d = opendir(file_path);
		if(d != NULL)
		{
			while((dir = readdir(d)) != NULL)
			{
				/* Make sure it is a file */
				if(dir->d_type != DT_REG)
				{
					continue;
				}

				/* Get start of name string */
				char *p = NULL;
				if((p = strstr(dir->d_name, "implicit_")) == NULL)
				{
					continue;
				}
				
				/* Get end of name string */
				char *pp = NULL;
			       	if((pp = strstr(dir->d_name, ".")) == NULL)
				{
					continue;	
				}
				
				/* Delta between the memory is length.
				 * Copy to buffer.  */
				uint32_t length = pp - p;
				memcpy(
					implicit_function_name[implicit_function_index],
					p, 
					length
				);
				
				/* Copy file path to buffer such that it can be read */
				char file[sizeof(dir->d_name) + sizeof(file_path)];
				memcpy(
					file,
					file_path, 
					sizeof(file_path)-1
				);

				memcpy(
					file + sizeof(file_path)-1, 
					dir->d_name,
					255	
				);


				/* Read file contents */
				size_t read = light_file_read_buffer(
					file, 
					(uint8_t *)implicit_function[implicit_function_index],
					LIGHT_SHADER_FILE_MAX_SIZE
				);

				if(read == 0){
					fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", file);
					continue;
				}
				implicit_function_length[implicit_function_index] = read;
				implicit_function_index++;
				
			}	
			closedir(d);
		}

		instance->implicit_instance.implicit_function_name_count = implicit_function_index;
		memcpy(instance->implicit_instance.implicit_function_name, implicit_function_name, sizeof(implicit_function_name));

		/* Begining, middle and end section of the function with switch statements
		 * that will merge the functions 
		 */
		const char function_start[] = 
		"struct distance_result {\n"
		"	vec4 p;		\n"
		"	float min;	\n"
		"	vec3 color;	\n"
		"};			\n"
		"distance_result distance_object(vec4 p, float t_max, uint index)\n"
		"{ 								\n"
		"	vec4 p_res = vec4(0.0);					\n"
		" 	vec3 color = vec3(1.0);					\n"
		" 	float min_t = t_max;					\n"
		"	switch(index){ 						\n";

		const char function_switch[] = 
		"		case %u:{ \n"
	       	"			p_res = vec4(p.xyz, 1); \n"
		" 			min_t = %s(p.xyz);	\n"
		" 			break;			\n"
		" 		}\n"
		" \n";

		const char function_end[] = 
		"	} \n"
		"					\n"
		" 	distance_result result;\n"
		" 	result.p = p_res;	\n"
		" 	result.min = min_t;	\n"
		" 	result.color = color;	\n"
		"	return result; 		\n"
		"} 				\n";
	
		uint32_t function_shader_offset = 0;
		char function_shader_source[LIGHT_SHADER_FILE_MAX_SIZE];
		memcpy(
			function_shader_source, 
			function_start, 
			sizeof(function_start) -1
		);
		function_shader_offset += sizeof(function_start) - 1;

		for(uint32_t i = 0; i < implicit_function_index; i++)
		{
			char function[512];
			int characters = snprintf(
				function, 
				sizeof(function), 
				function_switch,  	
				i, 
				implicit_function_name[i]
			);

			if(!(characters > 0 || characters < sizeof(function))){
				fprintf(stderr, "Formating or buffer to small. \n");
				continue;		
			}

			if(!(function_shader_offset + sizeof(function_end) + characters < 8192)){
				fprintf(stderr, "Buffer to small. \n");
				continue;
			}
			
			memcpy(
				function_shader_source + function_shader_offset, 
				function,
				characters
			);


			function_shader_offset += characters; 
		}

		memcpy(
			function_shader_source + function_shader_offset, 
			function_end, 
			sizeof(function_end)-1
		);
		function_shader_offset += sizeof(function_end)-1;




		const char *compute_shader_filename = "../data/implicit.txt";
		char compute_shader_source[LIGHT_SHADER_FILE_MAX_SIZE];
		size_t read = light_file_read_buffer(
				compute_shader_filename, 
				(uint8_t *)compute_shader_source, 
				LIGHT_SHADER_FILE_MAX_SIZE
		);

		if(read == 0){
			fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", compute_shader_filename);
			return -1;
		}

		char compute_shader_source_header[512];
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
		
		{
			const GLchar *source_fragment[implicit_function_index+3];

			source_fragment[0] = compute_shader_source_header;

			for(uint32_t i = 0; i < implicit_function_index; i++){
				source_fragment[i+1] = implicit_function[i];
			}

			source_fragment[implicit_function_index+1] = function_shader_source;
			source_fragment[implicit_function_index+2] = compute_shader_source;

			

			uint32_t length_fragment[implicit_function_index+3];

			length_fragment[0] = gen;
			for(uint32_t i = 0; i < implicit_function_index; i++){
				length_fragment[i+1] = implicit_function_length[i];
			}

			length_fragment[implicit_function_index+1] = function_shader_offset;
			length_fragment[implicit_function_index+2] = read;

		
			const GLchar *source_vertex[] = {
				light_vertex_shader_source	
			};

			instance->implicit_instance.program = light_shader_vertex_create(
					source_vertex, NULL, 1,
					source_fragment, length_fragment, implicit_function_index+3
			);

			if(instance->implicit_instance.program  == 0){
				fprintf(stderr, "light_shader_vertex_create(): failed \n");
				return -1;
			}
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
