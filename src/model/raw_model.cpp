#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "raw_model.h"

struct vertex *raw_model_vertices(struct raw_model * model, uint32_t * count)
{
	*count = model->vertex_count;
	struct vertex *address = (struct vertex *)(( char *)model + sizeof(struct raw_model));
	return  address;
}

uint32_t *raw_model_indices(struct raw_model * model, uint32_t *count)
{
	*count = model->indice_count;
	uint32_t dummy = 0;
	uint32_t* address = (uint32_t*)(raw_model_vertices( model, &dummy) + model->vertex_count);
	return address; 
}


struct raw_model * raw_model_create(const vertex *vertices, uint32_t vertex_count, const uint32_t *indices, uint32_t indice_count)
{
	
	struct raw_model * result = (struct raw_model *)malloc( sizeof(struct raw_model) + sizeof(struct vertex) * vertex_count + sizeof( uint32_t ) * indice_count );
	
	result->vertex_count = vertex_count;
	result->indice_count = indice_count;
	
	uint32_t vertex_cpy_count = 0;
	struct vertex * vertex_dest = raw_model_vertices(result, &vertex_cpy_count);
	memcpy( vertex_dest, vertices, vertex_cpy_count * sizeof(struct vertex ) );
	
	uint32_t indice_cpy_count = 0;
	uint32_t * indice_dest = raw_model_indices(result, &indice_cpy_count);
	memcpy(indice_dest, indices, indice_cpy_count * sizeof(uint32_t));
	
	
	return result;
}


void raw_model_normals_compute( )
{
	
}

void raw_model_release(struct raw_model * model)
{
	model->vertex_count = 0;
	model->indice_count = 0;
	
	free(model);
}

int32_t raw_model_size(FILE *fp, uint32_t *vertex_size, uint32_t *indice_size)
{
	struct raw_model model_header;
	uint32_t read_result = (uint32_t)fread(&model_header, 1, sizeof(struct raw_model), fp);
	if(read_result == sizeof(model_header))
	{
		*vertex_size = model_header.vertex_count * sizeof(struct vertex);
		*indice_size = model_header.indice_count * sizeof(uint32_t);
		return 1;
	}

	return -1;
}

struct raw_model * raw_model_load(FILE *fp)
{
	struct raw_model *result = 0;

	struct raw_model model_header;
	uint32_t read_result = ( uint32_t )fread(&model_header, 1, sizeof(struct raw_model), fp);

	if(read_result == sizeof(model_header))
	{
		uint32_t size = 0;
		size += model_header.vertex_count * sizeof(struct vertex);
		size += model_header.indice_count * sizeof(uint32_t);
		
		result = (struct raw_model*)malloc(sizeof(struct raw_model) + size);
		
		if(result)
		{
			read_result = (uint32_t)fread((char*)result + sizeof(struct raw_model), 1, size, fp);
		
			if(read_result == size)
			{
				memcpy(result, &model_header, sizeof(struct  raw_model));
			}	
			else{
				free(result);
				result = 0;
			}
		}
	}
	
	return result;
}

uint32_t raw_model_save(FILE *fp, struct raw_model * model)
{
	uint32_t size = sizeof(struct vertex) * model->vertex_count
				  + sizeof(uint32_t) * model->indice_count
				  + sizeof(struct raw_model);
				  

	return (uint32_t)fwrite(model, sizeof(char), size, fp);
}



