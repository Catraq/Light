#include "scene/implicit.h"

#include <dirent.h>

#include "error.h"
#include "config.h"

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

	static int 
light_scene_implicit_initialize_inerita(
		struct light_scene_state_instance *instance,
		char *implicit_function_ptr[],
		uint32_t *implicit_function_length, 
		char *implicit_function_name_ptr[], 
		uint32_t implicit_function_count
		)
{
	
	/* Used for calcutaimg the volume of the implicit shape 
	 * using samples_inside/total_samples * sampling_volume 
	 */
	const char *volume_shader_filename = "../data/volume.cs";
	char volume_shader_source[LIGHT_SHADER_FILE_MAX_SIZE];
	size_t volume_bytes_read = light_file_read_buffer(
			volume_shader_filename, 
			(uint8_t *)volume_shader_source, 
			LIGHT_SHADER_FILE_MAX_SIZE
			);
	

	if(volume_bytes_read == 0){
		fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", volume_shader_filename);
		return -1;
	}


	/* Used for calculating the inerita of the implicit shape */
	const char *inertia_shader_filename = "../data/inertia.txt";
	char inertia_shader_source[LIGHT_SHADER_FILE_MAX_SIZE];
	size_t bytes_read = light_file_read_buffer(
			inertia_shader_filename, 
			(uint8_t *)inertia_shader_source, 
			LIGHT_SHADER_FILE_MAX_SIZE
			);

	if(bytes_read == 0){
		fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", inertia_shader_filename);
		return -1;
	}
	
	char shader_header[512];
	uint32_t shader_header_length = snprintf(shader_header, 512, 
			"#version 430 core \n" 
			"#define OBJECT_NODE_COUNT %u \n",
			instance->build.implicit_build.object_node_count
			); 

	/* Combine the implcit function into a single function that determines 
	 * if a point is inside or outside 
	 */
	const char function_inside_first[] = 
		"uint function_inside(uint index, vec3 p)		\n"
		"{							\n"
		"	switch(index){					\n";

	const char function_inside_last[] = 
		"	}				\n"
		"	return 0;			\n"
		"}					\n";

	const char function_inside_switch[] = 
		"		case %u: return %s(p) > 0 ? 0 : 1;	\n"; 

	uint32_t function_inside_function_offset = 0;
	char function_inside_function[LIGHT_SHADER_FILE_MAX_SIZE];

	if(!(function_inside_function_offset < sizeof(function_inside_first)-1)){
		printf("function_inside_function buffer to small. Increase and recompile.\n");
		return -1;	
	}
	memcpy(
			function_inside_function, 
			function_inside_first, 
			sizeof(function_inside_first) -1
	      );

	function_inside_function_offset += sizeof(function_inside_first)-1;

	for(uint32_t i = 0; i < implicit_function_count; i++){
		char function[512];
		int characters = snprintf(
				function,
				sizeof(function),
				function_inside_switch, 
				i,
				implicit_function_name_ptr[i]
				);

		if(!(characters > 0 || characters < sizeof(function))){
			fprintf(stderr, "Formating or buffer to small. Increase and recompile. \n");
			return -1;	
		}

		if(!(function_inside_function_offset + characters < sizeof(function_inside_function))){
			fprintf(stderr, "Buffer to small. \n");
			return -1;	
		}

		memcpy(
				function_inside_function + function_inside_function_offset, 
				function,
				characters	
		      );

		function_inside_function_offset += characters;

	}

	if(!(function_inside_function_offset + sizeof(function_inside_last) < sizeof(function_inside_function))){
		fprintf(stderr, "Buffer to small. \n");
		return -1;	
	}

	memcpy(
			function_inside_function + function_inside_function_offset, 
			function_inside_last,
			sizeof(function_inside_last) - 1
	      );

	function_inside_function_offset += sizeof(function_inside_last) - 1;

	/* Fill opengl shader source buffers such that the shader 
	 * can be compiled 
	 * */
	const GLchar *compute_source[implicit_function_count + 3];
	uint32_t compute_source_length[implicit_function_count + 3];

	compute_source[0] = shader_header;
	compute_source_length[0] = shader_header_length;

	for(uint32_t i = 0; i < implicit_function_count; i++){
		compute_source[i + 1] = implicit_function_ptr[i];
		compute_source_length[i + 1] = implicit_function_length[i];
	}

	compute_source[implicit_function_count + 1] = function_inside_function;
	compute_source_length[implicit_function_count + 1] = function_inside_function_offset;
	
	/* Set volume main shader */
	compute_source[implicit_function_count + 2] = volume_shader_source;
	compute_source_length[implicit_function_count + 2] = volume_bytes_read;


	instance->implicit_instance.volume_program = light_shader_compute_create(
			compute_source, compute_source_length, implicit_function_count + 3
			);
	
	/* Dipslay the shader in the case the compilation fails */	
	if(instance->implicit_instance.volume_program  == 0)
	{
		uint32_t line_number = 1;
		for(uint32_t i = 0; i < implicit_function_count+3; i++)
		{
			for(uint32_t j = 0; j < compute_source_length[i]; j++)
			{
				char c = compute_source[i][j];
				switch(c)
				{
					case '\n': printf("\n %u:", line_number);
						   line_number++;
						   break;
					default: 
						   printf("%c", c);
				}
			}	
		}

		return -1;
	}
	
	/* UNiform attributes for the volume shader */
	instance->implicit_instance.volume_program_samples_uniform = glGetUniformLocation(
		instance->implicit_instance.volume_program,
		"samples"
		);

	if(instance->implicit_instance.volume_program_samples_uniform == -1){
		glDeleteProgram(instance->implicit_instance.volume_program);
		fprintf(stderr, "Could not find samples uniform in shader for volume computation. \n");

		return -1;	
	}

	instance->implicit_instance.volume_program_node_index_uniform = glGetUniformLocation(
		instance->implicit_instance.volume_program,
		"node_index"
		);

	if(instance->implicit_instance.volume_program_node_index_uniform == -1){

		glDeleteProgram(instance->implicit_instance.volume_program);

		fprintf(stderr, "Could not find node_index uniform in shader for volume computation. \n");

		return -1;	
	}

	/* Change main shader to inertia computation */
	compute_source[implicit_function_count + 2] = inertia_shader_source;
	compute_source_length[implicit_function_count + 2] = bytes_read;


	instance->implicit_instance.inertia_program = light_shader_compute_create(
			compute_source, compute_source_length, implicit_function_count + 3
			);
	
	/* Dipslay the shader in the case the compilation fails */	
	if(instance->implicit_instance.inertia_program  == 0)
	{
		uint32_t line_number = 1;
		for(uint32_t i = 0; i < implicit_function_count+3; i++)
		{
			for(uint32_t j = 0; j < compute_source_length[i]; j++)
			{
				char c = compute_source[i][j];
				switch(c)
				{
					case '\n': printf("\n %u:", line_number);
						   line_number++;
						   break;
					default: 
						   printf("%c", c);
				}
			}	
		}

		glDeleteProgram(instance->implicit_instance.volume_program);

		return -1;
	}
	
	/* UNiform attributes for the volume shader */
	instance->implicit_instance.inertia_program_samples_uniform = glGetUniformLocation(
		instance->implicit_instance.inertia_program,
		"samples"
		);

	if(instance->implicit_instance.inertia_program_samples_uniform == -1){
		glDeleteProgram(instance->implicit_instance.volume_program);
		glDeleteProgram(instance->implicit_instance.inertia_program);

		fprintf(stderr, "Could not find samples uniform in shader for inerita computation. \n");

		return -1;	
	}

	instance->implicit_instance.inertia_program_node_index_uniform = glGetUniformLocation(
		instance->implicit_instance.inertia_program,
		"node_index"
		);

	if(instance->implicit_instance.inertia_program_node_index_uniform == -1){

		glDeleteProgram(instance->implicit_instance.volume_program);
		glDeleteProgram(instance->implicit_instance.inertia_program);

		fprintf(stderr, "Could not find node_index uniform in shader for inertia computation. \n");

		return -1;	
	}



	return 0;
}

