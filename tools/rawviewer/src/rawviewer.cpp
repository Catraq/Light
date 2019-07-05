#include <platform.h>

#include <camera.h>
#include <camera_input.h>

#include <physic.h>
#include <renderer.h>

#include <math/vec.h>
#include <math/mat4x4.h>

#include <vertex_buffer.h>
#include <vertex_instance.h>

const char vertex_shader_source[] = 
{
	"#version 330 core 					\n"
	" layout ( location = 0 ) in vec3 r_position; 		\n"
	" layout ( location = 1 ) in vec3 r_normal;   		\n"
	" layout ( location = 2 ) in vec3 r_color; 		\n"
	" layout ( location = 3 ) in mat4 r_model; 		\n"
	" out vec3 normal; 					\n"
	" uniform scene{ 					\n"
	"	mat4 view; 					\n"
	" };							\n"
	" void main(){						\n"
	" 	normal = r_color;				\n"
	" 	gl_Position = view  * r_model *vec4(r_position , 1.0f );        \n"
	"} 							\n"
};

const char fragment_shader_source[] = 
{
	"#version 330 core 				\n"
	"out vec4 fcolor; 				\n"
	"in vec3  normal;				\n"
	"void main(){					\n"
	"	fcolor = vec4(vec3(1,0,0),1);		\n"
	"}								\n"
};



struct frame_info
{
	vec2  				mouse;
	uint32_t 			width;
	uint32_t 			height;
};


struct frame_info_update_result
{
	vec2 mouse_delta;
};

static struct frame_info_update_result frame_info_update(struct frame_info *info)
{
	struct frame_info_update_result result = {};
	
	struct vec2 mouse;
	platform_mouse(&mouse);
	platform_resolution(&info->width, &info->height);
	
	const float half_width_f  = (float)info->width/2.0f;
	const float half_height_f = (float)info->height/2.0f;
	
	struct vec2 tmp;
	tmp.x = -(half_width_f/info->width -  mouse.x/info->width );
	tmp.y =  (half_height_f/info->height - mouse.y/info->height );
	
	result.mouse_delta = v2sub(info->mouse, tmp); 
	result.mouse_delta = v2scl(result.mouse_delta, 2.0f); 
	info->mouse = tmp;
	
	return result;
}


#if 0
vertex_buffer_handler load_model(  vertex_buffer *buffer, const char *filename )
{
	vertex_buffer_handler model_handler; 
	FILE *fp = fopen( filename, "rb");
	if( fp ){
		
		raw_model *raw_model_data = raw_model_load( fp );
		
		if( raw_model_data )
		{
			uint32_t vertex_count = 0;
			vertex *vertices = raw_model_vertices( raw_model_data, &vertex_count);
			
			uint32_t indice_count = 0;
			uint32_t *indices = raw_model_indices( raw_model_data, &indice_count );
			
			model_handler = vertex_buffer_push( buffer, vertices, vertex_count, indices, indice_count ); 					
			raw_model_release( raw_model_data );
		}
		fclose( fp );
	}
	
	return ( model_handler );
}
#endif 

		
	
GLuint create_program(const char *vertex_source, const char *fragment_source)
{
	int result = 0;
	
	GLuint program = glCreateProgram();
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	
	
	const GLchar *vsource = (const GLchar *)vertex_source;
	const GLchar *fsource = (const GLchar *)fragment_source;

	glShaderSource(vertex_shader, 1, &vsource, 0);
	glShaderSource(fragment_shader, 1, &fsource, 0);
	
	glCompileShader(vertex_shader);
	glCompileShader(fragment_shader);
	
	
	GLint v_compiled = GL_FALSE;
	GLint f_compiled = GL_FALSE;
	
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &v_compiled);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &f_compiled);
	
	if( v_compiled == GL_FALSE 
	||  f_compiled == GL_FALSE )
	{
		
		GLchar log[255];
		GLsizei length;
		
		glGetShaderInfoLog(vertex_shader, 255, &length, (GLchar*)&log);
		if( length != 0 )
		{
			printf(" ---- Vertexshader log ---- \n %s \n", (GLchar*)&log);
		}
		
		length = 0;
		glGetShaderInfoLog(fragment_shader, 255, &length, (GLchar*)&log);
		if( length != 0)
		{
			printf(" ---- Fragmentshader log ---- \n %s \n", (GLchar*)&log);
		}
		
	}
	
	
	
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	
	glLinkProgram(program);
	
	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	
	GLint linked = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if(linked == GL_FALSE) {
		GLsizei length;
		GLchar log[255];
		glGetProgramInfoLog(program, 255, &length, (GLchar*)&log);
		printf(" ---- Link log ---- \n %s \n", (GLchar*)&log);
	}
	

	return program;
}



