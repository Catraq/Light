
const char light_game_mesh_vertex_shader_source[] = 
{
	"#version 330 core 					\n"
	" layout ( location = 0 ) in vec3 r_position; 		\n"
	" layout ( location = 1 ) in vec3 r_normal;   		\n"
	" layout ( location = 2 ) in vec3 r_color; 		\n"
	" layout ( location = 3 ) in mat4 r_model; 		\n"
	" out vec3 v_normal; 					\n"
	" out vec3 v_position; 					\n"
	" uniform scene{ 					\n"
	"	mat4 view; 					\n"
	" };							\n"
	" void main(){						\n"
	" 	mat4 mn = transpose(inverse(r_model));		\n"
	" 	vec4 n = normalize(vec4(r_normal, 0.0));	\n"
	" 	vec4 m_n = normalize(mn * n);			\n"
	" 	v_normal = m_n.xyz;				\n"
	"   	vec4 w_position = r_model * vec4(r_position , 1.0f ); \n"	
	"	v_position = w_position.xyz;			\n"
	" 	gl_Position = view  * w_position; \n"
	"} 							\n"
};


const char light_game_mesh_fragment_shader_source[] = 
{
	"#version 330 core 				\n"
	"out vec4 fcolor; 				\n"
	"in vec3  v_normal;				\n"
	"in vec3 v_position;				\n"
	"layout(location=0) out vec3 normal_texture;	\n"
	"layout(location=1) out vec3 position_texture;	\n"
	"layout(location=2) out vec3 color_texture;	\n"
	"void main(){					\n"
	"	vec3 light_dir = normalize(vec3(10.0f, 100.0f, 0.0f) - v_normal);		\n"
	"	float diffuse = max(dot(v_normal, light_dir), 0.0);		\n"
	"	color_texture = vec3(1.0, 1.0, 1.0) * (0.1 + diffuse); 			\n"
	"	normal_texture = normalize(v_normal);	\n"
	"	position_texture = v_position;		\n"
	"}						\n"
};


struct light_game_mesh
{
	struct light_vertex_instance vertex_instance;
	GLint program;

};

struct light_game_mesh_instance
{
	struct mat4x4 translation;	
};

int light_game_mesh_init(struct light_scene_instance *scene, struct light_game_mesh *mesh, struct light_vertex_buffer *vertex_buffer,  struct light_vertex_buffer_handler *handler)
{
	GLint program = light_create_program(light_game_mesh_vertex_shader_source, light_game_mesh_fragment_shader_source);
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

	light_vertex_instance_init(&mesh->vertex_instance, vertex_buffer, handler);

	const GLuint vertex_buffer_mat4x4_offset = 0;
	const char *model_transform_attribute = "r_model";
	result = light_vertex_instance_attribute(&mesh->vertex_instance, vertex_buffer_mat4x4_offset, program, model_transform_attribute, VERTEX_INSTANCE_ATTRIBUTE_MAT4X4);
	if(result < 0)
	{
		light_vertex_instance_deinit(&mesh->vertex_instance);

		printf("vertex_instance_attribute_mat4x4(): Failed. \n");
		return -1;
	}
	
	mesh->program = program;
	
	return 0;
}

void light_game_mesh_deinit(struct light_game_mesh *mesh)
{
	glDeleteProgram(mesh->program);

	light_vertex_instance_deinit(&mesh->vertex_instance);
}

void light_game_mesh_render(struct light_game_mesh *mesh, struct light_game_mesh_instance *instance, uint32_t instance_count)
{
	glUseProgram(mesh->program);
	light_vertex_instance_draw(&mesh->vertex_instance, instance_count);
}

int light_game_mesh_instance_init(struct light_game_mesh *mesh, struct light_game_mesh_instance *instance, uint32_t instance_count)
{
	light_vertex_instance_allocate(&mesh->vertex_instance, instance, sizeof(struct light_game_mesh_instance) * instance_count);
	return 0;
}	

void light_game_mesh_instance_commit(struct light_game_mesh *mesh, struct light_game_mesh_instance *instance, uint32_t instance_count)
{
	light_vertex_instance_commit(&mesh->vertex_instance, instance, sizeof(struct light_game_mesh_instance ) * instance_count);
}


