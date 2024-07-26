#include "math/mat4x4.h"

#include <math.h>
#include <stdint.h> 

struct mat3x3 m3x3id(void)
{
	struct mat3x3 result = {};
	result.m[ 0 ] = 1.0f; result.m[ 3 ] = 0.0f; result.m[ 6 ] = 0.0f;
	result.m[ 1 ] = 0.0f; result.m[ 4 ] = 1.0f; result.m[ 7 ] = 0.0f;
	result.m[ 2 ] = 0.0f; result.m[ 5 ] = 0.0f; result.m[ 8 ] = 1.0f;
	return (result);
}

struct vec3 m3x3mulv3(struct mat3x3 lhs, struct vec3 rhs)
{
	struct vec3 result = {};
	result.x = lhs.m[0] * rhs.x + lhs.m[3] * rhs.y + lhs.m[6] * rhs.z;
	result.y = lhs.m[1] * rhs.x + lhs.m[4] * rhs.y + lhs.m[7] * rhs.z;
	result.z = lhs.m[2] * rhs.x + lhs.m[5] * rhs.y + lhs.m[8] * rhs.z;
	return(result);
}

struct mat3x3 m3x3mul(struct mat3x3 lhs, struct mat3x3 rhs) 
{
	struct mat3x3 result;
	for (uint8_t i = 0 ; i < 3 ; i++) {
		for (uint8_t j = 0 ; j < 3 ; j++) {
			result.m[i*3 + j] = rhs.m[i*3] * lhs.m[ j ]  +
				rhs.m[i*3 + 1] * lhs.m[1*3 + j] +
				rhs.m[i*3 + 2] * lhs.m[2*3 + j];
		}
	}
	return(result);
}


struct mat3x3 m3x3inv(struct mat3x3 target, int *result)
{
	float det = target.m[0] * (target.m[4]*target.m[8] - target.m[5]*target.m[7])
		  - target.m[1] * (target.m[3]*target.m[8] - target.m[5]*target.m[6])
		  + target.m[2] * (target.m[3]*target.m[7] - target.m[4]*target.m[6]);
	
	struct mat3x3 inverse = {};

	if(det == 0.0f){
		*result = -1;
		return inverse;
	}

	inverse.m[0] = 1/det * (target.m[4]*target.m[8] - target.m[7]*target.m[5]);
	inverse.m[1] = 1/det * (target.m[7]*target.m[2] - target.m[1]*target.m[8]);
	inverse.m[2] = 1/det * (target.m[1]*target.m[5] - target.m[4]*target.m[2]);
	inverse.m[3] = 1/det * (target.m[6]*target.m[5] - target.m[3]*target.m[8]);
	inverse.m[4] = 1/det * (target.m[0]*target.m[8] - target.m[6]*target.m[2]);
	inverse.m[5] = 1/det * (target.m[3]*target.m[2] - target.m[0]*target.m[5]);
	inverse.m[6] = 1/det * (target.m[3]*target.m[7] - target.m[6]*target.m[4]);
	inverse.m[7] = 1/det * (target.m[6]*target.m[1] - target.m[0]*target.m[7]);
	inverse.m[8] = 1/det * (target.m[0]*target.m[4] - target.m[3]*target.m[1]);

		
        *result = 0;
	return inverse;	
}

struct mat4x4 m4x4id(void)
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

struct vec4 m4x4mulv4(struct mat4x4 lhs, struct vec4 rhs) 
{
	struct vec4 result = {};
	result.x = lhs.m[0] * rhs.x + lhs.m[4] * rhs.y + lhs.m[8] * rhs.z + lhs.m[12] * rhs.w;
	result.y = lhs.m[1] * rhs.x + lhs.m[5] * rhs.y + lhs.m[9] * rhs.z + lhs.m[13] * rhs.w;
	result.z = lhs.m[2] * rhs.x + lhs.m[6] * rhs.y + lhs.m[10] * rhs.z + lhs.m[14] * rhs.w;
	result.w = lhs.m[3] * rhs.x + lhs.m[7] * rhs.y + lhs.m[11] * rhs.z + lhs.m[15] * rhs.w;
	return(result);
}