void view_initialize(struct camera_view_state *view_state, GLuint buffer_base_index, uint32_t width, uint32_t height)
{


	camera_initialize(view_state, buffer_base_index);

	//Camera attributes 
	const float fov = 3.14f/3.0f;
	const float far = 1000.0f;
	const float close = 1.0f;
	float ratio = ((float)width / (float)height); 

	view_state->near = close;
	view_state->far = far;
	view_state->fov = fov;
	
	//Setup camera. 
	camera_view_projection(view_state, width, height);
	view_state->position = (vec3){0.0f, 0.0f, 5.0f};
	view_state->rotation = (vec3){0.0f, 0.0f, 0.0f};

}



int load_model(struct vertex_buffer *buffer, struct vertex_buffer_handler *handler, const char *filename)
{

	int result = -1;
	FILE *fp = fopen(filename, "rb");
	if(fp)
	{
		struct raw_model *raw_model_data = raw_model_load(fp);
		if(raw_model_data)
		{
			uint32_t vertex_count = 0;
			uint32_t indice_count = 0;

			struct vertex *vertices = raw_model_vertices(raw_model_data, &vertex_count);
			uint32_t *indices = raw_model_indices(raw_model_data, &indice_count);
			
			*handler = vertex_buffer_push(buffer, vertices, vertex_count, indices, indice_count); 					
			raw_model_release(raw_model_data);
			result = 1;
		}
		fclose(fp);
	}
	return result;
}


int model_data_size(const char *model_str[], int model_str_count, uint32_t *vertex_size, uint32_t *indice_size)
{
	int result = 1;
	uint32_t indice_total_size = 0;
	uint32_t vertex_total_size = 0;

	for(int i = 0; i < model_str_count; i++){
		FILE *fp = fopen(model_str[i], "rb");
		if(fp)
		{
			uint32_t vertex_size, indice_size;
			uint32_t loaded = raw_model_size(fp, &vertex_size, &indice_size);
			if(loaded < 0){
				printf("failed to read size of model in file: %s \n", model_str[i]);
				result = -1;
			}else{
				indice_total_size += indice_size;
				vertex_total_size += vertex_size;
			}
			fclose(fp);
		}
		else{
			printf("failed to open file: %s \n", model_str[i]);
			result = -1;
		}
	}

	*vertex_size = vertex_total_size;
	*indice_size = indice_total_size;

	return result;
}

int load_vertex_buffer(struct vertex_buffer *vertex_buffer, struct vertex_buffer_handler *model,
const char *model_path[], const int model_count)
{
	int result = 0;
	uint32_t indice_total_size = 0;
       	uint32_t vertex_total_size = 0;

	result = model_data_size(model_path, model_count, &vertex_total_size, &indice_total_size);
	if(result < 0){
		return -1;	
	}


	vertex_buffer_initialize(vertex_buffer, vertex_total_size, indice_total_size);


	
	result = 1;
	for(int i = 0; i < model_count; i++){
		int loaded = load_model(vertex_buffer, &model[i], model_path[i]);
		if(!loaded){
			printf("failed to open file: %s \n", model_path[i]);
			result = -1;
		}
	}

	if(result < 0){
		return -1;	
	}

	
	return result;
}