void 
light_scene_implicit_compute_inerita(
		struct light_scene_state_instance *instance,
		struct light_scene_implicit_object *objects, 
		uint32_t object_count,
		struct mat3x3 *object_inerita_without_mass, 
		uint32_t samples

)
{
	uint32_t volume_buffer_size = sizeof(GLfloat) * object_count;

	GLuint volume_dummy[object_count];
	memset(volume_dummy, 0, sizeof(volume_dummy));

	GLuint volume_buffer;
	glGenBuffers(1, &volume_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, volume_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, volume_buffer_size, volume_dummy, GL_STATIC_DRAW);

	uint32_t object_buffer_size = sizeof(struct light_scene_implicit_object) * object_count;
	GLuint object_buffer;
	glGenBuffers(1, &object_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, object_buffer);
	glBufferData(GL_UNIFORM_BUFFER, object_buffer_size, objects, GL_STATIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, volume_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, object_buffer);
	
	/*Compute volumes */	
	glUseProgram(instance->implicit_instance.volume_program);
	glUniform1f(instance->implicit_instance.volume_program_samples_uniform, (float)samples);

	for(uint32_t i = 0; i < object_count; i++)
	{
		glUniform1ui(instance->implicit_instance.volume_program_node_index_uniform, i);
		glDispatchCompute(samples, samples, samples);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	float sample_volume = 10*10*10;
	GLuint volume_samples[object_count];

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, volume_buffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof volume_samples, volume_samples);
	
	float volumes[object_count];
	for(uint32_t i = 0; i < object_count; i++)
	{
		volumes[i] = volume_samples[i] * sample_volume/(float)(samples*samples*samples);	
	}
	
	/* Compute inerita */
	uint32_t inerita_samples[object_count][16];
	memset(inerita_samples, 0, sizeof(inerita_samples));

	GLuint inertia_buffer;
	glGenBuffers(1, &inertia_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, inertia_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(inerita_samples), inerita_samples, GL_STATIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inertia_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, object_buffer);

	glUseProgram(instance->implicit_instance.inertia_program);
	glUniform1f(instance->implicit_instance.inertia_program_samples_uniform, (float)samples);

	for(uint32_t i = 0; i < object_count; i++)
	{
		glUniform1ui(instance->implicit_instance.inertia_program_node_index_uniform, i);
		glDispatchCompute(samples, samples, samples);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(inerita_samples), inerita_samples);
	
	for(uint32_t i = 0; i < object_count; i++)
	{
		for(uint32_t j = 0; j < 9; j++)
		{
			float s = samples;
			object_inerita_without_mass[i].m[j] = (float)(inerita_samples[i][j])*sample_volume/(float)(1000*s*s*s*volumes[i]); 
		}	
	}



	glDeleteBuffers(1, &inertia_buffer);
	glDeleteBuffers(1, &object_buffer);
	glDeleteBuffers(1, &volume_buffer);

	return 0;
}


	static int 
