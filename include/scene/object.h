#ifndef LIGHT_SCENE_OBJECT_H
#define LIGHT_SCENE_OBJECT_H

#include "math/vec.h"
#include "math/mat4x4.h"

/* 
 * Structures in thei file will be mirrored on GPU. 
 * They should comply with layout std430 as it 
 * is used in the shaders. 
 */


/*
 * mirrored in gpu. have to correspond with shader input. 
 */
struct light_scene_implicit_object
{
	uint32_t object_index;
	uint32_t padding[3];

	struct mat4x4 translation;
	struct mat4x4 translation_inv;

};

/*
 * mirrored in gpu. have to correspond with shader input. 
 */
struct light_scene_particle_emitter_normal
{
	struct vec3 position;
	float lifetime_expected;
	struct vec3 velocity_expected;
	float lifetime_variance;	
	struct vec3 velocity_variance;
	uint32_t emitter_offset;
	uint32_t emitter_count;
	uint32_t padding[3];
};

struct light_scene_implicit_collision
{
	struct vec3 position;
	float dummy_1;
	uint32_t a;
	uint32_t b;
	uint32_t dummy_2[2];
};



/*
 * mirrored in gpu. have to correspond with shader input. 
 */
struct light_scene_light_light_instance
{
	struct vec3 position;
	float padding1;
	struct vec3 color;
	float padding2;
};


#endif 
