#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "math/vec.h"
#include "math/mat4x4.h"
#include "platform.h"

#include "camera.h"
#include "camera_input.h"


#include "model/raw_model.h"


#include "framebuffer.h"

#include "vertex_buffer.h"
#include "vertex_buffer_model.h"
#include "vertex_instance.h"
#include "vertex_instance_attribute.c"
#include "vertex_shader_program.c"

#include "frame.c"
#include "surface.c"

#include <cblas.h>
#include <lapacke.h>

#include "physic/physic.c"
#include "physic/physic_rope.c"

#include "scene.c"
#include "scene/mesh.c"
#include "scene/plane.c"
#include "scene/particle_system.c"


const char light_game_implicit_sphere_shader_source[] = 
{
	"#version 330 core 					\n"
	" layout ( location = 0 ) in vec2 r_position; 		\n"
	" layout ( location = 1 ) in float r_radius;		\n"
	/* Translation of sphere */
	" layout ( location = 3 ) in mat4 r_model; 		\n"
	/* Size of screen */
	" out vec2 dimension;					\n"
	/* Center of sphere */
	" out float s_radius;					\n"
	" out float s_fov;					\n"
	" out mat4 s_model_view_inv;				\n"
	" uniform scene{ 					\n"
	"	mat4 view; 					\n"
	"	float fov;					\n"
	"	uint width;					\n"
	"	uint height;					\n"
	" };							\n"
	" void main(){						\n"
	"	s_fov = fov;					\n"
	"	s_model_view_inv = inverse(view * r_model);					\n"
	"	s_radius = r_radius;					\n"
	"	dimension = vec2(width, height);			\n"
	" 	gl_Position = vec4(r_position, 0.0, 1.0); 		\n"
	"} 								\n"
};



const char light_game_implicit_sphere_fragment_shader_source[] = 
{
	"#version 330 core 				\n"
	"out vec4 fcolor; 				\n"
	"in vec2 dimension;				\n"
	"in float s_radius;				\n"
	"in mat4 s_view;				\n"
	"in float s_fov;				\n"
	"in mat4 s_model_view_inv;				\n"
	"layout(location=0) out vec3 normal_texture;	\n"
	"layout(location=1) out vec3 position_texture;	\n"
	"layout(location=2) out vec3 color_texture;	\n"
	"	\n"
	"float cylinder(vec3 p, float radius){			\n"
	"	return length(p) - radius;					\n"
	"}									\n"
	"void main(){					\n"
	"	vec3 uv = vec3(2.0*gl_FragCoord.xy/dimension.xy - 1.0, 1.0);	\n"
	"	uv.x *= dimension.x/dimension.y;		\n"
	"	uv = normalize(uv);				\n"
	"	float t = 0.0;					\n"
	"	float t_max = 300.0;				\n"
	"	for(int i = 0; i < 32; i++){			\n"
	"		vec4 p = s_model_view_inv * vec4(uv * t, 1.0);			\n"
	"		float h = cylinder(p.xyz, s_radius);	\n"
	"		if(h < 0.001 || t > t_max){break;}			\n" 
	"		t += h;					\n"
	"	}						\n"
	"	if(t < t_max){					\n"
	"		color_texture = vec3(1.0, 0.0, 0.0);	\n"
	"	}						\n"
	"	else{discard;}"
	"}							\n"
};


/* 
 * Implcit sphere instance. Should be at most one instance
 * but should not make any difference if multiple is used. 
 */
struct light_game_implicit_sphere
{
	/* Full screen quad used for rendering the implcit sphere. */
	struct light_surface surface;

	/* Buffer for all instances */
	GLuint instance_buffer;
};

/*
 * Mirrored in GPU. Have to correspond with shader input. 
 * Adjust vertex attribute location if changed. 
 */
struct light_game_implicit_sphere_instance
{
	struct mat4x4 translation;

	/* TODO: radius of each sphere  */
	float radius 
};

