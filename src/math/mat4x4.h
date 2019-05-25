#ifndef MAT4X4_H
#define MAT4X4_H

#include <math.h>
#include <stdint.h> 

#include "vec.h"
#include "quaternion.h"

struct mat4x4
{
	float m[16];
};


struct mat4x4 m4x4id( )
{
	struct mat4x4 result;
	result.m[ 0 ] = 1.0f; result.m[ 4 ] = 0.0f; result.m[ 8 ] = 0.0f; result.m[ 12 ] = 0.0f;
	result.m[ 1 ] = 0.0f; result.m[ 5 ] = 1.0f; result.m[ 9 ] = 0.0f; result.m[ 13 ] = 0.0f;
	result.m[ 2 ] = 0.0f; result.m[ 6 ] = 0.0f; result.m[ 10 ] = 1.0f; result.m[ 14 ] = 0.0f;
	result.m[ 3 ] = 0.0f; result.m[ 7 ] = 0.0f; result.m[ 11 ] = 0.0f; result.m[ 15 ] = 1.0f;
	return (result);
}


struct mat4x4 m4x4mul(struct mat4x4 lhs, struct mat4x4 rhs) 
{
	struct mat4x4 result;
	for (uint8_t i = 0 ; i < 4 ; i++) {
		for (uint8_t j = 0 ; j < 4 ; j++) {
			result.m[i*4 + j] = rhs.m[i*4] * lhs.m[ j ]  +
							rhs.m[i*4 + 1] * lhs.m[1*4 + j] +
							rhs.m[i*4 + 2] * lhs.m[2*4 + j] +
							rhs.m[i*4 + 3] * lhs.m[3*4 + j];
		}
	}
	return(result);
}

struct mat4x4 m4x4trs(struct vec3 pos)
{
	struct mat4x4 result = m4x4id();

	result.m[12] = pos.x;
	result.m[13] = pos.y;
	result.m[14] = pos.z;

	return (result);
}

struct mat4x4 m4x4rotq(struct quaternion q)
{
	struct mat4x4 result = {};

	//Col 1
	result.m[0] = 1.0f - 2.0f*(q.y*q.y - q.z*q.z);
	result.m[1] = 2.0f*(q.x*q.y + q.w*q.z);
	result.m[2] = 2.0f*(q.x*q.z - q.w*q.y);
	result.m[3] = 0.0f; 
	
	//Col 2
	result.m[4] = 2.0f*(q.x*q.y - q.w*q.z);
	result.m[5] = 1.0f - 2.0f*(q.x*q.x - q.z*q.z);
	result.m[6] = 2.0f*(q.x*q.z - q.w*q.y);
	result.m[7] = 0.0f; 
	
	//Col 3
	result.m[8] = 2.0f*(q.z*q.x + q.w*q.y);
	result.m[9] = 2.0f*(q.z*q.y + q.w*q.x);
	result.m[10] = 1.0f - 2.0f*(q.x*q.x - q.y*q.y);
	result.m[11] = 0.0f; 
	
	//Col 4
	result.m[12] = 0.0f;
	result.m[13] = 0.0f;
	result.m[14] = 0.0f;
	result.m[15] = 1.0f;


	return(result);
}

struct mat4x4 m4x4rote(struct vec3 rotation)
{
	struct mat4x4 a, b, c;
	a = m4x4id();
	b = m4x4id();
	c = m4x4id();
	
	a.m[5] = cosf(rotation.x); a.m[9] =  -sinf(rotation.x); 
	a.m[6] = sinf(rotation.x); a.m[10] = cosf(rotation.x); 
	
	b.m[0] = cosf(rotation.y); b.m[8]  =  -sinf(rotation.y); 
	b.m[2] = sinf(rotation.y); b.m[10] =  cosf(rotation.y);

	c.m[0] = cosf(rotation.z); c.m[4] = -sinf(rotation.z); 
	c.m[1] = sinf(rotation.z); c.m[5] =  cosf(rotation.z); 

	mat4x4 r1 = m4x4mul(c, a);
	mat4x4 r2 = m4x4mul(r1 ,b);
	return(r2);
}

struct mat4x4 m4x4pers(float ratio, float fov, float near, float far)
{
	mat4x4 result;
	const float range = near - far;
	const float length = tanf( fov / 2.0f );
	
	for( uint32_t i = 0; i < 16; i++)
		result.m[i] = 0.0f;

	result.m[0]  = 1.0f / ( length * ratio );
	result.m[5]  = 1.0f / length;
	result.m[10] = ( - near - far ) / range;
	result.m[11] = 1.0f;	
	result.m[14] = 2.0f * far * near / range;
	
	return ( result );
}


#endif //MAT4X4_H
