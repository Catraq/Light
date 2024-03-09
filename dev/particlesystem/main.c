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

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

struct light_kd_tree_node
{

	struct light_kd_tree_node *right;
	struct light_kd_tree_node *left;

	struct vec3 position;	
};

uint32_t
light_kd_tree_node_partition(struct light_kd_tree_node *tree_node, uint32_t *tree_node_sorted,  uint32_t tree_node_dimension,  uint32_t low, uint32_t high)
{
	float *pivot_array = &tree_node[tree_node_sorted[high]].position;
	float pivot = pivot_array[tree_node_dimension];

	uint32_t i = low-1;

	for(uint32_t j = low; j < high; j++)
	{
		float *value_array = &tree_node[tree_node_sorted[j]].position;
		float value = value_array[tree_node_dimension];

		if(value <= pivot)
		{
			i++;

			uint32_t t = tree_node_sorted[i];
			tree_node_sorted[i] = tree_node_sorted[j];	
			tree_node_sorted[j] = t;	
		}
	}


	uint32_t t = tree_node_sorted[i+1];
	tree_node_sorted[i+1] = tree_node_sorted[high];	
	tree_node_sorted[high] = t;	


	return i+1;
}


void
light_kd_tree_node_quicksort(struct light_kd_tree_node *tree_node, uint32_t *tree_node_sorted,  uint32_t tree_node_dimension,  int32_t low, int32_t high)
{
	if(low < high)
	{
		uint32_t p = light_kd_tree_node_partition(tree_node, tree_node_sorted, tree_node_dimension, low, high);
		light_kd_tree_node_quicksort(tree_node, tree_node_sorted, tree_node_dimension,  low, p-1);
		light_kd_tree_node_quicksort(tree_node, tree_node_sorted, tree_node_dimension,  p+1, high);
	}	
}

struct light_kd_tree_node *
light_kd_tree_node_build_internal(struct light_kd_tree_node *tree_node, uint32_t *tree_node_sorted, uint32_t tree_node_dimension, uint32_t tree_node_low,  uint32_t tree_node_high)
{
	struct light_kd_tree_node *root = NULL;


	if(tree_node_low == tree_node_high)
		return &tree_node[tree_node_sorted[tree_node_low]];


	if(tree_node_low < tree_node_high)
	{
		uint32_t median = tree_node_low + (tree_node_high - tree_node_low+1)/2;

		root = &tree_node[tree_node_sorted[median]];

		tree_node_dimension = (tree_node_dimension%+1)%3;

		light_kd_tree_node_quicksort(tree_node, tree_node_sorted, tree_node_dimension, tree_node_low, median - 1);
		root->right = light_kd_tree_node_build_internal(tree_node, tree_node_sorted, tree_node_dimension, tree_node_low, median-1);

		
		if(median + 1 <= tree_node_high)
		{
			light_kd_tree_node_quicksort(tree_node, tree_node_sorted, tree_node_dimension, median + 1, tree_node_high);
			root->left = light_kd_tree_node_build_internal(tree_node, tree_node_sorted, tree_node_dimension, median+1, tree_node_high);
		}
	}

	return root;
}

struct light_kd_tree_node *
light_kd_tree_node_build(struct light_kd_tree_node *tree_node, uint32_t tree_node_count)
{
	uint32_t tree_node_sorted[tree_node_count];
	for(uint32_t i = 0; i < tree_node_count; i++)
	{
		tree_node_sorted[i] = i;
	}
	
	light_kd_tree_node_quicksort(tree_node, tree_node_sorted, 0, 0, tree_node_count - 1);
	return light_kd_tree_node_build_internal(tree_node, tree_node_sorted, 0, 0, tree_node_count-1);
}

