
const char light_scene_plane_vertex_shader_source[] = 
{
	"#version 330 core 					\n"
	" layout ( location = 0 ) in vec3 r_position; 		\n"
	" uniform scene{ 					\n"
	"	mat4 view; 					\n"
	" };							\n"
	" void main(){						\n"
	"   	vec4 w_position = vec4(r_position , 1.0f ); 	\n"	
	" 	gl_Position = view  * w_position; 		\n"
	"} 							\n"
};


const char light_scene_plane_fragment_shader_source[] = 
{
	"#version 330 core 				\n"
	"out vec4 fcolor; 				\n"
	"layout(location=2) out vec3 color_texture;	\n"
	"void main(){					\n"
	"	color_texture = vec3(1.0, 1.0, 1.0);	\n"
	"}						\n"
};


struct light_scene_plane
{
	GLuint program;
};

struct light_scene_plane_instance
{
	GLuint element_buffer;
	GLuint vertex_buffer;
	GLuint vertex_array;
};


const GLuint light_scene_plane_quad_indices[] = {
	1, 0, 2,
	2, 0, 3
};

const GLuint light_scene_plane_quad_indices_count = sizeof(light_scene_plane_quad_indices)/sizeof(light_scene_plane_quad_indices[0]);

int
light_scene_plane_init(struct light_scene_instance *scene, struct light_scene_plane *plane)
{
	GLint program = light_create_program(light_scene_plane_vertex_shader_source, light_scene_plane_fragment_shader_source);
	if(program < 0)
	{
		printf("light_create_program(). Failed. \n");
		return -1;
	}

	int result =  light_scene_bind(scene, program);
	if(result < 0)
	{
		printf("light_scene_bind(): Failed. \n");	
		return -1;
	}
	
	plane->program = program;

	return 0;
}

int
light_scene_plane_instance_init(struct light_scene_plane *plane, struct light_scene_plane_instance *instance)
{
	glGenBuffers(1, &instance->vertex_buffer);
	glGenVertexArrays(1, &instance->vertex_array);
	glGenBuffers(1, &instance->element_buffer);


	glBindVertexArray(instance->vertex_array);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(light_scene_plane_quad_indices), light_scene_plane_quad_indices, GL_STATIC_DRAW); 


	glBindBuffer(GL_ARRAY_BUFFER, instance->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), NULL, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);

	glBindVertexArray(0);

	return 0;
}

int
light_scene_plane_instance_commit(struct light_scene_plane_instance *instance, struct light_physic_plane_static *plane)
{
	
	struct vec3 p = plane->position;
	struct vec3 c = v3cross(plane->normal, plane->dir);

	struct vec3 p1 = v3scl(plane->dir, plane->width/2.0f);
	struct vec3 p2 = v3scl(c, plane->height/2.0f);

	struct vec3 coords[] = {
		v3sub(p, v3add(p1, p2)), 
		v3add(p, v3sub(p1, p2)), 
		v3add(p, v3add(p1, p2)), 
		v3sub(p, v3sub(p1, p2)), 
	};	


	glBindBuffer(GL_ARRAY_BUFFER, instance->vertex_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * sizeof(float), coords);

	return 0;
}

int
light_scene_plane_render(struct light_scene_plane *plane, struct light_scene_plane_instance *instance, uint32_t instance_count)
{
	glUseProgram(plane->program);
	for(uint32_t i = 0; i < instance_count; i++)
	{
		glBindVertexArray(instance[i].vertex_array);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance[i].element_buffer);
		glDrawElements(GL_TRIANGLE_STRIP, light_scene_plane_quad_indices_count, GL_UNSIGNED_INT, 0);	
	}

	return 0;
}


