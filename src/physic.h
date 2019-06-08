#ifndef PHYSIC_H
#define PHYSIC_H

#include "math/vec.h"
#include "math/mat4x4.h"



struct physic_body
{
	struct mat4x4 translation;
	struct vec3 velocity;
	struct vec3 position;
	struct vec3 rotation;
};

struct physic_instance
{
	uint32_t offset;
	uint32_t count;
	struct physic_body *body;
	struct vertex_buffer_handler *vb_handler;
};



struct physic_body * physic_instance_bodies(struct physic *physic, struct physic_instance *instance);

void physic_instance_initialize(struct physic *physic, struct physic_instance *instance, struct vertex_buffer_handler *handler, uint32_t body_count);
void physic_instance_simulate( physic_instance *instance, physic_body *bodies, const float& deltatime);

#endif //PHYSIC_H