struct light_kd_tree_node *
light_kd_tree_node_closest_internal(struct light_kd_tree_node *tree_node, struct vec3 point, uint32_t tree_node_dimension)
{
	if(tree_node == NULL)
		return NULL;

	float *v1 = (float *)&tree_node->position;
	float *v2 = (float *)&point;


	float delta = v2[tree_node_dimension] - v1[tree_node_dimension];

	tree_node_dimension = (tree_node_dimension+1)%3;

	float distance = v3len(v3sub(tree_node->position, point));
	struct light_kd_tree_node *closest = tree_node;


	struct light_kd_tree_node *inc = light_kd_tree_node_closest_internal(delta > 0.0f ? tree_node->left : tree_node->right, point, tree_node_dimension);
	if(inc != NULL){

		if(distance > v3len(v3sub(inc->position, point))){
			closest = inc;
			distance = v3len(v3sub(inc->position, point));
		}

	}
	
	printf("(delta=%f, distance=%f),(%f,%f,%f) \n", delta, distance, tree_node->position.x, tree_node->position.y, tree_node->position.z);
	if(delta > distance)
	{
		struct light_kd_tree_node *dec = light_kd_tree_node_closest_internal(delta > 0.0f ? tree_node->right: tree_node->left, point, tree_node_dimension);
		if(dec != NULL){
			if(distance > v3len(v3sub(dec->position, point))){
				closest = dec;
				distance = v3len(v3sub(dec->position, point));
			}
		}
	}

	return closest;
}


struct light_kd_tree_node *
light_kd_tree_node_closest(struct light_kd_tree_node *tree_node, struct vec3 point)
{

	float *v1 = (float *)&tree_node->position;
	float *v2 = (float *)&point;

	float delta = v2[0] - v1[0];
	
	uint32_t tree_node_dimension = (0+1)%3;

	float distance = v3len(v3sub(tree_node->position, point));
	struct light_kd_tree_node *closest = tree_node;


	struct light_kd_tree_node *inc = light_kd_tree_node_closest_internal(delta > 0.0f ? tree_node->left : tree_node->right, point, tree_node_dimension);
	if(inc != NULL){

		if(distance > v3len(v3sub(inc->position, point))){
			closest = inc;
			distance = v3len(v3sub(inc->position, point));
		}

	}
	
	printf("(delta=%f, distance=%f),(%f,%f,%f) \n", delta, distance, tree_node->position.x, tree_node->position.y, tree_node->position.z);

	if(delta > distance)
	{
		struct light_kd_tree_node *dec = light_kd_tree_node_closest_internal(delta > 0.0f ? tree_node->right: tree_node->left, point, tree_node_dimension);
		if(dec != NULL){
			if(distance > v3len(v3sub(dec->position, point))){
				closest = dec;
				distance = v3len(v3sub(dec->position, point));
			}
		}
	}
	
	return closest;
}

