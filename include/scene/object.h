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


/*
 * mirrored in gpu. have to correspond with shader input. 
 */
struct light_scene_implicit_sphere_instance
{
	struct vec3 color;

	float radius;
};


/*
 * mirrored in gpu. have to correspond with shader input. 
 */
struct light_scene_implicit_cylinder_instance
{
	struct vec3 color;

	/* radius of each cylinder  */
	float radius; 

	/* height of cylinder */
	float height;
	
	/* Required padding */
	float dummy[3];
};

/*
 * mirrored in gpu. have to correspond with shader input. 
 */
struct light_scene_implicit_box_instance
{
	struct vec3 color;

	float padding1;

	/* box dimension */	
	struct vec3 dimension;

	float padding2;
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
