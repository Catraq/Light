#ifndef VERTEX_BUFFER_MODEL_H_
#define VERTEX_BUFFER_MODEL_H_

#include "vertex_buffer.h"

int light_vertex_buffer_model_load(struct light_vertex_buffer *buffer, struct light_vertex_buffer_handler *handler, const char *model_path[], const int model_path_count);


int light_vertex_buffer_model_load_model(struct light_vertex_buffer *buffer, struct light_vertex_buffer_handler *handler, const char *filename);
int light_vertex_buffer_model_data_size(const char *model_str[], int model_str_count, uint32_t *vertex_size, uint32_t *indice_size);

#endif 