light_scene_implicit_initialize_render(
		struct light_scene_state_instance *instance,
		char *implicit_function_ptr[],
		uint32_t *implicit_function_length, 
		char *implicit_function_name_ptr[], 
		uint32_t implicit_function_count
		)
{

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

	/* It is -1 on the sizeof operator as the null terminating 
	 * characters is excluded */
	uint32_t function_shader_offset = 0;
	char function_shader_source[LIGHT_SHADER_FILE_MAX_SIZE];
	memcpy(
			function_shader_source, 
			function_start, 
			sizeof(function_start) -1
	      );
	function_shader_offset += sizeof(function_start) - 1;

	for(uint32_t i = 0; i < implicit_function_count; i++)
	{
		char function[512];
		int characters = snprintf(
				function, 
				sizeof(function), 
				function_switch,  	
				i, 
				implicit_function_name_ptr[i]
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
		const GLchar *source_fragment[implicit_function_count+3];

		source_fragment[0] = compute_shader_source_header;

		for(uint32_t i = 0; i < implicit_function_count; i++){
			source_fragment[i+1] = implicit_function_ptr[i];
		}

		source_fragment[implicit_function_count+1] = function_shader_source;
		source_fragment[implicit_function_count+2] = compute_shader_source;



		uint32_t length_fragment[implicit_function_count+3];

		length_fragment[0] = gen;
		for(uint32_t i = 0; i < implicit_function_count; i++){
			length_fragment[i+1] = implicit_function_length[i];
		}

		length_fragment[implicit_function_count+1] = function_shader_offset;
		length_fragment[implicit_function_count+2] = read;


		const GLchar *source_vertex[] = {
			light_vertex_shader_source	
		};

		instance->implicit_instance.render_program = light_shader_vertex_create(
				source_vertex, NULL, 1,
				source_fragment, length_fragment, implicit_function_count+3
				);


		if(instance->implicit_instance.render_program  == 0){


			uint32_t line_number = 1;
			for(uint32_t i = 0; i < implicit_function_count+3; i++)
			{
				for(uint32_t j = 0; j < length_fragment[i]; j++)
				{
					char c = source_fragment[i][j];
					switch(c)
					{
						case '\n': printf("\n %u:", line_number);
							   line_number++;
							   break;
						default: 
							   printf("%c", c);
					}
				}	
			}

			fprintf(stderr, "light_shader_vertex_create(): failed \n");
			return -1;
		}
	}

	return 0;

}

	static int
light_scene_implicit_initialize_physic(
		struct light_scene_state_instance *instance,
		char *implicit_function_ptr[],
		uint32_t *implicit_function_length, 
		char *implicit_function_name_ptr[], 
		uint32_t implicit_function_count
		)
{
	const char *shader_filename = "../data/physic.txt";
	char shader_source[LIGHT_SHADER_FILE_MAX_SIZE];
	size_t shader_length = light_file_read_buffer(
			shader_filename, 
			(uint8_t *)shader_source, 
			LIGHT_SHADER_FILE_MAX_SIZE
			);
	if(shader_length == 0){
		fprintf(stderr, "light_file_read_buffer() failed. Could not read %s \n", shader_filename);
		return -1;	
	}

	char shader_source_header[2048];
	uint32_t shader_header_lenght = snprintf(shader_source_header, 2048, 
			"#version 430 core 						\n" 
			"layout (local_size_x=1, local_size_y=1, local_size_z=1) in;	\n"
			"#define OBJECT_NODE_COUNT %u 					\n"
			"struct object_node{						\n"
			"	uint object_index;					\n"
			"	uint padding[3];					\n"
			"	mat4 translation;					\n"
			"	mat4 translation_inv;					\n"
			"};								\n"
			"	\n"
			"	\n"
			"struct collision_pair{						\n"
			"	vec3 position;						\n"
			"	float dummy_1;						\n"
			"	uint a;							\n"
			"	uint b;							\n"
			"	uint dummy_2[2];					\n"
			"};								\n"
			"	\n"
			"layout(binding=5) uniform nodes_ubo{	\n"
			"	object_node nodes[OBJECT_NODE_COUNT];			\n"
			"	uint nodes_count;					\n"
			"	uint nodes_ubo_dummy[3];				\n"
			"};	\n"
			"	\n"
			"	\n",
		instance->build.implicit_build.object_node_count
			); 



	const char shader_source_distance_function[] = 
		"float distance_%s_%s(vec3 p, uint a, uint b)	\n"
		"{						\n"
		"	return %s((nodes[a].translation_inv * vec4(p, 1.0)).xyz) + %s((nodes[b].translation_inv * vec4(p, 1.0)).xyz);\n"
		""
		"}		\n";

	const char shader_source_gradient_function[] = 
		"vec3 grad_%s_%s(vec3 p, uint a, uint b)	\n"
		"{						\n"
		"	vec2 step = vec2(0.000001, 0);		\n"
		"	return vec3(				\n"
		"		1/(2*step.x)*(distance_%s_%s(p+step.xyy, a, b) - distance_%s_%s(p-step.xyy, a, b)), \n"
		"		1/(2*step.x)*(distance_%s_%s(p+step.yxy, a, b) - distance_%s_%s(p-step.yxy, a, b)), \n"
		"		1/(2*step.x)*(distance_%s_%s(p+step.yyx, a, b) - distance_%s_%s(p-step.yyx, a, b))); \n"
		"}		\n";

	uint32_t shader_source_implicit_function_offset = 0;
	char shader_source_implicit_functions[LIGHT_IMPLICIT_PHYSIC_FUNCTION_SIZE];

	for(uint32_t i = 0; i < implicit_function_count; i++)
	{
		for(uint32_t j = 0; j < implicit_function_count; j++)
		{

			{	
				char distance_function[2048];	
				int characters = snprintf(
						distance_function, 
						sizeof(distance_function),
						shader_source_distance_function,
						implicit_function_name_ptr[i],
						implicit_function_name_ptr[j],
						implicit_function_name_ptr[i],
						implicit_function_name_ptr[j]
						);

				if(!(characters > 0 || characters < sizeof(distance_function))){
					fprintf(stderr, "Formating buffer to small. \n");
					return -1;	
				}

				if(!(shader_source_implicit_function_offset + characters < LIGHT_IMPLICIT_PHYSIC_FUNCTION_SIZE)){
					fprintf(stderr,"Target buffer to small. \n" );
					return -1;
				}


				memcpy(
						shader_source_implicit_functions + shader_source_implicit_function_offset,
						distance_function,
						characters
				      );

				shader_source_implicit_function_offset += characters;
			}
			{
				char grad_function[2048];
				int characters = snprintf(
						grad_function, 
						sizeof(grad_function),
						shader_source_gradient_function,
						implicit_function_name_ptr[i],
						implicit_function_name_ptr[j],

						implicit_function_name_ptr[i],
						implicit_function_name_ptr[j],

						implicit_function_name_ptr[i],
						implicit_function_name_ptr[j],

						implicit_function_name_ptr[i],
						implicit_function_name_ptr[j],

						implicit_function_name_ptr[i],
						implicit_function_name_ptr[j],

						implicit_function_name_ptr[i],
						implicit_function_name_ptr[j],

						implicit_function_name_ptr[i],
						implicit_function_name_ptr[j]
							);

						if(!(characters > 0 || characters < sizeof(grad_function))){
							fprintf(stderr, "Formating buffer to small. \n");
							return -1;	
						}

						if(!(shader_source_implicit_function_offset + characters < LIGHT_IMPLICIT_PHYSIC_FUNCTION_SIZE)){
							fprintf(stderr,"Target buffer to small. \n" );
							return -1;
						}

						memcpy(
								shader_source_implicit_functions + shader_source_implicit_function_offset,
								grad_function,
								characters
						      );

						shader_source_implicit_function_offset += characters;
			}
		}
	}

	const char shader_source_intersect_start[] = 
		"struct intersection{			\n"
		"	vec3 position;			\n"
		"	uint intersection; 		\n"
		"};					\n"
		"intersection intersect(uint a, uint b)	\n"
		"{				\n";

	const char shader_source_intersect_end[] = 
		"	intersection t;	\n"
		"	t.intersection = 0;	\n"
		"}	\n";

	const char if_str[] = "if";
	const char else_if_str[] = "else if";
	const char shader_source_intersect_if_first[] = 
		"%s(nodes[a].object_index==%u && nodes[b].object_index==%u)	\n"
		"{								\n"
		"	vec3 minimal = vec3(0);					\n"
		"	for(uint i = 0; i < 32; i++){				\n"
		"		vec3 grad = grad_%s_%s(minimal, a, b);		\n"
		"		minimal = minimal - grad;			\n"
		"		if(distance_%s_%s(minimal, a, b) < 0.0)		\n"
		"		{						\n"
		"			intersection t;				\n"
		"			t.position = minimal;			\n"
		"			t.intersection = 1;			\n"
		"			return t;				\n"
		"		}						\n"
		"	}							\n"
		"}								\n";


	if(!(shader_source_implicit_function_offset + sizeof(shader_source_intersect_start) - 1 < LIGHT_IMPLICIT_PHYSIC_FUNCTION_SIZE)){
		fprintf(stderr,"Target buffer to small. \n" );
		return -1;
	}

	memcpy(
			shader_source_implicit_functions + shader_source_implicit_function_offset,
			shader_source_intersect_start,
			sizeof(shader_source_intersect_start) - 1 
	      );

	shader_source_implicit_function_offset += sizeof(shader_source_intersect_start) - 1; 

	for(uint32_t i = 0; i < implicit_function_count; i++)
	{
		for(uint32_t j = 0; j < implicit_function_count; j++)
		{
			char if_function[512];
			int characters = snprintf(
					if_function, 
					sizeof(if_function),
					shader_source_intersect_if_first, 
					(i == 0) && (j == 0) ? if_str : else_if_str,
					i,j,
					implicit_function_name_ptr[i],
					implicit_function_name_ptr[j], 
					implicit_function_name_ptr[i],
					implicit_function_name_ptr[j]
					);

			if(!(characters > 0 || characters < sizeof(if_function))){
				fprintf(stderr, "Formating buffer to small. \n");
				return -1;	
			}

			if(!(shader_source_implicit_function_offset + characters < LIGHT_IMPLICIT_PHYSIC_FUNCTION_SIZE)){
				fprintf(stderr,"Target buffer to small. \n" );
				return -1;
			}

			memcpy(
					shader_source_implicit_functions + shader_source_implicit_function_offset,
					if_function,
					characters
			      );

			shader_source_implicit_function_offset += characters;

		}
	}

	if(!(shader_source_implicit_function_offset + sizeof(shader_source_intersect_end) - 1 < LIGHT_IMPLICIT_PHYSIC_FUNCTION_SIZE)){
		fprintf(stderr,"Target buffer to small. \n" );
		return -1;
	}

	memcpy(
			shader_source_implicit_functions + shader_source_implicit_function_offset,
			shader_source_intersect_end,
			sizeof(shader_source_intersect_end) -1 
	      );
	shader_source_implicit_function_offset += sizeof(shader_source_intersect_end) - 1;


	const GLchar *compute_source[implicit_function_count + 3];
	uint32_t compute_source_length[implicit_function_count + 3];

	compute_source[0] = shader_source_header;
	compute_source_length[0] = shader_header_lenght;


	for(uint32_t i = 0; i < implicit_function_count; i++){
		compute_source[i+1] = implicit_function_ptr[i];
	}

	for(uint32_t i = 0; i < implicit_function_count; i++){
		compute_source_length[i+1] = implicit_function_length[i];
	}

	compute_source[implicit_function_count + 1] = shader_source_implicit_functions;
	compute_source_length[implicit_function_count + 1] = shader_source_implicit_function_offset;

	compute_source[implicit_function_count + 2] = shader_source;
	compute_source_length[implicit_function_count + 2] = shader_length;




	instance->implicit_instance.physic_program = light_shader_compute_create(
			compute_source, compute_source_length, implicit_function_count + 3
			);

	if(instance->implicit_instance.physic_program  == 0){

		uint32_t line_number = 1;
		for(uint32_t i = 0; i < implicit_function_count + 3; i++)
		{
			for(uint32_t j = 0; j < compute_source_length[i]; j++)
			{
				char c = compute_source[i][j];
				switch(c)
				{
					case '\n': printf("\n %u:", line_number);
						   line_number++;
						   break;
					default: 
						   printf("%c", c);
				}
			}	
		}


		fprintf(stderr, "light_shader_compute_create() failed. \n");
		return -1;
	}


	return 0;

}

int light_scene_implicit_initialize(struct light_scene_state_instance *instance)
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

	char *implicit_function_ptr[LIGHT_IMPLICIT_FUNCTION_MAX_COUNT];
	char *implicit_function_name_ptr[LIGHT_IMPLICIT_FUNCTION_MAX_COUNT];
	for(uint32_t i = 0; i < LIGHT_IMPLICIT_FUNCTION_MAX_COUNT; i++)
	{
		implicit_function_ptr[i] = implicit_function[i];
		implicit_function_name_ptr[i] = implicit_function_name[i];
	}


	struct dirent *dir = NULL;
	DIR *d = opendir(file_path);
	if(d != NULL)
	{
		while((dir = readdir(d)) != NULL && implicit_function_index < LIGHT_IMPLICIT_FUNCTION_MAX_COUNT)
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

	/* Copy count and function names to struct */
	instance->implicit_instance.implicit_function_name_count = implicit_function_index;
	memcpy(instance->implicit_instance.implicit_function_name, implicit_function_name, sizeof(implicit_function_name));



	int result = light_scene_implicit_initialize_render(
			instance,
			implicit_function_ptr,
			implicit_function_length,
			implicit_function_name_ptr,
			implicit_function_index
			);
	if(result < 0){
		fprintf(stderr, "light_scene_implicit_initialize_render() failed. \n");
		return -1;	
	}

	result = light_scene_implicit_initialize_physic(
			instance,
			implicit_function_ptr,
			implicit_function_length,
			implicit_function_name_ptr,
			implicit_function_index
			);

	if(result < 0){
		//Cleanup render 
		glDeleteProgram(instance->implicit_instance.render_program); 
		fprintf(stderr, "light_scene_implicit_initialize_physic() failed. \n");
		return -1;
	}


	result = light_scene_implicit_initialize_inerita(
		instance,
		implicit_function_ptr,
		implicit_function_length, 
		implicit_function_name_ptr, 
		implicit_function_index
	);

	if(result < 0){
		glDeleteProgram(instance->implicit_instance.render_program); 
		glDeleteProgram(instance->implicit_instance.physic_program); 
		fprintf(stderr, "light_scene_implicit_initialize_inerita() failed. \n");
		return -1;

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

	uint32_t node_count = instance->build.implicit_build.object_node_count;
	uint32_t pair_count = node_count*(node_count - 1)/2;

	uint32_t collision_pair_buffer_size = sizeof(struct light_scene_implicit_collision) * pair_count;

	GLuint collision_pair_buffer;
	glGenBuffers(1, &collision_pair_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, collision_pair_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, collision_pair_buffer_size,  NULL, GL_STATIC_DRAW);

	GLuint counter_buffer;
	glGenBuffers(1, &counter_buffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counter_buffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);




	instance->implicit_instance.collision_pair_buffer = collision_pair_buffer;
	instance->implicit_instance.collision_pair_counter_buffer = counter_buffer;
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
	if(instance->build.implicit_build.object_node_count < object_node_count){
		fprintf(stderr, "Too many object i buffer. \n");
		return;	
	}

	instance->implicit_instance.object_node_count = object_node_count;

	CHECK_GL_ERROR();
	glBindBuffer(
			GL_UNIFORM_BUFFER,
			instance->implicit_instance.object_node_buffer
		    );

	CHECK_GL_ERROR();
	glBufferSubData(
			GL_UNIFORM_BUFFER, 
			0,
			sizeof(struct light_scene_implicit_object)*object_node_count, 
			object_node
		       );

	CHECK_GL_ERROR();
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

void light_scene_implicit_dispatch_render(
		struct light_scene_state_instance  *instance,
		uint32_t width, uint32_t height
		)
{
	glUseProgram(instance->implicit_instance.render_program);
	light_surface_render(&instance->surface);
}

uint32_t light_scene_implicit_dispatch_physic(
		struct light_scene_state_instance *instance,
		struct light_scene_implicit_collision *collision,
		const uint32_t collision_count
		)
{

	GLuint intersection_count = 0;;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER,  instance->implicit_instance.collision_pair_counter_buffer);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &intersection_count) ;

	glUseProgram(instance->implicit_instance.physic_program);

	uint32_t node_count = instance->implicit_instance.object_node_count;
	uint32_t pair_count = node_count*(node_count - 1)/2;

	glDispatchCompute(pair_count, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &intersection_count);


	uint32_t read_count = intersection_count < collision_count ? intersection_count : collision_count;
	if(read_count > 0){	

		glBindBuffer(
				GL_SHADER_STORAGE_BUFFER, 
				instance->implicit_instance.collision_pair_buffer
			    );

		glGetBufferSubData(
				GL_SHADER_STORAGE_BUFFER, 
				0, 
				sizeof(struct light_scene_implicit_collision) * read_count,
				collision
				);

	}

	return read_count;
}

void light_scene_implicit_deinitialize(
		struct light_scene_state_instance  *instance
		)
{
	glDeleteProgram(instance->implicit_instance.render_program);
	glDeleteBuffers(1, &instance->implicit_instance.light_buffer);
	glDeleteBuffers(1, &instance->implicit_instance.object_node_buffer);
}
