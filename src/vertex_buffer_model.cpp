#include <stdio.h>

#include "vertex_buffer_model.h"

int vertex_buffer_model_load(struct vertex_buffer *buffer, struct vertex_buffer_handler *handler, const char *model_path[], const int model_path_count)
{
	int result = 0;

	uint32_t indice_total_size = 0;
       	uint32_t vertex_total_size = 0;

	result = vertex_buffer_model_data_size(model_path, model_path_count, &vertex_total_size, &indice_total_size);
	if(result < 0){
		return -1;	
	}

	/* TODO: Could fail. Should check.  */
	vertex_buffer_initialize(buffer, vertex_total_size, indice_total_size);


	
	result = 0;
	for(int i = 0; i < model_path_count; i++){
		int loaded = vertex_buffer_model_load_model(buffer, &handler[i], model_path[i]);
		if(!loaded){
			fprintf(stderr, "%s:%s: Failed to open file: %s \n", __FILE__, __LINE__, model_path[i]);
			result = -1;
		}
	}
	
	return result;
}


int vertex_buffer_model_load_model(struct vertex_buffer *buffer, struct vertex_buffer_handler *handler, const char *filename)
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



int vertex_buffer_model_data_size(const char *model_str[], int model_str_count, uint32_t *vertex_size, uint32_t *indice_size)
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