void translation_attribute_pointer(GLuint location, GLsizei stride, GLuint offset)
{	

	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(offset));
	glVertexAttribDivisor(location, 1);
	location++;
	
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(sizeof(struct vec4) + offset));
	glVertexAttribDivisor(location, 1);
	location++;

	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(2*sizeof(struct vec4) + offset));
	glVertexAttribDivisor(location, 1);
	location++;	
	
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(3*sizeof(struct vec4) + offset));
	glVertexAttribDivisor(location, 1);

}


int main( int args, char *argv[])
{
	int result = -1;
	struct camera_view_state view_state;
	struct frame_info frame_info;
	struct vertex_buffer vertex_buffer;
	struct vertex_buffer_handler model;


	if( args != 2 )
	{
		printf("Usage: %s filename \n", argv[0] );
		return ( result );
	}
	
	const char *filename = argv[1];
	
	result = platform_initialize();
	if(result < 0)
	{
		fprintf(stderr, "Error: could not initialize platform.\n");
		exit(EXIT_FAILURE);
	}

#if 0
	//Init vertex buffer for render engine
	vertex_buffer_initialize(&buffer);
	vertex_buffer_handler model = load_model( &buffer, filename);
	vertex_buffer_commit( &buffer );
#endif 


	result = load_vertex_buffer(&vertex_buffer, &model, &filename, 1);
	if(result < 0){
		fprintf(stderr, "Error: could not load vertex buffer data. ");	
		exit(EXIT_FAILURE);
	}
	
	const GLuint block_location = 0;
	view_initialize(&view_state,  block_location, 512, 512);
#if 0	
	//Init render handle for the object
	render_instance model_render_handler;
	{
		renderer_instance_initialize(&r_renderer, &model_render_handler,&buffer, &model);
	}
#endif 
	
	GLuint program = create_program(vertex_shader_source, fragment_shader_source);
	camera_buffer_bind(&view_state, program);

	const int translation_offset = 0;	

	struct vertex_instance instance;	
	vertex_instance_initialize(&instance, &vertex_buffer, &model);

	glBindVertexArray(instance.vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, instance.instance_buffer);

	GLuint translation_index = glGetAttribLocation(program, "r_model");
	if(translation_index  < 0){
		fprintf(stderr, "Error: could not find r_model attribute location. \n");	
	}
	translation_attribute_pointer(translation_index, sizeof(struct mat4x4), 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	struct vec3 position = {0.0f, 0.0f, 10.0f};
	struct vec3 rotation = {0.0f, 0.0f, 0.0f};

	struct mat4x4 model_view;	
	model_view = m4x4trs(position);
	vertex_instance_update(instance.instance_buffer, &model_view, sizeof(struct mat4x4));
	

	while(!platform_exit())
	{
		struct frame_info_update_result frame_result = frame_info_update(&frame_info);
		camera_view_projection(&view_state, frame_info.width, frame_info.height);
		camera_view_matrix(&view_state);
		struct vec2 mouse_delta = frame_result.mouse_delta;

	
		if(platform_mouse_key(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
			vec3 delta = {mouse_delta.y, mouse_delta.x, 0.0f};
			rotation = v3add(rotation, delta);

			struct mat4x4 model_rot = m4x4rote(rotation); 
			struct mat4x4 model_pos = m4x4trs(position);
			struct mat4x4 model = m4x4mul(model_pos, model_rot);

			vertex_instance_update(instance.instance_buffer, &model, sizeof(model));
		}

		glViewport(0, 0, frame_info.width, frame_info.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
		
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		glUseProgram(program);
		vertex_instance_draw(&instance, 1);
		glBindVertexArray(0);
#if 0
	
		
		//TODO: This should be done in some other way...
		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ){
			//model_body[0].rotation = v3add(model_body[0].rotation, update_camera_rotation(frame_result.mouse_delta));
		}
		
		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ){
			//model_body[0].position.z += frame_result.mouse_delta.y*20.0f;
		}
		 
#if 0	
		physic_instance_translate_body( &model_body[0] );
		renderer_instance_dwrite_instances( &model_render_handler, model_body, 1 );
#endif 

#endif 

		
		platform_update();
	}
	
	//vertex_buffer_deinitialize( &buffer );
	platform_deinitialize();
	
	return (result);
}
