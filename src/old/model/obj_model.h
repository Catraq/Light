#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vertex_buffer.h"

/*
	TODO: 
		Fix normals with indices. 
		They are in wrong order now. 
	BUGS:
		Á vertex won't be parsed 
		if it's on the first line.
	
*/

struct obj_model
{
	uint32_t vertex_count;
	uint32_t indice_count;
	
	vertex 	 *vertices;
	uint32_t *indices;
};


void obj_release( obj_model *model )
{
	model->vertex_count = 0;
	model->indice_count  = 0;
	
	free( model->vertices );
	free( model->indices );
	
	model->vertices = 0;
	model->indices  = 0;
}



bool obj_load( const char* filename, obj_model *model )
{	

	vertex 		*vertices 	= 0;
	vec3 		*normals = 0, *positions = 0;
	uint32_t 	*indices 	= 0;
	
	uint32_t 	vertex_count 	= 0, 
				texcoord_count 	= 0, 
				normal_count 	= 0, 
				face_count 		= 0,
				vertex_index 	= 0,
				normal_index 	= 0,
				indice_index 	= 0;
		
	
	bool result = false;
	FILE *fp = fopen( filename, "r");
	
	if(!fp)
	{
		return ( result );
	}
	
	
	rewind( fp );
	
	while( !(feof(fp)) )
	{
		int8_t line[2];
		fscanf(fp, "%2s", line);
	
		if( memcmp(line, "v", sizeof("v") ) == 0 )
		{
			vertex_count++;
		}
		else if( memcmp(line, "vt", sizeof("vt") ) == 0 )
		{
			texcoord_count++;
		}
		else if( memcmp(line, "vn", sizeof("vn") ) == 0 )
		{
			normal_count++;
		}
		else if( memcmp(line, "f", sizeof("f") ) == 0 )
		{
			face_count++;
		}
		
		uint8_t end[1];
		while( end[0] != '\n') {
			fscanf( fp, "%c", end );
		}
		
	}
	rewind( fp );
	
	
	printf("OBJ_LOADER: vertices: %u normals: %u faces: %u  \n", vertex_count, normal_count, face_count);
	
	
	
		
	normals = ( vec3 *) malloc( sizeof( vec3 ) * normal_count );
	if( !normals )
	{
		printf("OBJ_LOADER: could not allocate memory for normals.  \n");
		goto end;
	}

	positions = ( vec3 *) malloc( sizeof( vec3 ) * vertex_count );
	if( !positions )
	{
		printf("OBJ_LOADER: could not allocate memory for positions.  \n");
		goto end;
	}
	
	vertices = ( vertex * )  malloc( sizeof( vertex )   * face_count * 3 );
	if( !vertices )
	{
		printf("OBJ_LOADER: could not allocate memory for vertcies  \n");
		goto end;
	}
	
	indices  = ( uint32_t * )malloc( sizeof( uint32_t ) * face_count *3  );
	if( !indices )
	{  
		printf("OBJ_LOADER: could not allocate memory for indices  \n");
		free( vertices );
		goto end;
	}
	
	
	
	while( !(feof(fp)) )
	{
		int8_t line[2];
		fscanf(fp, "%2s", line);
		
		if( memcmp(line, "v", sizeof("v") ) == 0 )
		{
			if( vertex_index == vertex_count)
			{
				printf("OBJ_LOADER: vertex out of bounds. Max %u got %u", vertex_index, vertex_count);
			}
			else{
				vec3 position;
				fscanf( fp, "%f %f %f \n",  &position.x, &position.y, &position.z);
				positions[vertex_index] = position;
				vertex_index++;
			}
		}
		else if( memcmp(line, "vt",  sizeof( "vt" ) ) == 0 )
		{
			
		}
		else if( memcmp(line, "vn", sizeof( "vn" )  ) == 0 )
		{
			if( normal_index == normal_count)
			{
				printf("OBJ_LOADER: normal out of bounds. Max %u got %u", normal_index, normal_count);  
			}
			else{
				vec3 normal;
				fscanf( fp, "%f %f %f",  &normal.x, &normal.y, &normal.z);
				normals[normal_index] = normal;
				normal_index++;
			}
		}
		else{}
		
		uint8_t end[1];
		while( end[0] != '\n') {
			fscanf( fp, "%c", end );
		}
		
	}
	
	rewind( fp );
	

	indice_index = 0;
	while( !(feof(fp)) )
	{
		char line[1];
		line[0] = 'a';
		int32_t read = fscanf(fp, "%1s", line);
		
		if( line[0] == 'f' )
		{
			
			uint32_t position[3];
			uint32_t texcoord[3];
			uint32_t normal[3];
			
			memset( position, (uint32_t)0, sizeof( position ));
			memset( texcoord, (uint32_t)0, sizeof( texcoord ));
			memset( normal, (uint32_t)0, sizeof( normal ));
			
			read = fscanf( fp, "%u/%u/%u %u/%u/%u %u/%u/%u", &position[0], &texcoord[0], &normal[0],   //Vertices 
															 &position[1], &texcoord[1], &normal[1],   //Texcoords
															 &position[2], &texcoord[2], &normal[2] ); //Normals	
																	 
			//printf("%u/%u/%u %u/%u/%u %u/%u/%u \n",   position[0], texcoord[0], normal[0],   //Vertices 
			//										   position[1], texcoord[1], normal[1],   //Texcoords
			//										   position[2], texcoord[2], normal[2] ); //Normals		
			{  

				indices[ indice_index ] 				= position[0] - 1;
				vertices[ position[0] - 1 ].position 	= positions[ position[0] - 1 ];
				vertices[ position[0] - 1 ].normal 		= normals[normal[0] - 1 ];
				indice_index++;

			}
			{	
				indices[ indice_index ] 				= position[1] - 1;
				vertices[ position[1] - 1 ].position 	= positions[ position[1] - 1 ];
				vertices[ position[1] - 1 ].normal 		= normals[normal[1] - 1 ];
				indice_index++;
				
			}
			{		
				indices[ indice_index ] 					= position[2] - 1;
				vertices[ position[2] - 1 ].position 		= positions[ position[2] - 1 ];
				vertices[ position[2] - 1 ].normal 			= normals[normal[2] - 1 ];
				indice_index++;
			
			}		
			
		}
		
		uint8_t end[1];
		while( end[0] != '\n') {
			fscanf( fp, "%c", end );
		}
		
	}
	
	
	result = true;

end:

	free( normals );
	free( positions );
	
	model->vertices 	= vertices;
	model->indices  	= indices;
	model->vertex_count = vertex_count;
	model->indice_count = indice_index;
	
	return ( result );
}




#endif //OBJ_LOADER_H