void
light_kd_tree_node_print(struct light_kd_tree_node *tree_node, uint32_t depth)
{
	if(tree_node == NULL)
		return;

	for(uint32_t i = 0; i < depth; i++)
	{
		printf("-");		
	}	
	printf("(%f,%f,%f)->%u \n", tree_node->position.x, tree_node->position.y, tree_node->position.z, depth);
	
	depth++;
	light_kd_tree_node_print(tree_node->left, depth);
	light_kd_tree_node_print(tree_node->right, depth);
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




	const int model_count = 3;
	const char *model_str[model_count];
       	model_str[0] = "../data/sphere.raw";
       	model_str[1] = "../data/ship.raw";	
       	model_str[2] = "../data/cube.raw";	


	struct light_vertex_buffer vertex_buffer;
	struct light_vertex_buffer_handler model[model_count];

	/* Load 3D models to GPU memory.  */
	result = light_vertex_buffer_model_load(&vertex_buffer, model, model_str, model_count);
	if(result < 0){
		fprintf(stderr, "Error: could not load vertex buffer data. \n ");	
		exit(EXIT_FAILURE);
	}
	

	const uint32_t 			n_i = 4;
	const uint32_t 			n_j = 1;

	const uint32_t 			game_mesh_instance_count = n_i*n_j;
	struct light_game_mesh 		game_mesh;
	struct light_game_mesh_instance game_mesh_instance[game_mesh_instance_count];

	result = light_game_mesh_init(&scene, &game_mesh, &vertex_buffer, &model[0]);
	if(result < 0)
	{
		printf("light_game_mesh_init(): Failed. \n");
		exit(EXIT_FAILURE);
	}

	result = light_game_mesh_instance_init(&game_mesh, game_mesh_instance, game_mesh_instance_count);
       	if(result < 0)
	{
		printf("light_game_mesh_instance_init(): Failed. \n");
		exit(EXIT_FAILURE);
	}

	const uint32_t plane_instance_count = 5;
	struct light_scene_plane scene_plane;
	struct light_scene_plane_instance scene_plane_instance[plane_instance_count];

	result = light_scene_plane_init(&scene, &scene_plane);
	if(result < 0)
	{
		printf("light_game_plane_init(): Failed. \n");
		exit(EXIT_FAILURE);
	}

	
	float _y = -20.0f;

	
	uint32_t body_count = game_mesh_instance_count;
	float *body_dt = (float *)malloc(sizeof(float) * body_count);
	//struct light_physic_sphere_body *body = (struct light_physic_sphere_body *)malloc(sizeof(struct light_physic_sphere_body) * body_count); 
	struct light_physic_particle *body = (struct light_physic_particle  *)malloc(sizeof(struct light_physic_particle) * body_count); 
	struct light_physic_intersect *body_intersect = (struct light_physic_intersect *)malloc(body_count * sizeof(struct light_physic_intersect)); 
	
	for(uint32_t i = 0; i < n_i; i++)
	{
		for(uint32_t j = 0; j < n_j; j++)
		{
			body[i*n_j + j] = (struct light_physic_particle){
				.position = (struct vec3){.x = 0.0f*i, .y = 2.0f-4.0f*i, .z = 0.0f*j},
				.velocity = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
				.mass = 1.0f, 
				//.radius = 1.0f,
			};
		}
	}
	

	uint32_t body_cols = 3*game_mesh_instance_count;
	uint32_t body_rows = n_i*(n_j-1) + n_j*(n_i-1);

	printf("rows=%u, cols=%u \n", body_rows, body_cols);


	
	uint32_t plane_static_count = 5;
	struct light_physic_plane_static *plane_static = (struct light_physic_plane_static *)malloc(plane_static_count*sizeof(struct light_physic_plane_static)); 

	plane_static[0] = (struct light_physic_plane_static){
		.normal = (struct vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f},
		.dir= (struct vec3){.x = 1.0f, .y = 0.0f, .z = 0.0f},
		.position = (struct vec3){.x = 0.0f, .y = _y + 0.0f, .z = 0.0f},	
		.width = 50.0f,
		.height = 50.0f
	}; 


	plane_static[1] = (struct light_physic_plane_static){
		.normal = (struct vec3){.x = 1.0f, .y = 0.0f, .z = 0.0f},
		.dir= (struct vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f},
		.position = (struct vec3){.x = 25.0f, .y = _y+50.0f/2.0f, .z = 0.0f},
		.width = 50.0f,
		.height = 50.0f
	}; 

	plane_static[2] = (struct light_physic_plane_static){
		.normal = (struct vec3){.x = 1.0f, .y = 0.0f, .z = 0.0f},
		.dir= (struct vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f},
		.position = (struct vec3){.x = -25.0f, .y = _y+50.0f/2.0f, .z = 0.0f},
		.width = 50.0f,
		.height = 50.0f
	}; 


	plane_static[3] = (struct light_physic_plane_static){
		.normal = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 1.0f},
		.dir= (struct vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f},
		.position = (struct vec3){.x = 0.0f, .y = _y+50.0f/2, .z = 25.0f},
		.width = 50.0f,
		.height = 50.0f
	}; 

	plane_static[4] = (struct light_physic_plane_static){
		.normal = (struct vec3){.x = 0.0f, .y = 0.0f, .z = 1.0f},
		.dir= (struct vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f},
		.position = (struct vec3){.x = 0.0f, .y = _y+50.0f/2.0f, .z = -25.0f},
		.width = 50.0f,
		.height = 50.0f
	}; 


	result = light_scene_plane_instance_init(&scene_plane, &scene_plane_instance[0]);
	if(result < 0)
	{
		printf("light_game_plane_instance_init(): Failed. \n");
		exit(EXIT_FAILURE);
	}
	light_scene_plane_instance_commit(&scene_plane_instance[0], &plane_static[0]);


	result = light_scene_plane_instance_init(&scene_plane, &scene_plane_instance[1]);
	if(result < 0)
	{
		printf("light_game_plane_instance_init(): Failed. \n");
		exit(EXIT_FAILURE);
	}
	light_scene_plane_instance_commit(&scene_plane_instance[1], &plane_static[1]);


	result = light_scene_plane_instance_init(&scene_plane, &scene_plane_instance[2]);
	if(result < 0)
	{
		printf("light_game_plane_instance_init(): Failed. \n");
		exit(EXIT_FAILURE);
	}
	light_scene_plane_instance_commit(&scene_plane_instance[2], &plane_static[2]);

	result = light_scene_plane_instance_init(&scene_plane, &scene_plane_instance[3]);
	if(result < 0)
	{
		printf("light_game_plane_instance_init(): Failed. \n");
		exit(EXIT_FAILURE);
	}
	light_scene_plane_instance_commit(&scene_plane_instance[3], &plane_static[3]);

	result = light_scene_plane_instance_init(&scene_plane, &scene_plane_instance[4]);
	if(result < 0)
	{
		printf("light_game_plane_instance_init(): Failed. \n");
		exit(EXIT_FAILURE);
	}
	light_scene_plane_instance_commit(&scene_plane_instance[4], &plane_static[4]);







	GLint particle_system_program = light_create_program(particle_system_vertex_shader_source, particle_system_fragment_shader_source);
	if(particle_system_program < 0)
	{
		printf("light_create_program(): Failed. \n");
		exit(EXIT_FAILURE);	
	}

	result = light_scene_bind(&scene, particle_system_program);
 	if(result < 0){
		printf("light_scene_bind(): Failed. \n");
		exit(EXIT_FAILURE);
	}	


	struct light_scene_particle_system particle_system;

	result = light_scene_particle_system_init(&particle_system, particle_system_program);
	if(result < 0){
		printf("light_scene_particle_system_init(): Failed. \n");
		exit(EXIT_FAILURE);
	}
	
	const uint32_t particle_system_instance_count = 1;
	struct light_scene_particle_system_instance particle_system_instance[particle_system_instance_count];
	for(uint32_t i = 0; i <  particle_system_instance_count; i++)
	{
		uint32_t particle_count = 5;
		result = light_scene_particle_system_instance_init(&particle_system, &particle_system_instance[i], particle_count);
		if(result < 0){
			printf("light_scene_particle_system_instance_init(): Failed. \n");
			exit(EXIT_FAILURE);
		}

		for(uint32_t j = 0; j < particle_system_instance[i].particle_count; j++)
		{
			float a = 3.14f * (float)rand()/(float)RAND_MAX;
			float r = 0.1f*(float)rand()/(float)RAND_MAX;
			struct vec3 v = (struct vec3){
				.x = r*cos(2*a),
				.y = -1.0f,
				.z = r*sin(2*a), 
		       	};
			
			particle_system_instance[i].particles[j].velocity = v;
			particle_system_instance[i].particles[j].mass = 0.0000001f;
				
		}
		
		struct vec3 p = (struct vec3){.x = 0.0f, .y = 0.0f, .z=10.0f*i};
		particle_system_instance[i].translation = m4x4trs(p);
	}

	


	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	uint32_t 	fps_frame_count 	= 0;
	float 		fps_sample_interval 	= 5.0f;	
	clock_t 	fps_sample_last		= clock();
	
	clock_t time = clock();

	struct light_kd_tree_node kd_tree_node[body_count];


	while(!light_platform_exit())
    	{

		
		for(uint32_t i = 0; i < body_count; i++)
		{
			kd_tree_node[i].left  = NULL;
			kd_tree_node[i].right = NULL;
			kd_tree_node[i].position = body[i].position;
		}

		printf("\n");
		struct light_kd_tree_node *root = light_kd_tree_node_build(kd_tree_node, body_count);

		light_kd_tree_node_print(root, 0);

		for(uint32_t i = 0; i < body_count; i++)
		{
			struct light_kd_tree_node *c = light_kd_tree_node_closest(root, kd_tree_node[i].position);
			if(c != NULL)
			{
				printf("%u->(%f,%f,%f)->(%f,%f,%f)\n",i, kd_tree_node[i].position.x, kd_tree_node[i].position.y, kd_tree_node[i].position.z, c->position.x, c->position.y, c->position.z);		
			
			}
		
		}
		printf("\n");


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


		struct vec3 gravity = (struct vec3){.x = 0.0f, .y = -0.0f, .z = 0.0f};

		#if 0
		for(uint32_t j = 0; j < particle_system_instance_count; j++)
		{
			for(uint32_t i = 0; i < particle_system_instance[j].particle_count; i++)
			{

				particle_system_instance[j].lifetime[i] -= deltatime;

				if(particle_system_instance[j].lifetime[i] <= 0.0f)
				{
					float a = 3.14f * (float)rand()/(float)RAND_MAX;
					float r = 5.0f*(float)rand()/(float)RAND_MAX;
					struct vec3 v = (struct vec3){
						.x = r*cos(2*a),
						.y = -1.0f,
						.z = r*sin(2*a), 
		       			};
			
					particle_system_instance[j].particles[i].velocity = v;
					particle_system_instance[j].particles[i].position = (struct vec3){
						.x = 0.0f, .y = 0.0f, .z = 5.0f
					};

					particle_system_instance[j].lifetime[i] = 10.0f *(float)rand()/(float)RAND_MAX;
				}

				float dt = MIN(particle_system_instance[j].lifetime[i],  deltatime);
				particle_system_instance[j].time[i] = dt;


				particle_system_instance[j].particles[i].velocity = v3add(particle_system_instance[j].particles[i].velocity, v3scl(gravity, deltatime)); 
			}
		}

#endif 
		
		for(uint32_t i = 0; i < body_count; i++)
		{
			body_dt[i] = deltatime;	
			body[i].velocity = v3add(body[i].velocity, v3scl(gravity, deltatime));
		}
#if 0

		for(uint32_t j = 0; j < particle_system_instance_count; j++)
		{
			for(uint32_t i = 0; i < game_mesh_instance_count; i++)
			{
				uint32_t intersect_count = light_physic_sphere_particle_intersect(&body[i], particle_system_instance[j].particles, particle_system_instance[j].intersect, particle_system_instance[j].particle_count);	
				for(uint32_t k = 0; k < intersect_count; k++)
				{
					uint32_t l = particle_system_instance[j].intersect[k].i;
					float t = particle_system_instance[j].intersect[k].t;

					if(t > particle_system_instance[j].time[l] || t > body_dt[i]){
						continue;	
					}
					
					particle_system_instance[j].time[l] -= t;
					//body_dt[i] -= t;

					//body[i].position = v3add(body[i].position, v3scl(body[i].velocity, t));
					particle_system_instance[j].particles[l].position = v3add(particle_system_instance[j].particles[l].position, v3scl(particle_system_instance[j].particles[l].velocity, t));

					light_physic_sphere_particle_resolve(&body[i], &particle_system_instance[j].particles[l]);
				}	
			}




			for(uint32_t i = 0; i < plane_static_count; i++)
			{
				uint32_t intersect_count = light_physic_plane_particle_intersect(&plane_static[i], particle_system_instance[j].particles, particle_system_instance[j].intersect, particle_system_instance[j].particle_count);
				for(uint32_t k = 0; k < intersect_count; k++)
				{
					uint32_t l = particle_system_instance[j].intersect[k].i;
					float t = particle_system_instance[j].intersect[k].t;

					if(t > particle_system_instance[j].time[l]){
						continue;	
					}

					particle_system_instance[j].time[l] -= t;
					particle_system_instance[j].particles[l].position = v3add(particle_system_instance[j].particles[l].position, v3scl(particle_system_instance[j].particles[l].velocity, t));

					light_physic_plane_particle_resolve(&plane_static[i], &particle_system_instance[j].particles[l]);

				}	
			}




		}

	#endif 

#if 0
		for(uint32_t i = 0; i < body_count; i++)
		{
			uint32_t i_c = light_physic_sphere_sphere_intersect(&body[i], &body[i+1], &body_intersect[i+1], body_count - (i + 1));
			for(uint32_t j = 0; j < i_c; j++)
			{
				uint32_t l = body_intersect[i+1+j].i + i + 1;		
				float t = body_intersect[i+1+j].t;	

				if(t > body_dt[l] || t > body_dt[i]){
					continue;
				}

				light_physic_sphere_sphere_resolve(&body[i], &body[l]);
			}
		}



		for(uint32_t i = 0; i < plane_static_count; i++)
		{	
			uint32_t intersect_count = light_physic_plane_sphere_intersect(&plane_static[i], body, body_intersect, body_count);
			for(uint32_t j = 0; j < intersect_count; j++)
			{
				uint32_t l = body_intersect[j].i;
				float t = body_intersect[j].t;

				if(t > body_dt[l]){
					continue;
				}

				body_dt[l] -= t;
				light_physic_plane_sphere_resolve(&plane_static[i], &body[l]);

			}
		}
#endif 

#if  0
		{

			for(uint32_t i = 0; i < n_i; i++)
			{
				for(uint32_t j = 0; j < n_j-1; j++)
				{
					uint32_t k = i*n_j + j;
					uint32_t l = i*n_j + j + 1;

					struct vec3 delta1 = v3sub(body[k].position, body[l].position);
					float len1 = v3len(delta1);

					for(uint32_t p = j; p < n_j-1; p++)
					{
						uint32_t v = i*n_j + p;
						uint32_t b = i*n_j + p + 1;

						struct vec3 delta2 = v3sub(body[v].position, body[b].position);
						float len2 = v3len(delta2);

						float JJ = 0.0f;
					        if(k == v){
							JJ += v3dot(delta1, delta2)/(len1*len2); 
						}
						if(k == b){
							JJ -= v3dot(delta1, delta2)/(len1*len2); 
						}
						if(l == v){
							JJ -= v3dot(delta1, delta2)/(len1*len2); 
						}
						if(l == b){
							JJ += v3dot(delta1, delta2)/(len1*len2); 
						}


						uint32_t m1 = (j + i*(n_j-1))*(n_j*(n_i-1) + n_i*(n_j-1)) + i*(n_j -1)+ p;
						uint32_t m2 = (m1%body_rows)*body_rows + (m1/body_rows);

						//u[m1] = 10*JJ;
						//u[m2] = 10*JJ;
#if 0
						printf("u=(%f, %f)", JJ, u[m1]);
						printf("O((%u %u),(%u, %u))->%u,%u, ", k, l, v, b, m1,m2);
#endif 
					}

					for(uint32_t o = i+1; o < n_i; o++)
					{
						for(uint32_t p = 0; p < n_j-1; p++)
						{
							uint32_t v = o*n_j + p;
							uint32_t b = o*n_j + p + 1;

							struct vec3 delta2 = v3sub(body[v].position, body[b].position);
							float len2 = v3len(delta2);

							float JJ = 0.0f;
							if(k == v){
								JJ += v3dot(delta1, delta2)/(len1*len2); 
							}
							if(k == b){
								JJ -= v3dot(delta1, delta2)/(len1*len2); 
							}
							if(l == v){
								JJ -= v3dot(delta1, delta2)/(len1*len2); 
							}
							if(l == b){
								JJ += v3dot(delta1, delta2)/(len1*len2); 
							}


							uint32_t m1 = (j + i*(n_j-1))*(n_j*(n_i-1) + n_i*(n_j-1)) + o*(n_j -1)+ p;
							uint32_t m2 = (m1%body_rows)*body_rows + (m1/body_rows);

							//u[m1] = 10*JJ;
							//u[m2] = 10*JJ;
#if 0
							printf("O2((%u %u),(%u, %u))->%u,%u, ", k, l, v, b, m1,m2);
#endif 
						}
					}

					for(uint32_t o = 0; o < n_i-1; o++)
					{
						for(uint32_t p = 0; p < n_j; p++)
						{
							uint32_t v = o*n_j + p;
							uint32_t b = (o+1)*n_j + p;
							
							struct vec3 delta2 = v3sub(body[v].position, body[b].position);
							float len2 = v3len(delta2);

							float JJ = 0.0f;
							if(k == v){
								JJ += v3dot(delta1, delta2)/(len1*len2); 
							}
							if(k == b){
								JJ -= v3dot(delta1, delta2)/(len1*len2); 
							}
							if(l == v){
								JJ -= v3dot(delta1, delta2)/(len1*len2); 
							}
							if(l == b){
								JJ += v3dot(delta1, delta2)/(len1*len2); 
							}

							uint32_t m1 = (j + i*(n_j-1))*(n_j*(n_i-1) + n_i*(n_j-1)) + n_i*(n_j -1) + o*(n_j) + p;
							uint32_t m2 = (m1%body_rows)*body_rows + (m1/body_rows);

							//u[m1] = 10*JJ;
							//u[m2] = 10*JJ;
#if 0
							printf("O3((%u %u),(%u, %u))->%u,%u, ", k, l, v, b, m1,m2);
#endif 
						}
					}



					//printf("\n");
				}
			}

			for(uint32_t i = 0; i < n_i-1; i++)
			{
				for(uint32_t j = 0; j < n_j; j++)
				{
					uint32_t k = i*n_j + j;
					uint32_t l = (i+1)*n_j + j;


					struct vec3 delta1 = v3sub(body[k].position, body[l].position);
					float len1 = v3len(delta1);

					for(uint32_t p = j; p < n_j; p++)
					{
						uint32_t v = i*n_j + p;
						uint32_t b = (i+1)*n_j + p;

						struct vec3 delta2 = v3sub(body[v].position, body[b].position);
						float len2 = v3len(delta2);


						float JJ = 0.0f;
						if(k == v){
							JJ += v3dot(delta1, delta2)/(len1*len2); 
						}
						if(k == b){
							JJ -= v3dot(delta1, delta2)/(len1*len2); 
						}
						if(l == v){
							JJ -= v3dot(delta1, delta2)/(len1*len2); 
						}
						if(l == b){
							JJ += v3dot(delta1, delta2)/(len1*len2); 
						}


						uint32_t m1 = n_i*(n_j-1)*(n_i*(n_j-1) + n_j*(n_i-1)) + j*(n_i*(n_j-1) + n_j*(n_i-1)) + i*n_j*(n_i*(n_j-1) + n_j*(n_i-1)) + i*n_j + n_i*(n_j-1) + p;
						uint32_t m2 = (m1%body_rows)*body_rows + (m1/body_rows);

						//u[m1] = 10*JJ;
						//u[m2] = 10*JJ;
#if 0
						printf("O4((%u, %u)(%u %u),(%u, %u))->%u,%u, ",i,j, k, l, v, b, m1,m2);
#endif 
					}


					for(uint32_t o = i+1; o < n_i-1; o++)
					{
						for(uint32_t p = 0; p < n_j; p++)
						{
							uint32_t v = o*n_j + p;
							uint32_t b = (o+1)*n_j + p;

							struct vec3 delta2 = v3sub(body[v].position, body[b].position);
							float len2 = v3len(delta2);


							float JJ = 0.0f;
							if(k == v){
								JJ += v3dot(delta1, delta2)/(len1*len2); 
							}
							if(k == b){
								JJ -= v3dot(delta1, delta2)/(len1*len2); 
							}
							if(l == v){
								JJ -= v3dot(delta1, delta2)/(len1*len2); 
							}
							if(l == b){
								JJ += v3dot(delta1, delta2)/(len1*len2); 
							}

							uint32_t m1 = n_i*(n_j-1)*(n_i*(n_j-1) + n_j*(n_i-1)) + j*(n_i*(n_j-1) + n_j*(n_i-1)) + i*n_j*(n_i*(n_j-1) + n_j*(n_i-1)) + o*n_j + n_i*(n_j-1) + p;
							uint32_t m2 = (m1%body_rows)*body_rows + (m1/body_rows);

							//u[m1] = 10*JJ;
							//u[m2] = 10*JJ;
#if 0
							printf("O5((%u, %u)(%u %u),(%u, %u))->%u, ",i,j, k, l, v, b, m1,m2);
#endif
						}
					}


				//	printf("\n");
				}
			}


		}
#endif 
			
#if 0
		double Jdqdt[body_rows];
		cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, body_rows, 1, body_cols, -alpha, body_J, body_cols, dqdt, 1, beta, Jdqdt, 1);

		double lambda[body_rows];
		cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, body_rows, 1, body_rows, alpha, uinv, body_rows, Jdqdt, 1, beta, lambda, 1);

		double p[body_cols];
		cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans, body_cols, 1, body_rows, alpha, body_J, body_cols, lambda, 1, beta, p, 1);

		double minvp[body_cols];                
		cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, body_cols, 1, body_cols, alpha, Minv, body_cols, p, 1, beta, minvp, 1);
	
		
		for(uint32_t i = 0; i < n_i; i++)
		{
			for(uint32_t j = 0; j < n_j; j++)
			{
				body[i*n_j+j].velocity.x = body[i*n_j+j].velocity.x + minvp[(i*n_j+j)*3] ; 
				body[i*n_j+j].velocity.y = body[i*n_j+j].velocity.y + minvp[(i*n_j+j)*3 +1] ; 
				body[i*n_j+j].velocity.z = body[i*n_j+j].velocity.z + minvp[(i*n_j+j)*3+2] ; 

			}
		}
