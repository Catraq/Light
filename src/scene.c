#include "scene.h"

static void light_scene_view_initialize(struct light_camera_view_state *view_state, GLuint buffer_base_index, uint32_t width, uint32_t height)
{

	light_camera_initialize(view_state, buffer_base_index);

	//Camera attributes 
	const float fov = 3.14f/3.0f;

	view_state->fov = fov;
	
	//Setup camera. 
	view_state->position = (struct vec3){0.0f, 0.0f, 0.0f};
	view_state->rotation = (struct vec3){0.0f, 0.0f, 0.0f};

}




static int light_scene_buffer_initialize(struct light_scene_instance_buffer *instance_buffer, struct light_scene_instance_build *instance_build)
{
	glGenBuffers(1, &instance_buffer->sphere_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance_buffer->sphere_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(struct light_scene_implicit_sphere_instance) * instance_build->sphere_count, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &instance_buffer->cylinder_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance_buffer->cylinder_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(struct light_scene_implicit_cylinder_instance) * instance_build->cylinder_count, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &instance_buffer->box_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance_buffer->box_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(struct light_scene_implicit_box_instance) * instance_build->box_count, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &instance_buffer->light_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance_buffer->light_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(struct light_scene_light_light_instance) * instance_build->light_count, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	return 0;
}

void light_scene_deinitialize(struct light_scene_instance *instance)
{
	glDeleteBuffers(1, &instance->instance_buffer.sphere_buffer);
	glDeleteBuffers(1, &instance->instance_buffer.cylinder_buffer);
	glDeleteBuffers(1, &instance->instance_buffer.box_buffer);
	glDeleteBuffers(1, &instance->instance_buffer.light_buffer);
	
	
	light_framebuffer_deinitialize(&instance->framebuffer);

}


int light_scene_initialize(struct light_scene_instance *instance, struct light_scene_instance_build instance_build)
{
	int result = 0;

	instance->instance_build = instance_build;

	/* Platform Joystick initialization */
	result = light_camera_input_initialize(&instance->update_state);
	if(result < 0){
		fprintf(stderr, "Error: could not initialize camera input.\n");
		return -1;	
	}


	/* Opengl camera uniform block location. */
	const GLuint uniform_buffer_block_location = 0;

	struct light_frame_info frame_info = light_frame_info_update(NULL);
	light_scene_view_initialize(&instance->view_state, uniform_buffer_block_location, frame_info.width, frame_info.height);

	/* Initialize framebuffer */
	uint32_t frame_width = 512, frame_height = 512;
	light_framebuffer_initialize(&instance->framebuffer, frame_width, frame_height);

	return light_scene_buffer_initialize(&instance->instance_buffer, &instance->instance_build);
}
void light_scene_buffer_commit_sphere(struct light_scene_instance *scene, struct light_scene_implicit_sphere_instance *sphere, uint32_t sphere_count, uint32_t sphere_offset)
{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->instance_buffer.sphere_buffer);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(struct light_scene_implicit_sphere_instance) * sphere_offset, sizeof(struct light_scene_implicit_sphere_instance) * sphere_count, sphere);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void light_scene_buffer_commit_cylinder(struct light_scene_instance *scene, struct light_scene_implicit_cylinder_instance *cylinder, uint32_t cylinder_count, uint32_t cylinder_offset)
{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->instance_buffer.cylinder_buffer);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(struct light_scene_implicit_cylinder_instance) * cylinder_offset, sizeof(struct light_scene_implicit_cylinder_instance) * cylinder_count, cylinder);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void light_scene_buffer_commit_box(struct light_scene_instance *scene, struct light_scene_implicit_box_instance *box, uint32_t box_count, uint32_t box_offset)
{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->instance_buffer.box_buffer);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(struct light_scene_implicit_box_instance) * box_offset, sizeof(struct light_scene_implicit_box_instance) * box_count, box);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

	
void light_scene_buffer_commit_light(struct light_scene_instance *scene, struct light_scene_light_light_instance *light, uint32_t light_count, uint32_t light_offset)
{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->instance_buffer.light_buffer);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(struct light_scene_light_light_instance) * light_offset, sizeof(struct light_scene_light_light_instance) * light_count, light);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}


int light_scene_bind_program(struct light_scene_instance *instance, GLint program)
{
	
	return light_camera_buffer_bind(&instance->view_state, program);
}

int light_scene_bind(struct light_scene_instance *instance, uint32_t width, uint32_t height, const float deltatime)
{	
	struct light_frame_info frame_result = light_frame_info_update(&instance->frame_info);
	struct vec2 delta_mouse = light_frame_info_mouse_delta(&frame_result, &instance->frame_info);
	instance->frame_info = frame_result; 

	light_camera_input_update(&instance->update_state, &instance->view_state, 10.0f, delta_mouse, deltatime);

	light_camera_view_matrix(&instance->view_state, width, height);

	/* Resize and clear framebuffer */	
	light_framebuffer_resize(&instance->framebuffer, width, height);
	
	glViewport(0,0, width, height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glBindImageTexture(0, instance->framebuffer.color_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, instance->framebuffer.position_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, instance->framebuffer.normal_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(3, instance->framebuffer.composed_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);


	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0,instance->instance_buffer.sphere_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1,instance->instance_buffer.cylinder_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2,instance->instance_buffer.box_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3,instance->instance_buffer.light_buffer);


	return 0;
}










