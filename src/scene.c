#include "scene.h"

#include "error.h"

void light_scene_object_commit(
		struct light_scene_state_instance *instance,
		struct light_scene_object *object,
		uint32_t object_count
)
{
	struct light_scene_implicit_object object_node[object_count];
	
	for(uint32_t i = 0; i < object_count; i++)
	{

		struct mat4x4 TRS = m4x4mul(
				m4x4rote(object[i].rotation),
				m4x4scl(object[i].scale)
		);

		TRS = m4x4mul(m4x4trs(object[i].position), TRS);
		
		int dummy = 0;
		object_node[i].object_index = object[i].object_index;
		object_node[i].translation = TRS;
		object_node[i].translation_inv = m4x4inv(&TRS, &dummy); 

	}

	light_scene_implicit_commit_objects(
			instance,
			object_node, 
			object_count
	);
}

const char *
light_scene_object_implicit_name(struct light_scene_instance *instance, uint32_t index)
{
	return instance->state_instance.implicit_instance.implicit_function_name[index];
}

uint32_t 
light_scene_object_implicit_name_count(struct light_scene_instance *instance)
{
	return instance->state_instance.implicit_instance.implicit_function_name_count;
}



int light_scene_initialize(
		struct light_scene_instance *instance,
	       	struct light_platform *platform,
		struct light_scene_state_build state_build 
)
{
	int result = 0;

	
	struct light_camera_view_state *view_state = &instance->view_state;
	//Camera attributes 
	const float fov = 3.14f/2.0f;

	view_state->fov = fov;
	view_state->near = 0.1f;
	view_state->far = 100.0f;
	
	//Setup camera. 
	view_state->position = (struct vec3){0.0f, 0.0f, 0.0f};
	view_state->rotation = (struct vec3){0.0f, 0.0f, 0.0f};

	result = light_camera_initialize(view_state);
	CHECK_GL_ERROR();
	if(result < 0){
		fprintf(stderr, "light_camera_initialize() failed. \n");
		return -1;	
	}


	/* Initialize framebuffer */
	uint32_t frame_width = 512, frame_height = 512;
	result = light_framebuffer_initialize(&instance->framebuffer, frame_width, frame_height);
	CHECK_GL_ERROR();
	if(result < 0)
	{
		light_camera_deinitialize(view_state);
		fprintf(stderr, "light_framebuffer_initialize() failed. \n");
		return -1;	
	}
	
	
	result = light_scene_state_initialize(&instance->state_instance, state_build);
	CHECK_GL_ERROR();
	if(result < 0){

		light_camera_deinitialize(view_state);
		light_framebuffer_deinitialize(&instance->framebuffer);

		printf("light_scene_state_initialize(): failed. \n");
		return -1;	
	}

	return 0;
}

void light_scene_deinitialize(struct light_scene_instance *instance)
{

	
	light_camera_initialize(&instance->view_state);
	light_framebuffer_deinitialize(&instance->framebuffer);
	light_scene_state_deinitialize(&instance->state_instance);
}


int light_scene_update(
		struct light_scene_instance *instance,
	       	struct light_platform *platform,
	       	uint32_t width, uint32_t height,
	       	const float deltatime,
		struct light_scene_object *objects,
		uint32_t object_count)
{	
	struct light_frame_info frame_result = light_frame_info_update(
			&instance->frame_info,
			platform
	);

	struct vec2 delta_mouse = light_frame_info_mouse_delta(&frame_result, &instance->frame_info);
	instance->frame_info = frame_result; 

	light_camera_input_update(
			&instance->update_state,
		       	&instance->view_state,
		       	platform,
		       	10.0f,
		       	delta_mouse, 
			deltatime
	);

	CHECK_GL_ERROR();

	light_camera_view_matrix(&instance->view_state, width, height);
	CHECK_GL_ERROR();

	/* Resize and clear framebuffer */	
	light_framebuffer_resize(&instance->framebuffer, width, height);
	CHECK_GL_ERROR();

	glViewport(0,0, width, height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	light_camera_buffer_bind(&instance->view_state);
	CHECK_GL_ERROR();



	uint32_t object_node_count = instance->state_instance.implicit_instance.object_node_count;
	uint32_t object_node_pair = object_node_count*(object_node_count - 1)/2;

	struct light_scene_implicit_collision collision[object_node_pair];

	uint32_t collision_count = light_scene_state_dispatch(
			&instance->state_instance, 
			&instance->framebuffer, 
			width, height, deltatime,
			collision,
			object_node_pair
			);
	
	struct mat3x3 inerita = m3x3id();
	for(uint32_t i = 0; i < 3; i++)
	{
		inerita.m[3*i+i] = 10.0f;	
	}
	
	int result = 0;
	struct mat3x3 inertia_inv = m3x3inv(inerita, &result);
	

	for(uint32_t k = 0; k < collision_count; k++){
		uint32_t i = collision[k].a;			
		uint32_t j = collision[k].b;			

		struct vec3 d = v3sub(objects[i].position, objects[j].position);
		struct vec3 r_a = v3sub(collision[k].position, objects[i].position);
		struct vec3 r_b = v3sub(collision[k].position, objects[j].position);
		
		struct vec3 J_vec3[4];
		J_vec3[0] = v3scl(d, -1);
		J_vec3[1] = v3scl(v3cross(r_a, d), -1);
		J_vec3[2] = d;
		J_vec3[3] = v3cross(r_b, d);

		struct vec3 J_M_inv_vec3[4];
		J_M_inv_vec3[0] = v3scl(J_vec3[0], 1/objects[i].mass);
		J_M_inv_vec3[1] = m3x3mulv3(inertia_inv, J_vec3[1]);
		J_M_inv_vec3[2] = v3scl(J_vec3[2], 1/objects[j].mass);
		J_M_inv_vec3[3] = m3x3mulv3(inertia_inv, J_vec3[3]);

		float J_M_inv_J_t = v3dot(J_M_inv_vec3[0], J_vec3[0]) + v3dot(J_M_inv_vec3[1], J_vec3[1]) + v3dot(J_M_inv_vec3[2], J_vec3[2])  + v3dot(J_M_inv_vec3[3], J_vec3[3]); 
		float J_dq_dt = v3dot(J_vec3[0], objects[i].velocity) + v3dot(J_vec3[1], objects[i].angular_velocity) + v3dot(J_vec3[2], objects[j].velocity) + v3dot(J_vec3[3], objects[j].angular_velocity);

		float k_d = 0.5/deltatime;


		float lambda = (-J_dq_dt/deltatime - k_d) /(J_M_inv_J_t);
		if(lambda > 0)
			lambda = 0;

		objects[i].velocity = v3add(objects[i].velocity, v3scl(J_vec3[0], lambda*deltatime/objects[i].mass));
		objects[i].angular_velocity = v3add(objects[i].angular_velocity, v3scl(m3x3mulv3(inertia_inv, J_vec3[1]), lambda*deltatime));

		objects[j].velocity = v3add(objects[j].velocity, v3scl(J_vec3[2], lambda*deltatime/objects[j].mass));
		objects[j].angular_velocity = v3add(objects[j].angular_velocity, v3scl(m3x3mulv3(inertia_inv, J_vec3[3]), lambda*deltatime));
		
	}
		
	
	for(uint32_t i = 0; i < object_count; i++){
		objects[i].position = v3add(objects[i].position, v3scl(objects[i].velocity, deltatime));
		objects[i].rotation = v3add(objects[i].rotation, v3scl(objects[i].angular_velocity, deltatime));
	}




	return 0;
}










