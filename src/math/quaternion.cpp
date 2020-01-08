#include "math/quaternion.h"

#include <math.h>

float qlen(struct quaternion q)
{
	return sqrtf(q.x*q.x +q.y*q.y +q.z*q.z +q.w*q.w); 
}

struct quaternion qnorm(struct quaternion lhs)
{
	float len = qlen(lhs);
	struct quaternion result = {
			lhs.x/len,
			lhs.y/len,
			lhs.z/len,
			lhs.w/len
	};
	
	return result;
}

struct quaternion qangle(struct vec3 axis, float angle) 
{	
	float s = sinf(angle)/2.0f;

	struct quaternion result = {
		axis.x * s,
		axis.y * s,
		axis.z * s,
		cosf(angle/2.0f)
	};
	
	return result;
}

struct quaternion qmul(struct quaternion lhs, struct quaternion rhs) 
{	
	struct quaternion result = {};
	
	result.w = lhs.w*rhs.w - (lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z);

	//Note:		 Wa*B + Wb*A + cross a b
	result.x = lhs.w*rhs.x + rhs.w*lhs.x + rhs.y*lhs.z - rhs.z*lhs.y;
	result.y = lhs.w*rhs.y + rhs.w*lhs.y + rhs.z*lhs.x - rhs.x*lhs.z;
	result.z = lhs.w*rhs.z + rhs.w*lhs.z + rhs.x*lhs.y - rhs.y*lhs.x;

	return result;
}