struct mat4x4 m4x4trs(struct vec3 pos)
{
	struct mat4x4 result = m4x4id();

	result.m[12] = pos.x;
	result.m[13] = pos.y;
	result.m[14] = pos.z;

	return result;
}
struct mat4x4 m4x4scl(struct vec3 scale)
{
	struct mat4x4 result = m4x4id();

	result.m[0] = scale.x;
	result.m[5] = scale.y;
	result.m[10] = scale.z;
	return result;
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


	return result;
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

	struct mat4x4 r1 = m4x4mul(c, a);
	struct mat4x4 r2 = m4x4mul(r1 ,b);
	return r2;
}

struct mat4x4 m4x4pers(float ratio, float fov, float near, float far)
{
	struct mat4x4 result;
	const float range = near - far;
	const float length = tanf(fov / 2.0f);

	for( uint32_t i = 0; i < 16; i++)
		result.m[i] = 0.0f;

	result.m[0]  = 1.0f / ( length * ratio );
	result.m[5]  = 1.0f / length;
	result.m[10] = (near + far) / range;
	result.m[11] = 1.0f;	
	result.m[14] = -2.0f * far * near / range;

	return result;
}

/* Compute 4x4 matrix inverse. Code from stachoverflow */
struct mat4x4 m4x4inv(struct mat4x4 *target, int *result)
{
	struct mat4x4 inv;
	int i;

	inv.m[0] = target->m[5]  * target->m[10] * target->m[15] - 
		target->m[5]  * target->m[11] * target->m[14] - 
		target->m[9]  * target->m[6]  * target->m[15] + 
		target->m[9]  * target->m[7]  * target->m[14] +
		target->m[13] * target->m[6]  * target->m[11] - 
		target->m[13] * target->m[7]  * target->m[10];

	inv.m[4] = -target->m[4]  * target->m[10] * target->m[15] + 
		target->m[4]  * target->m[11] * target->m[14] + 
		target->m[8]  * target->m[6]  * target->m[15] - 
		target->m[8]  * target->m[7]  * target->m[14] - 
		target->m[12] * target->m[6]  * target->m[11] + 
		target->m[12] * target->m[7]  * target->m[10];

	inv.m[8] = target->m[4]  * target->m[9] * target->m[15] - 
		target->m[4]  * target->m[11] * target->m[13] - 
		target->m[8]  * target->m[5] * target->m[15] + 
		target->m[8]  * target->m[7] * target->m[13] + 
		target->m[12] * target->m[5] * target->m[11] - 
		target->m[12] * target->m[7] * target->m[9];

	inv.m[12] = -target->m[4]  * target->m[9] * target->m[14] + 
		target->m[4]  * target->m[10] * target->m[13] +
		target->m[8]  * target->m[5] * target->m[14] - 
		target->m[8]  * target->m[6] * target->m[13] - 
		target->m[12] * target->m[5] * target->m[10] + 
		target->m[12] * target->m[6] * target->m[9];

	inv.m[1] = -target->m[1]  * target->m[10] * target->m[15] + 
		target->m[1]  * target->m[11] * target->m[14] + 
		target->m[9]  * target->m[2] * target->m[15] - 
		target->m[9]  * target->m[3] * target->m[14] - 
		target->m[13] * target->m[2] * target->m[11] + 
		target->m[13] * target->m[3] * target->m[10];

	inv.m[5] = target->m[0]  * target->m[10] * target->m[15] - 
		target->m[0]  * target->m[11] * target->m[14] - 
		target->m[8]  * target->m[2] * target->m[15] + 
		target->m[8]  * target->m[3] * target->m[14] + 
		target->m[12] * target->m[2] * target->m[11] - 
		target->m[12] * target->m[3] * target->m[10];

