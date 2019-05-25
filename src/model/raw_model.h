#ifndef RAW_MODEL_H
#define RAW_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../math/vec.h"


struct vertex
{
	vec3 position;
	vec3 normal;
};

struct raw_model 
{
	uint32_t vertex_count;
	uint32_t indice_count;
	
	// Vertexdata ... // 
	// Indicedata ... //
};




vertex *raw_model_vertices( raw_model * model, uint32_t * count )
{
	*count = model->vertex_count;
	vertex *address = ( vertex *)(( char *)model + sizeof( raw_model ));
	return ( address );
}

uint32_t *raw_model_indices( raw_model * model, uint32_t * count )
{
	*count = model->indice_count;
	uint32_t dummy = 0;
	uint32_t* address = (uint32_t*)(raw_model_vertices( model, &dummy) + model->vertex_count);
	return ( address ); 
}


raw_model * raw_model_create( const vertex *vertices, uint32_t vertex_count, const uint32_t *indices, uint32_t indice_count )
{
	
	raw_model * result = ( raw_model *)malloc( sizeof(raw_model) + sizeof( vertex ) * vertex_count + sizeof( uint32_t ) * indice_count );
	
	result->vertex_count = vertex_count;
	result->indice_count = indice_count;
	
	uint32_t vertex_cpy_count = 0;
	vertex * vertex_dest = raw_model_vertices( result, &vertex_cpy_count );
	memcpy( vertex_dest, vertices, vertex_cpy_count * sizeof( vertex ) );
	
	uint32_t indice_cpy_count = 0;
	uint32_t * indice_dest = raw_model_indices( result, &indice_cpy_count );
	memcpy( indice_dest, indices, indice_cpy_count * sizeof( uint32_t ) );
	
	
	return ( result );
}


void raw_model_normals_compute( )
{
	
}

void raw_model_release( raw_model * model )
{
	model->vertex_count = 0;
	model->indice_count = 0;
	
	free( model );

}

raw_model * raw_model_load( FILE *fp )
{
	raw_model *result = 0;

	raw_model model_header;
	uint32_t read_result = ( uint32_t )fread( &model_header, 1, sizeof( raw_model ), fp );

	if( read_result == sizeof( model_header ))
	{
		uint32_t size = 0;
		size += model_header.vertex_count * sizeof( vertex );
		size += model_header.indice_count * sizeof( uint32_t );
		
		result = (raw_model*)malloc( sizeof( raw_model ) + size );
		
		if( result )
		{
			read_result = ( uint32_t )fread( (char*)result + sizeof( raw_model ), 1, size, fp );
		
			if( read_result == size)
			{
				memcpy( result, &model_header, sizeof( raw_model ));
			}	
			else{
				free( result );
				result = 0;
			}
		}
	}
	
	return result;
}

uint32_t raw_model_save( FILE *fp, raw_model * model )
{
	uint32_t size = sizeof( vertex ) * model->vertex_count
				  + sizeof( uint32_t ) * model->indice_count
				  + sizeof( raw_model );
				  

	return ( uint32_t )fwrite( model, sizeof( char ), size, fp );
}



#endif //RAW_MODEL_H