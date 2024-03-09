
const char particle_system_vertex_shader_source[] = 
{
	"#version 330 core 					\n"
	" layout ( location = 0 ) in vec3 r_position; 		\n"
	" out vec3 v_position; 					\n"
	" uniform scene{ 					\n"
	"	mat4 view; 					\n"
	" };							\n"
	"uniform mat4 u_translation;				\n"
	" void main(){						\n"
	"	vec4 proj = view * u_translation * vec4(r_position, 1.0);	\n"
	"	v_position = proj.xyz;				\n"
	" 	gl_Position = proj; 				\n"
	"} 							\n"
};


const char particle_system_fragment_shader_source[] = 
{
	"#version 330 core 				\n"
	"in vec3 v_position;				\n"
	"layout(location=0) out vec3 normal_texture;	\n"
	"layout(location=1) out vec3 position_texture;	\n"
	"layout(location=2) out vec3 color_texture;	\n"
	"void main(){					\n"
	"	color_texture = vec3(1.0, 0.0, 0.0);	\n"
	"	normal_texture = vec3(1.0, 0.0, 0.0);	\n"
	"	position_texture = vec3(1.0, 0.0, 0.0); \n"
	"}						\n"
};

struct light_scene_particle_system
{
	GLint program;

	GLint translation_uniform_location; 	
};

struct light_scene_particle_system_instance
{
	/* Number of particles */
	uint32_t particle_count;

	/* Particles */
	struct light_physic_particle *particles;

	/* Intersection time */
	struct light_physic_intersect *intersect;

	/* intergration time */
	float *time;	

	/* current alive time left*/
	float *lifetime; 

	struct mat4x4 translation;	

	GLuint vertex_array;
	GLuint particle_buffer;

};


int light_scene_particle_system_init(struct light_scene_particle_system *system, GLint program)
{
	const GLchar *uniform_name = "u_translation";
	GLint translation_uniform_location = glGetUniformLocation(program, uniform_name);
	if(translation_uniform_location < 0)
	{
		printf("glGetUniformLocation(): Failed. Could not find %s \n", uniform_name);
		return -1;	
	}
	
	system->translation_uniform_location = translation_uniform_location; 
	system->program = program;

	return 0;
}

int light_scene_particle_system_deinit(struct light_scene_particle_system *system)
{
	glDeleteProgram(system->program);

	return 0;
}

void light_scene_particle_system_render(struct light_scene_particle_system *system, struct light_scene_particle_system_instance *instance, uint32_t  instance_count)
{
	glUseProgram(system->program);

	for(uint32_t i = 0; i < instance_count; i++)
	{
		glUniformMatrix4fv(system->translation_uniform_location, 1, GL_FALSE, &instance[i].translation);
		glBindVertexArray(instance[i].vertex_array);
		glDrawArrays(GL_POINTS, 0, instance[i].particle_count);
	}
}


int light_scene_particle_system_instance_init(struct light_scene_particle_system *system, struct light_scene_particle_system_instance *instance, uint32_t particle_count)
{
		
	struct light_physic_particle *particles = (struct light_physic_particle *)malloc(particle_count * sizeof(struct light_physic_particle));	
	if(particles == NULL){
		return -1;
	}

	struct light_physic_intersect *intersect = (struct light_physic_intersect *)malloc(particle_count * sizeof(struct light_physic_intersect));	
	if(intersect == NULL){
		free(particles);
		return -1;
	}

	float *time = (float *)malloc(particle_count * sizeof(float));
	if(time == NULL)
	{
		free(particles);
		free(intersect);

		return -1;	
	}
	
	float *lifetime = (float *)malloc(particle_count * sizeof(float));
	if(lifetime == NULL)
	{
		free(particles);
		free(intersect);
		free(time);

		return -1;	
	}



	for(uint32_t i = 0; i < particle_count; i++)
	{
		particles[i].position 	= (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f};
		particles[i].velocity	= (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f};
		particles[i].mass 	= 1.0f;

		lifetime[i] = 0.0f;
	}

	instance->particle_count = particle_count;
	instance->particles = particles;
	instance->intersect = intersect;
	instance->time = time;
	instance->lifetime = lifetime;

	glGenBuffers(1, &instance->particle_buffer);
	glGenVertexArrays(1, &instance->vertex_array);

	glBindVertexArray(instance->vertex_array);

	glBindBuffer(GL_ARRAY_BUFFER, instance->particle_buffer);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct light_physic_particle), (const GLvoid*)0);
			
	glBindVertexArray(0);



	glBindBuffer(GL_ARRAY_BUFFER, instance->particle_buffer);
	glBufferData(GL_ARRAY_BUFFER, particle_count * sizeof(struct light_physic_particle), particles, GL_STREAM_DRAW);


	return 0;
}

void light_scene_particle_system_instance_commit(struct light_scene_particle_system_instance *instance)
{
	glBindBuffer(GL_ARRAY_BUFFER, instance->particle_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, instance->particle_count * sizeof(struct light_physic_particle), instance->particles);
}


void light_scene_particle_system_instance_deinit(struct light_scene_particle_system_instance *instance)
{
	free(instance->particles);
	free(instance->intersect);
	free(instance->time);
	free(instance->lifetime);

	glDeleteBuffers(1, &instance->particle_buffer);
	glDeleteVertexArrays(1, &instance->vertex_array);

}