	inv.m[9] = -target->m[0]  * target->m[9] * target->m[15] + 
		target->m[0]  * target->m[11] * target->m[13] + 
		target->m[8]  * target->m[1] * target->m[15] - 
		target->m[8]  * target->m[3] * target->m[13] - 
		target->m[12] * target->m[1] * target->m[11] + 
		target->m[12] * target->m[3] * target->m[9];

	inv.m[13] = target->m[0]  * target->m[9] * target->m[14] - 
		target->m[0]  * target->m[10] * target->m[13] - 
		target->m[8]  * target->m[1] * target->m[14] + 
		target->m[8]  * target->m[2] * target->m[13] + 
		target->m[12] * target->m[1] * target->m[10] - 
		target->m[12] * target->m[2] * target->m[9];

	inv.m[2] = target->m[1]  * target->m[6] * target->m[15] - 
		target->m[1]  * target->m[7] * target->m[14] - 
		target->m[5]  * target->m[2] * target->m[15] + 
		target->m[5]  * target->m[3] * target->m[14] + 
		target->m[13] * target->m[2] * target->m[7] - 
		target->m[13] * target->m[3] * target->m[6];

	inv.m[6] = -target->m[0]  * target->m[6] * target->m[15] + 
		target->m[0]  * target->m[7] * target->m[14] + 
		target->m[4]  * target->m[2] * target->m[15] - 
		target->m[4]  * target->m[3] * target->m[14] - 
		target->m[12] * target->m[2] * target->m[7] + 
		target->m[12] * target->m[3] * target->m[6];

	inv.m[10] = target->m[0]  * target->m[5] * target->m[15] - 
		target->m[0]  * target->m[7] * target->m[13] - 
		target->m[4]  * target->m[1] * target->m[15] + 
		target->m[4]  * target->m[3] * target->m[13] + 
		target->m[12] * target->m[1] * target->m[7] - 
		target->m[12] * target->m[3] * target->m[5];

	inv.m[14] = -target->m[0]  * target->m[5] * target->m[14] + 
		target->m[0]  * target->m[6] * target->m[13] + 
		target->m[4]  * target->m[1] * target->m[14] - 
		target->m[4]  * target->m[2] * target->m[13] - 
		target->m[12] * target->m[1] * target->m[6] + 
		target->m[12] * target->m[2] * target->m[5];

	inv.m[3] = -target->m[1] * target->m[6] * target->m[11] + 
		target->m[1] * target->m[7] * target->m[10] + 
		target->m[5] * target->m[2] * target->m[11] - 
		target->m[5] * target->m[3] * target->m[10] - 
		target->m[9] * target->m[2] * target->m[7] + 
		target->m[9] * target->m[3] * target->m[6];

	inv.m[7] = target->m[0] * target->m[6] * target->m[11] - 
		target->m[0] * target->m[7] * target->m[10] - 
		target->m[4] * target->m[2] * target->m[11] + 
		target->m[4] * target->m[3] * target->m[10] + 
		target->m[8] * target->m[2] * target->m[7] - 
		target->m[8] * target->m[3] * target->m[6];

	inv.m[11] = -target->m[0] * target->m[5] * target->m[11] + 
		target->m[0] * target->m[7] * target->m[9] + 
		target->m[4] * target->m[1] * target->m[11] - 
		target->m[4] * target->m[3] * target->m[9] - 
		target->m[8] * target->m[1] * target->m[7] + 
		target->m[8] * target->m[3] * target->m[5];

	inv.m[15] = target->m[0] * target->m[5] * target->m[10] - 
		target->m[0] * target->m[6] * target->m[9] - 
		target->m[4] * target->m[1] * target->m[10] + 
		target->m[4] * target->m[2] * target->m[9] + 
		target->m[8] * target->m[1] * target->m[6] - 
		target->m[8] * target->m[2] * target->m[5];

	float det = target->m[0] * inv.m[0] + target->m[1] * inv.m[4] + target->m[2] * inv.m[8] + target->m[3] * inv.m[12];

	if (det == 0){
		*result = -1;
		return inv;
	}

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
		inv.m[i] = inv.m[i] * det;

	*result = 0;
	return inv;
}



