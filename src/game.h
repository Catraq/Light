
int load_model(struct vertex_buffer *buffer, struct vertex_buffer_handler *handler,  const char *filename )
{
	int result = -1;
	FILE *fp = fopen(filename, "rb");
	if( fp ){
		
		raw_model *raw_model_data = raw_model_load(fp);
		
		if( raw_model_data )
		{
			uint32_t vertex_count = 0;
			vertex *vertices = raw_model_vertices( raw_model_data, &vertex_count);
			
			uint32_t indice_count = 0;
			uint32_t *indices = raw_model_indices( raw_model_data, &indice_count );
			
			*handler = vertex_buffer_push( buffer, vertices, vertex_count, indices, indice_count ); 					
			raw_model_release( raw_model_data );
			result = 0;
		}
		fclose( fp );
	}
	
	return (result);
}



struct game
{
	
	struct vertex_buffer_handler spaceship_model_handler;
	struct vertex_buffer_handler floor_model_handler;
	struct physic_instance spaceship_physic_handler;
	struct physic_instance floorphy;
	struct render_instance spaceship_render_handler;
	struct render_instance floor_render_handler;
};

uint32_t game_required_memory()
{
	uint32_t size = 10;

	return(size);
}

int32_t game_create_instance(struct game *game, struct engine_core *core, uint8_t *memory)
{
	int32_t result = 0;

	result |= load_model(&core->vertex_buffer, &game->floor_model_handler,  "data/floor.raw");	
	result |= load_model(&core->vertex_buffer, &game->spaceship_model_handler, "data/ship2.raw");
	
	{
		const int num = 14;
		const uint32_t spaceship_count = num * num * num;
		physic_instance_initialize(&core->physic, &game->spaceship_physic_handler, &game->spaceship_model_handler, spaceship_count);
		
		physic_body bodies[spaceship_count];
		
		for( uint32_t i = 0; i < num; i++)
		{
			for( uint32_t j = 0; j < num; j++)
			{
				for( uint32_t k = 0; k < num; k++)
				{
					bodies[i + num * (j+num * k)].position = {j*10.15f,i*10.15f,k*12.15f};
					bodies[i + num * (j+num * k)].rotation = {0,0,0};
					bodies[i + num * (j+num * k)].velocity = {0,0,0};
				}
			}
		}
		
		physic_instance_commit(&core->physic, &game->spaceship_physic_handler, bodies, spaceship_count);
	}
	
	
	{
		const uint32_t count = 1;
		physic_instance_initialize(&core->physic, &game->floorphy, &game->floor_model_handler, count);
		
		physic_body bodies[count];
		
		bodies[0].position = vec3{0,-1.0f,0};
		bodies[0].rotation = vec3{0,0,0};
		bodies[0].velocity = vec3{0,0,0};
		
		physic_instance_commit(&core->physic, &game->floorphy, bodies, count);
	}
		
	renderer_instance_initialize(&core->renderer, &game->spaceship_render_handler,
															 &core->vertex_buffer, &game->spaceship_model_handler);

	renderer_instance_dwrite_instances(&game->spaceship_render_handler, physic_instance_bodies(&core->physic, &game->spaceship_physic_handler), game->spaceship_physic_handler.count );
	
	renderer_instance_initialize(&core->renderer, &game->floor_render_handler,
															 &core->vertex_buffer, &game->floor_model_handler);
	renderer_instance_dwrite_instances(&game->floor_render_handler, physic_instance_bodies(&core->physic, &game->floorphy), game->floorphy.count);
	
	


	return(result);
}
