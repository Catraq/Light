#ifndef LIGHT_SCENE_OBJECT_H
#define LIGHT_SCENE_OBJECT_H

/* 
 * Structures in thei file will be mirrored on GPU. 
 * They should comply with layout std430 as it 
 * is used in the shaders. 
 */

/* Note that the first object/node is for left 
 * and secound for right 
 */

enum
{
	/* Used for ignoring index of object */
	LIGHT_SCENE_IMPLCIT_OBJECT_NONE 		= 0xFFFFFFFF,
	LIGHT_SCENE_IMPLICIT_OBJECT_OBJECT_UNION 	= 0,
	LIGHT_SCENE_IMPLICIT_OBJECT_NODE_UNION 		= 1,
	LIGHT_SCENE_IMPLICIT_NODE_NODE_UNION 		= 2
};

struct light_scene_implcit_object_instance
{
	uint32_t levels;

	/* 
	 * What kind of operation that should be performed 
	 * between the different nodes/objects 
	 */	
	uint32_t index_type;

	/* Set the index of either the node or object. 
	 * If the index is MAX_UINT32 then it is ignored 
	 */
	uint32_t index_left;
	uint32_t index_right;


	struct mat4x4 translation;
	struct mat4x4 translation_inv;
};

struct light_scene_implicit_object_node
{
	uint32_t padding;
	/* 
	 * What kind of operation that should be performed 
	 * between the different nodes/objects 
	 */	
	uint32_t index_type;

	/* Set the index of either the node or object. 
	 * If the index is MAX_UINT32 then it is ignored 
	 */
	uint32_t index_left;
	uint32_t index_right;

	struct mat4x4 translation;
	struct mat4x4 translation_inv;

};

/*
 * mirrored in gpu. have to correspond with shader input. 
 * adjust vertex attribute location if changed. 
 */
struct light_scene_implicit_sphere_instance
{
	struct mat4x4 translation;

	struct mat4x4 translation_inv;

	struct vec3 color;

	float radius;
};


/*
 * mirrored in gpu. have to correspond with shader input. 
 * adjust vertex attribute location if changed. 
 */
struct light_scene_implicit_cylinder_instance
{
	struct mat4x4 translation;

	struct mat4x4 translation_inv;

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
 * adjust vertex attribute location if changed. 
 */
struct light_scene_implicit_box_instance
{
	struct mat4x4 translation;
	
	struct mat4x4 translation_inv;

	struct vec3 color;

	float padding1;

	/* box dimension */	
	struct vec3 dimension;

	float padding2;
};


struct light_scene_light_light_instance
{
	struct vec3 position;
	float padding1;
	struct vec3 color;
	float padding2;
};


#endif 
