#include <assert.h>

struct game
{
	
	struct vertex_buffer_handler spaceship_model_handler;
	struct physic_instance spaceship_physic_handler;
	struct render_instance spaceship_render_handler;
	struct render_instance gsrh_handler;
};

int load_model(struct vertex_buffer *buffer, struct vertex_buffer_handler *handler, const char *filename)
{
	assert(buffer||filename||handler);

	int result = -1;
	FILE *fp = fopen(filename, "rb");
	if(fp)
	{
		struct raw_model *raw_model_data = raw_model_load(fp);
		if(raw_model_data)
		{
			uint32_t vertex_count = 0;
			struct vertex *vertices = raw_model_vertices(raw_model_data, &vertex_count);
			
			uint32_t indice_count = 0;
			uint32_t *indices = raw_model_indices(raw_model_data, &indice_count);
			
			*handler = vertex_buffer_push(buffer, vertices, vertex_count, indices, indice_count); 					
			raw_model_release(raw_model_data);
			result = 1;
		}
		fclose(fp);
	}
	return result;
}



int32_t game_update(struct game *game, struct scene_instance *scene, const float deltatime)
{
	int32_t result = 1;	

	int glow = 5;
	int regular = 5;
	const int num = glow + regular;
	
	struct physic_body * body = physic_instance_bodies(&scene->physic, &game->spaceship_physic_handler);
	renderer_instance_dwrite_instances(&game->spaceship_render_handler, body, regular); 
	renderer_instance_dwrite_instances(&game->gsrh_handler, &body[regular], glow); 

	


	return result;
}

int32_t game_create_instance(struct game *game, struct scene_instance *scene)
{

	int32_t result = load_model(&scene->vertex_buffer, &game->spaceship_model_handler, "data/cube.raw");
	if(result)
	{
		vertex_buffer_commit(&scene->vertex_buffer);
	
		int glow = 5;
		int regular = 5;
		const int num = glow + regular;
		physic_instance_initialize(&scene->physic, &game->spaceship_physic_handler, &game->spaceship_model_handler, num);
		
		struct physic_body bodies[num];
		for(uint32_t k = 0; k < num; k++)
		{
			bodies[k].position = {k*10.15f,k*10.15f,k*12.15f};
			bodies[k].rotation = {0,0,0};
			bodies[k].velocity = {0,0,0};
		}
		
		physic_instance_commit(&scene->physic, &game->spaceship_physic_handler, bodies, num);

		
		renderer_instance_initialize(&scene->renderer,  {scene->vertex_buffer, game->spaceship_model_handler}, &game->spaceship_render_handler);
		renderer_instance_initialize(&scene->renderer, {scene->vertex_buffer, game->spaceship_model_handler}, &game->gsrh_handler);


		struct physic_body * body = physic_instance_bodies(&scene->physic, &game->spaceship_physic_handler);
		renderer_instance_dwrite_instances(&game->spaceship_render_handler, body, regular); 
		renderer_instance_dwrite_instances(&game->gsrh_handler, &body[regular], glow); 



	}
	else
	{
		printf("Failed to load gamefile \n");	
	}

	return(result);
}
