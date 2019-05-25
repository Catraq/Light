#ifndef PHYSIC_H
#define PHYSIC_H

#include <stdint.h>
#include <stdio.h>

#include "math/mat4x4.h"
#include "vertex_buffer.h"


struct physic_body
{
	mat4x4 		translation;
	vec3 		velocity;
	vec3 		position;
	vec3 		rotation;
	//float 		mass;
};



bool physic_body_intersect( physic_body *a , physic_body *b)
{
	#if 0
	vec3 delta = vec3_substract( &a->position, &a->rotation );
	const float length = vec3_length( &delta );
	return (length < a->mass + b->mass); 
	#endif 
	return ( false );
}

struct physic_instance
{
	uint32_t 				offset;
	uint32_t 				count;
	
	vertex_buffer_handler   *vb_handler;
	physic_instance 	    *next;
};


struct physic
{
	uint32_t 				body_count;
	physic_body 	   	   *bodies;
	physic_instance 	   *head;
};


void physic_initialize( physic *physic )
{
	physic->bodies 	   = 0;
	physic->body_count = 0;
	physic->head 	   = 0;

}


 
void physic_deinitialize( physic *physic )
{
	free( physic->bodies );
	physic->body_count = 0;
}


physic_body * physic_instance_bodies( physic *physic, physic_instance *instance )
{
	return &physic->bodies[ instance->offset ];
}

void physic_instance_initialize( physic *physic, physic_instance *instance, vertex_buffer_handler *handler, uint32_t body_count )
{
	
	//Set data index in physic buffer. 
	instance->vb_handler            = handler;
	instance->offset 				= 0;
	instance->count 				= body_count;
	
	//Calc new body buffer size 
	uint32_t new_body_count = physic->body_count + body_count;
	
	//Allocate new physic body buffer  
	physic_body *new_bodies = (physic_body *)malloc( sizeof( physic_body ) * new_body_count );
	
	//Grab ptr to behind the instance 
	physic_body *new_bodies_dest_ptr = new_bodies + body_count;
	
	//prev data behind new 
	memcpy( new_bodies_dest_ptr, physic->bodies, sizeof( physic_body ) * physic->body_count  );
	
	//Set new data 
	free( physic->bodies );
	
	//Update struct 
	physic->bodies = new_bodies;
	physic->body_count = new_body_count;
	
	//Put instance first in the list 
	instance->next = physic->head;
	physic->head = instance;
	
	//push offsets in the linked list 
	physic_instance *curr = instance->next;
	while( curr )
	{
		curr->offset += body_count;
		curr = curr->next;
	}
	
}

void physic_instance_commit( physic *physic, physic_instance *instance, physic_body *bodies, uint32_t count)
{
	for( uint32_t i = 0; i < count; i++)
	{
		memcpy( &physic->bodies[ instance->offset + i], &bodies[i], sizeof( physic_body ) );
	}
}

void physic_instance_print( physic_instance *instance )
{
	printf("\n Instance: %p \n", instance ); 
	printf(" Bodies: %u \n", instance->count );
	printf(" Offset: %u \n", instance->offset );
}


void physic_instance_translate_body(physic_body *body)
{
	struct mat4x4 rotation = m4x4rote(body->rotation);
	struct mat4x4 position = m4x4trs(body->position);
	body->translation = m4x4mul(position, rotation);
}

void physic_instance_simulate( physic_instance *instance, physic_body *bodies, const float& deltatime )
{
	
	const uint32_t body_offset = instance->offset;
	const uint32_t body_count  = instance->count;
	const uint32_t body_end    = body_offset + body_count;
	
	
	
	//Simulate against itself
	for( uint32_t i = body_offset; i < body_end; i++)
	{
		physic_body * abody = &bodies[i];
		
		physic_instance_translate_body(abody);

		for( uint32_t j = i; j < body_end; j++)
		{
			physic_body * bbody = &bodies[j];
			const bool intersection = physic_body_intersect(abody, bbody);
			if( intersection )
			{
				printf( " Intersection! ");
			}
		}
	}
		
	
	
	//Simulate this against all other down the list 
	physic_instance *curr = instance->next;
	while( curr )
	{
	
		const uint32_t other_body_offset = instance->offset;
		const uint32_t other_body_count  = instance->count;
		const uint32_t other_body_end    = other_body_offset + other_body_count;
	
		for( uint32_t i = body_offset; i < body_end; i++)
		{
			physic_body * abody = &bodies[i];
			
			for( uint32_t j = other_body_offset; j < other_body_end; j++)
			{
				physic_body * bbody = &bodies[j];
				const bool intersection = physic_body_intersect(abody, bbody);
				if( intersection )
				{
					printf( " Intersection! ");
				}
			}
		}
		
		
		curr = curr->next;
	}
}


void physic_update( physic *physic, const float& deltatime )
{
	//For each 
	physic_instance *curr = physic->head;
	physic_body *bodies = physic->bodies;
	while( curr )
	{
		physic_instance_simulate( curr, bodies, deltatime );
		
		//Grb next 
		curr = curr->next;
	}
	
}


#endif //PHYSIC_H