int light_game_implicit_sphere_init(struct light_scene_instance *scene, struct light_game_implicit_sphere *implicit_sphere)
{
	int result = 0;
	
	/* Create quad with implcit sphere shader */
	struct light_surface surface;
	result = light_surface_initialize_vertex_fragement_source(&surface, light_game_implicit_sphere_shader_source, light_game_implicit_sphere_fragment_shader_source);
	if(result < 0)
	{
		fprintf(stderr, "light_surface_initialize_vertex_fragment_source(): failed. \n");
		return -1;
	}
	
	const char *attribute_name = "r_model";
	GLint translation_index = glGetAttribLocation(surface.program, attribute_name);
	if(translation_index == -1){

		light_surface_deinitialize(&surface);

		fprintf(stderr, "glGetAttribLocation(): could not find %s .\n", attribute_name);
		return -1;	
	}

	const char *attribute_radius_name = "r_radius";
	GLint radius_index = glGetAttribLocation(surface.program, attribute_radius_name);
	if(radius_index == -1){

		light_surface_deinitialize(&surface);

		fprintf(stderr, "glGetAttribLocation(): could not find %s .\n", attribute_radius_name);
		return -1;	
	}

	glGenBuffers(1, &implicit_sphere->instance_buffer);
	glBindVertexArray(surface.vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, implicit_sphere->instance_buffer);
	{
		GLuint stride = sizeof(struct light_game_implicit_sphere_instance);
		GLuint offset = 0;


		GLuint location = translation_index;
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

		glEnableVertexAttribArray(radius_index);
		glVertexAttribPointer(radius_index, 1, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(4*sizeof(struct vec4) + offset));
		glVertexAttribDivisor(radius_index, 1);


	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* Bind shader program buffers to buffers in scene */
	result =  light_scene_bind(scene, surface.program);
	if(result < 0)
	{
		light_surface_deinitialize(&surface);

		glDeleteBuffers(1, &implicit_sphere->instance_buffer);

		printf("light_scene_bind(): Failed. \n");	
		return -1;
	}


	implicit_sphere->surface = surface;

	return 0;
}


int light_game_implicit_sphere_instance_init(struct light_game_implicit_sphere *implicit_sphere, struct light_game_implicit_sphere_instance *instance, uint32_t instance_count)
{

	glBindBuffer(GL_ARRAY_BUFFER, implicit_sphere->instance_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(struct light_game_implicit_sphere_instance) * instance_count, instance, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return 0;
}	

int light_game_implicit_sphere_instance_commit(struct light_game_implicit_sphere *implicit_sphere, struct light_game_implicit_sphere_instance *instance, uint32_t instance_count)
{

	glBindBuffer(GL_ARRAY_BUFFER, implicit_sphere->instance_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(struct light_game_implicit_sphere_instance) * instance_count, instance);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return 0;
}


void light_game_implcit_sphere_render(struct light_game_implicit_sphere *implicit_sphere, struct light_game_implicit_sphere_instance *instance, uint32_t instance_count)

{
	glDisable(GL_DEPTH_TEST);
	light_surface_render_instanced(&implicit_sphere->surface, instance_count);
	glEnable(GL_DEPTH_TEST);
}


const char light_game_implicit_cylinder_shader_source[] = 
{
	"#version 330 core 					\n"
	" layout ( location = 0 ) in vec2 r_position; 		\n"
	" layout ( location = 1 ) in float r_radius;		\n"
	" layout ( location = 2 ) in float r_height;		\n"
	/* Translation of sphere */
	" layout ( location = 3 ) in mat4 r_model; 		\n"
	/* Size of screen */
	" out vec2 dimension;					\n"
	/* Center of sphere */
	" out vec3 s_center;					\n"
	" out float s_radius;					\n"
	" out float s_height;					\n"
	" out mat4 s_view;					\n"
	" out mat4 s_model_view_inv;				\n"
	" uniform scene{ 					\n"
	"	mat4 view; 					\n"
	"	float fov;					\n"
	"	uint width;					\n"
	"	uint height;					\n"
	" };							\n"
	" void main(){						\n"
	"	s_view = view;					\n"
	"	s_radius = r_radius;				\n"
	"	s_height = r_height;				\n"
	"	s_model_view_inv = inverse(view * r_model);		\n"
	"	vec4 c = vec4(r_model[3][0], r_model[3][1], r_model[3][2], 1.0);	\n"
	"	s_center = (view*c).xyz;				\n "
	"	dimension = vec2(width, height);			\n"
	" 	gl_Position = vec4(r_position, 0.0, 1.0); 		\n"
	"} 								\n"
};


const char light_game_implicit_cylinder_fragment_shader_source[] = 
{
	"#version 330 core 				\n"
	"out vec4 fcolor; 				\n"
	"in vec2 dimension;				\n"
	"in vec3 s_center;				\n"
	"in float s_radius;				\n"
	"in float s_height;				\n"
	"in mat4 s_view;				\n"
	"in mat4 s_model_view_inv;				\n"
	"layout(location=0) out vec3 normal_texture;	\n"
	"layout(location=1) out vec3 position_texture;	\n"
	"layout(location=2) out vec3 color_texture;	\n"
	"	\n"
	"float cylinder(vec3 p, float height, float radius){			\n"
	"	vec2 d = abs(vec2(length(p.xz), p.y)) - vec2(radius, height);	\n"
	"	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));		\n"
	"}									\n"
	"void main(){					\n"
	"	vec3 center = s_center;			\n"
	"	vec3 uv = vec3(2.0*gl_FragCoord.xy/dimension.xy - 1.0, 1.0);	\n"
	"	uv.x *= dimension.x/dimension.y;		\n"
	"	uv = normalize(uv);				\n"
	"	float t = 0.0;					\n"
	"	float t_max = 300.0;				\n"
	"	for(int i = 0; i < 32; i++){			\n"
	"		vec4 p = s_model_view_inv * vec4(uv * t, 1.0);			\n"
	"		float h = cylinder(p.xyz, s_height, s_radius);	\n"
	"		if(h < 0.001 || t > t_max){break;}			\n" 
	"		t += h;					\n"
	"	}						\n"
	"	if(t < t_max){					\n"
	"		color_texture = vec3(0.0, 1.0, 0.0);	\n"
	"	}						\n"
	"	else{discard;}"
	"}							\n"
};

/* 
 * Implcit cylinder instance. Should be at most one instance
 * but should not make any difference if multiple is used. 
 */
struct light_game_implicit_cylinder
{
	/* Full screen quad used for rendering the implcit cylinder. */
	struct light_surface surface;

	/* Buffer for all instances */
	GLuint instance_buffer;
};

/*
 * Mirrored in GPU. Have to correspond with shader input. 
 * Adjust vertex attribute location if changed. 
 */
struct light_game_implicit_cylinder_instance
{
	struct mat4x4 translation;

	/* Radius of each cylinder  */
	float radius; 

	/* Height of cylinder */
	float height;
};

int light_game_implicit_cylinder_init(struct light_scene_instance *scene, struct light_game_implicit_cylinder *implicit_cylinder)
{
	int result = 0;
	
	/* Create quad with implcit cylinder shader */
	struct light_surface surface;
	result = light_surface_initialize_vertex_fragement_source(&surface, light_game_implicit_cylinder_shader_source, light_game_implicit_cylinder_fragment_shader_source);
	if(result < 0)
	{
		fprintf(stderr, "light_surface_initialize_vertex_fragment_source(): failed. \n");
		return -1;
	}
	
	const char *attribute_name = "r_model";
	GLint translation_index = glGetAttribLocation(surface.program, attribute_name);
	if(translation_index == -1){

		light_surface_deinitialize(&surface);

		fprintf(stderr, "glGetAttribLocation(): could not find %s .\n", attribute_name);
		return -1;	
	}

	const char *attribute_radius_name = "r_radius";
	GLint radius_index = glGetAttribLocation(surface.program, attribute_radius_name);
	if(radius_index == -1){

		light_surface_deinitialize(&surface);

		fprintf(stderr, "glGetAttribLocation(): could not find %s .\n", attribute_radius_name);
		return -1;	
	}

	const char *attribute_height_name = "r_height";
	GLint height_index = glGetAttribLocation(surface.program, attribute_height_name);
	if(height_index == -1){

		light_surface_deinitialize(&surface);

		fprintf(stderr, "glGetAttribLocation(): could not find %s .\n", attribute_height_name);
		return -1;	
	}

	glGenBuffers(1, &implicit_cylinder->instance_buffer);
	glBindVertexArray(surface.vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, implicit_cylinder->instance_buffer);
	{
		GLuint stride = sizeof(struct light_game_implicit_cylinder_instance);
		GLuint offset = 0;


		GLuint location = translation_index;
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

		glEnableVertexAttribArray(radius_index);
		glVertexAttribPointer(radius_index, 1, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(4*sizeof(struct vec4) + offset));
		glVertexAttribDivisor(radius_index, 1);

		glEnableVertexAttribArray(height_index);
		glVertexAttribPointer(height_index, 1, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(4*sizeof(struct vec4) + sizeof(float) + offset));
		glVertexAttribDivisor(height_index, 1);



	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* Bind shader program buffers to buffers in scene */
	result =  light_scene_bind(scene, surface.program);
	if(result < 0)
	{
		light_surface_deinitialize(&surface);

		glDeleteBuffers(1, &implicit_cylinder->instance_buffer);

		printf("light_scene_bind(): Failed. \n");	
		return -1;
	}


	implicit_cylinder->surface = surface;

	return 0;
}


int light_game_implicit_cylinder_instance_init(struct light_game_implicit_cylinder *implicit_cylinder, struct light_game_implicit_cylinder_instance *instance, uint32_t instance_count)
{

	glBindBuffer(GL_ARRAY_BUFFER, implicit_cylinder->instance_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(struct light_game_implicit_cylinder_instance) * instance_count, instance, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return 0;
}	

int light_game_implicit_cylinder_instance_commit(struct light_game_implicit_cylinder *implicit_cylinder, struct light_game_implicit_cylinder_instance *instance, uint32_t instance_count)
{

	glBindBuffer(GL_ARRAY_BUFFER, implicit_cylinder->instance_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(struct light_game_implicit_cylinder_instance) * instance_count, instance);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return 0;
}


void light_game_implcit_cylinder_render(struct light_game_implicit_cylinder *implicit_cyinder, struct light_game_implicit_cylinder_instance *instance, uint32_t instance_count)
{
	glDisable(GL_DEPTH_TEST);
	light_surface_render_instanced(&implicit_cyinder->surface, instance_count);
	glEnable(GL_DEPTH_TEST);
}



const char light_game_implicit_box_shader_source[] = 
{
	"#version 330 core 					\n"
	" layout ( location = 0 ) in vec2 r_position; 		\n"
	" layout ( location = 1 ) in vec3 r_dim;		\n"
	/* Translation of sphere */
	" layout ( location = 3 ) in mat4 r_model; 		\n"
	/* Size of screen */
	" out vec2 dimension;					\n"
	/* Center of sphere */
	" out vec3 s_dim;					\n"
	" out mat4 s_view;					\n"
	" out mat4 s_model_view_inv;				\n"
	" uniform scene{ 					\n"
	"	mat4 view; 					\n"
	"	float fov;					\n"
	"	uint width;					\n"
	"	uint height;					\n"
	" };							\n"
	" void main(){						\n"
	"	s_view = view;					\n"
	"	s_dim = r_dim;					\n"
	"	s_model_view_inv = inverse(view * r_model);		\n"
	"	vec4 c = vec4(r_model[3][0], r_model[3][1], r_model[3][2], 1.0);	\n"
	"	dimension = vec2(width, height);			\n"
	" 	gl_Position = vec4(r_position, 0.0, 1.0); 		\n"
	"} 								\n"
};


const char light_game_implicit_box_fragment_shader_source[] = 
{
	"#version 330 core 				\n"
	"out vec4 fcolor; 				\n"
	"in vec2 dimension;				\n"
	"in vec3 s_dim;				\n"
	"in mat4 s_view;				\n"
	"in mat4 s_model_view_inv;				\n"
	"layout(location=0) out vec3 normal_texture;	\n"
	"layout(location=1) out vec3 position_texture;	\n"
	"layout(location=2) out vec3 color_texture;	\n"
	"						\n"
	"float box(vec3 p, vec3 d){			\n"
	"	vec3 q = abs(p) - d;				\n"
	"	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);		\n"
	"}									\n"
	"void main(){					\n"
	"	vec3 uv = vec3(2.0*gl_FragCoord.xy/dimension.xy - 1.0, 1);	\n"
	"	uv.x *= dimension.x/dimension.y;		\n"
	"	uv = normalize(uv);				\n"
	"	float t = 0.0;					\n"
	"	float t_max = 300.0;				\n"
	"	for(int i = 0; i < 32; i++){			\n"
	"		vec4 p = s_model_view_inv * vec4(uv * t, 1.0);			\n"
	"		float h = box(p.xyz, s_dim);	\n"
	"		if(h < 0.001 || t > t_max){break;}			\n" 
	"		t += h;					\n"
	"	}						\n"
	"	if(t < t_max){					\n"
	"		color_texture = vec3(0.0, 0.0, 1.0);	\n"
	"	}						\n"
	"	else{discard;}"
	"}							\n"
};

/* 
 * Implcit box instance. Should be at most one instance
 * but should not make any difference if multiple is used. 
 */
struct light_game_implicit_box
{
	/* Full screen quad used for rendering the implcit box. */
	struct light_surface surface;

	/* Buffer for all instances */
	GLuint instance_buffer;
};

/*
 * Mirrored in GPU. Have to correspond with shader input. 
 * Adjust vertex attribute location if changed. 
 */
struct light_game_implicit_box_instance
{
	struct mat4x4 translation;
	
	/* Box dimension */	
	struct vec3 dimension;
};

int light_game_implicit_box_init(struct light_scene_instance *scene, struct light_game_implicit_box *implicit_box)
{
	int result = 0;
	
	/* Create quad with implcit box shader */
	struct light_surface surface;
	result = light_surface_initialize_vertex_fragement_source(&surface, light_game_implicit_box_shader_source, light_game_implicit_box_fragment_shader_source);
	if(result < 0)
	{
		fprintf(stderr, "light_surface_initialize_vertex_fragment_source(): failed. \n");
		return -1;
	}
	
	const char *attribute_name = "r_model";
	GLint translation_index = glGetAttribLocation(surface.program, attribute_name);
	if(translation_index == -1){

		light_surface_deinitialize(&surface);

		fprintf(stderr, "glGetAttribLocation(): could not find %s .\n", attribute_name);
		return -1;	
	}

	const char *attribute_dimension_name = "r_dim";
	GLint dimension_index = glGetAttribLocation(surface.program, attribute_dimension_name);
	if(dimension_index == -1){

		light_surface_deinitialize(&surface);

		fprintf(stderr, "glGetAttribLocation(): could not find %s .\n", attribute_dimension_name);
		return -1;	
	}

	glGenBuffers(1, &implicit_box->instance_buffer);
	glBindVertexArray(surface.vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, implicit_box->instance_buffer);
	{
		GLuint stride = sizeof(struct light_game_implicit_box_instance);
		GLuint offset = 0;


		GLuint location = translation_index;
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

		glEnableVertexAttribArray(dimension_index);
		glVertexAttribPointer(dimension_index, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(4*sizeof(struct vec4) + offset));
		glVertexAttribDivisor(dimension_index, 1);


	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* Bind shader program buffers to buffers in scene */
	result =  light_scene_bind(scene, surface.program);
	if(result < 0)
	{
		light_surface_deinitialize(&surface);

		glDeleteBuffers(1, &implicit_box->instance_buffer);

		printf("light_scene_bind(): Failed. \n");	
		return -1;
	}


	implicit_box->surface = surface;

	return 0;
}


int light_game_implicit_box_instance_init(struct light_game_implicit_box *implicit_box, struct light_game_implicit_box_instance *instance, uint32_t instance_count)
{

	glBindBuffer(GL_ARRAY_BUFFER, implicit_box->instance_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(struct light_game_implicit_box_instance) * instance_count, instance, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return 0;
}	

int light_game_implicit_box_instance_commit(struct light_game_implicit_box *implicit_box, struct light_game_implicit_box_instance *instance, uint32_t instance_count)
{

	glBindBuffer(GL_ARRAY_BUFFER, implicit_box->instance_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(struct light_game_implicit_box_instance) * instance_count, instance);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return 0;
}


void light_game_implcit_box_render(struct light_game_implicit_box *implicit_cyinder, struct light_game_implicit_box_instance *instance, uint32_t instance_count)

{
	glDisable(GL_DEPTH_TEST);
	light_surface_render_instanced(&implicit_cyinder->surface, instance_count);
	glEnable(GL_DEPTH_TEST);
}





int main(int args, char *argv[])
{
	int result;

	
	/* OpenGL Platform initialization */
	result = light_platform_initialize();
	if(result < 0)
	{
		fprintf(stderr, "Error: could not initialize platform.\n");
		exit(EXIT_FAILURE);
	}
	light_platform_update();


	
	
	/*Create a quad as render surface */	
	struct light_surface quad_surface;
	result = light_surface_initialize(&quad_surface);
	if(result <0)
	{
		printf("light_surface_initialize(): Failed. \n");
		exit(EXIT_FAILURE);	
	}

	

	/* Initialize framebuffer */
	uint32_t frame_width = 512, frame_height = 512;
	struct light_framebuffer framebuffer;
	light_framebuffer_initialize(&framebuffer, frame_width, frame_height);
	
	struct light_scene_instance scene;
	result = light_scene_initialize(&scene);
	if(result < 0)
	{
		printf("light_scene_initialize(): Failed. \n");
		exit(EXIT_FAILURE);
	}




	struct light_game_implicit_sphere implicit_sphere;
	result = light_game_implicit_sphere_init(&scene, &implicit_sphere);
	if(result < 0)
	{
		printf("light_game_implcit_sphere_init(): failed. \n");	
		exit(EXIT_FAILURE);
	}
	
	const uint32_t game_mesh_instance_count = 10;
	struct light_game_implicit_sphere_instance game_mesh_instance[game_mesh_instance_count];
	result = light_game_implicit_sphere_instance_init(&implicit_sphere, game_mesh_instance, game_mesh_instance_count);
	
	uint32_t body_count = game_mesh_instance_count;
	struct light_physic_particle *body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * body_count); 
	for(uint32_t i = 0; i < body_count; i++)
	{
		{
			game_mesh_instance[i].radius = 1.0f;

			body[i] = (struct light_physic_particle){
				.position = (struct vec3){.x = 1.0f*i, .y = -2.0f*i, .z = 10.0f},
				.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
				.mass = 1.0f, 
				//.radius = 1.0f,
			};
		}
	}
	

	struct light_game_implicit_cylinder implicit_cylinder;
	result = light_game_implicit_cylinder_init(&scene, &implicit_cylinder);
	if(result < 0)
	{
		printf("light_game_implcit_sphere_init(): failed. \n");	
		exit(EXIT_FAILURE);
	}

	const uint32_t cylinder_count = 5;
	struct light_game_implicit_cylinder_instance cylinder_instance[cylinder_count];
	result = light_game_implicit_cylinder_instance_init(&implicit_cylinder, cylinder_instance, cylinder_count);
	

	struct light_physic_particle *cylinder_body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * cylinder_count); 
	for(uint32_t i = 0; i < cylinder_count; i++)
	{
		{
			cylinder_instance[i].radius = 1.0f;
			cylinder_instance[i].height = 1.0f;

			cylinder_body[i] = (struct light_physic_particle){
				.position = (struct vec3){.x = 1.0f*i, .y = 2.0f*i, .z = 10.0f},
				.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
				.mass = 1.0f, 
				//.radius = 1.0f,
			};
		}
	}


	struct light_game_implicit_box implicit_box;
	result = light_game_implicit_box_init(&scene, &implicit_box);
	if(result < 0)
	{
		printf("light_game_implcit_sphere_init(): failed. \n");	
		exit(EXIT_FAILURE);
	}

	const uint32_t box_count = 5;
	struct light_game_implicit_box_instance box_instance[box_count];
	result = light_game_implicit_box_instance_init(&implicit_box, box_instance, box_count);
	

	struct light_physic_particle *box_body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * box_count); 
	for(uint32_t i = 0; i < box_count; i++)
	{
		{
			box_instance[i].dimension = (struct vec3){.x = 1.0f, .y = 1.0f, .z = 1.0f};

			box_body[i] = (struct light_physic_particle){
				.position = (struct vec3){.x = -1.0f*i, .y = -2.0f*i, .z = 10.0f},
				.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
				.mass = 1.0f, 
				//.radius = 1.0f,
			};
		}
	}


	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	clock_t 	fps_sample_last		= clock();
	
	clock_t time = clock();
 	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);	
	while(!light_platform_exit())
    	{

	
		float deltatime = (float)(clock() - time)/(float)CLOCKS_PER_SEC;

		time = clock();

		fps_frame_count++;
		float fps_interval_time = (float)(clock() - fps_sample_last)/(float)CLOCKS_PER_SEC;
		if(fps_interval_time > fps_sample_interval)
		{
			uint32_t frames_per_sec = (float)fps_frame_count/fps_interval_time;
			fps_frame_count  = 0;
			fps_sample_last = clock();
			
			printf("5s FPS average: %u \n", frames_per_sec);

		}
#if 0
		struct vec3 gravity = (struct vec3){.x = 0.0f, .y = -9.82f, .z=0.0f};
		for(uint32_t i = 1; i < body_count; i++)
		{
			body[i].velocity = v3add(body[i].velocity, v3scl(gravity, deltatime));	
		}
	
		body[0].velocity = (struct vec3){.x = 0.0f, .y=0.0f, .z=0.0f};	
		body[0].position = (struct vec3){.x = 0.0f, .y=0.0f, .z=10.0f};	
		light_physic_rope(body, body_count);
#endif 
	
		
		/* Copy the physic simulation into the rendering buffer */
		for(uint32_t i = 0; i < game_mesh_instance_count; i++)
		{
			body[i].position = v3add(body[i].position, v3scl(body[i].velocity, deltatime));
			struct vec3 p = body[i].position;
			game_mesh_instance[i].translation = m4x4trs(p);
		}


		light_game_implicit_sphere_instance_commit(&implicit_sphere, game_mesh_instance, game_mesh_instance_count);
	

		for(uint32_t i = 0; i < cylinder_count; i++)
		{
			cylinder_body[i].position = v3add(cylinder_body[i].position, v3scl(cylinder_body[i].velocity, deltatime));
			struct vec3 p = cylinder_body[i].position;
			cylinder_instance[i].translation = m4x4trs(p);
		}


		light_game_implicit_cylinder_instance_commit(&implicit_cylinder, cylinder_instance, cylinder_count);



		for(uint32_t i = 0; i < box_count; i++)
		{
			box_body[i].position = v3add(box_body[i].position, v3scl(box_body[i].velocity, deltatime));
			struct vec3 p = box_body[i].position;
			box_instance[i].translation = m4x4trs(p);
		}


		light_game_implicit_box_instance_commit(&implicit_box, box_instance, box_count);




	
		uint32_t width, height;
		light_platform_resolution(&width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
	

		/* Resize and clear framebuffer */	
		light_framebuffer_resize(&framebuffer, width, height);

		glBindTexture(GL_TEXTURE_2D, framebuffer.color_texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, NULL);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer);
				
			


		light_scene_update(&scene, width, height, deltatime);

		light_game_implcit_sphere_render(&implicit_sphere, game_mesh_instance, game_mesh_instance_count);

		light_game_implcit_cylinder_render(&implicit_cylinder, cylinder_instance, cylinder_count);

		light_game_implcit_box_render(&implicit_box, box_instance, box_count);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);

		light_surface_render(&quad_surface, framebuffer.color_texture);

		light_platform_update();
    	}
	
	printf( "Exiting");
	light_platform_deinitialize();
		
	return (result);
}
