#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "physic.h"



struct physic_body * physic_instance_bodies( physic *physic, physic_instance *instance )
{
}

void physic_instance_initialize(struct physic_instance *instance, struct vertex_buffer_handler *handler, uint32_t body_count)
{
	
	//Set data index in physic buffer. 
	instance->vb_handler = handler;
	instance->offset = 0;
	instance->count = body_count;
}



void physic_instance_simulate(struct physic_instance *instance, int instance_count, const float& deltatime)
{
	
}



