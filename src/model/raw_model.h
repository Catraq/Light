#ifndef RAW_MODEL_H
#define RAW_MODEL_H

#include <stdio.h>
#include "../math/vec.h"


struct vertex
{
	struct vec3 position;
	struct vec3 normal;
};

struct raw_model 
{
	uint32_t vertex_count;
	uint32_t indice_count;
	
};


struct vertex *raw_model_vertices(struct raw_model * model, uint32_t * count);
uint32_t *raw_model_indices(struct raw_model * model, uint32_t *count);
void raw_model_normals_compute(void);
void raw_model_release(struct raw_model * model);
struct raw_model * raw_model_load(FILE *fp);
uint32_t raw_model_save(FILE *fp, struct raw_model * model);

#endif //RAW_MODEL_H