#endif 
#if 0
		light_physic_rope(body, body_count);
#endif 
		
		struct vec3 va[n_i];	
				
		float b = 0.1f;
		float k = 0.1f;
#if 0	
		struct vec3 center = {.x=0.0f, .y=0.0f, .z=0.0f};
		struct vec3 x = v3scl(v3sub(body[0].position, center), k/body[0].mass);
		struct vec3 n = v3scl(x, 1/v3len(x));

		struct vec3 v = v3scl(v3scl(body[0].velocity,v3dot(body[0].velocity,n)), b/body[0].mass);

		va[0] = v3add(v,x);
#endif 
		
		for(uint32_t i = 1; i < n_i; i++)
		{

			struct vec3 x = v3sub(body[i].position, body[i-1].position);
			struct vec3 x1 = v3scl(x, k/body[i].mass);
			struct vec3 x2 = v3scl(x, -k/body[i-1].mass);

			struct vec3 n = v3scl(x, 1/v3len(x));

			struct vec3 v = v3sub(v3scl(body[i].velocity,v3dot(body[i].velocity,n)), v3scl(body[i-1].velocity,v3dot(body[i-1].velocity,n)));
			struct vec3 v1 = v3scl(v, b/body[i].mass);
			struct vec3 v2 = v3scl(v, b/body[i-1].mass);
			
			va[i-1] = v3add(v2,x2);
			va[i] = v3add(v1,x1);
		}

		for(uint32_t i = 0; i < n_i; i++)
		{
			body[i].velocity = v3sub(body[i].velocity, v3scl(va[i], deltatime)); 
		}
		
		for(uint32_t i = 0; i < game_mesh_instance_count; i++)
		{
			body[i].position = v3add(body[i].position, v3scl(body[i].velocity, body_dt[i]));
			struct vec3 p = body[i].position;
			game_mesh_instance[i].translation = m4x4trs(p);
		}

		light_game_mesh_instance_commit(&game_mesh, game_mesh_instance, game_mesh_instance_count);



		for(uint32_t j = 0; j < particle_system_instance_count; j++)
		{
			for(uint32_t i = 0; i < particle_system_instance[j].particle_count; i++)
			{
				particle_system_instance[j].particles[i].position = v3add(particle_system_instance[j].particles[i].position, v3scl(particle_system_instance[j].particles[i].velocity, particle_system_instance[j].time[i]));
			}

			light_scene_particle_system_instance_commit(&particle_system_instance[j]);
		}


	

	
		uint32_t width, height;
		light_platform_resolution(&width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
	

		/* In render */	
		light_framebuffer_resize(&framebuffer, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer);

		light_scene_update(&scene, width, height, deltatime);

		light_scene_particle_system_render(&particle_system, particle_system_instance, particle_system_instance_count);

		light_game_mesh_render(&game_mesh, game_mesh_instance, game_mesh_instance_count);

		//light_scene_plane_render(&scene_plane, scene_plane_instance, plane_instance_count);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);

		light_surface_render(&quad_surface, framebuffer.color_texture);

		light_platform_update();
    	}
	
	printf( "Exiting");
	light_platform_deinitialize();
		
	return (result);
}
