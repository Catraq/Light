#ifndef PHYSIC_H
#define PHYSIC_H

#include "math/vec.h"
#include "math/mat4x4.h"


struct physic_instance
{
	uint32_t offset;
	uint32_t count;
	
	struct vertex_buffer_handler *vb_handler;
	struct physic_instance *next;
};


struct physic
{
	uint32_t body_count;
	struct physic_body *bodies;
	struct physic_instance *head;
};

struct physic_body
{
	struct mat4x4 translation;
	struct vec3 velocity;
	struct vec3 position;
	struct vec3 rotation;
};



bool physic_body_intersect(struct physic_body *a, struct physic_body *b);
void physic_initialize(struct physic *physic);
void physic_deinitialize(struct physic *physic);
struct physic_body * physic_instance_bodies(struct physic *physic, struct physic_instance *instance);
void physic_instance_initialize(struct physic *physic, struct physic_instance *instance, struct vertex_buffer_handler *handler, uint32_t body_count);
void physic_instance_commit(struct physic *physic, struct physic_instance *instance, struct physic_body *bodies, uint32_t count);
void physic_instance_print( physic_instance *instance);
void physic_instance_translate_body(physic_body *body);
void physic_instance_simulate( physic_instance *instance, physic_body *bodies, const float& deltatime);
void physic_update(physic *physic, const float& deltatime);

#endif //PHYSIC_H
