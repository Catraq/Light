#ifndef QUATERNION_H_
#define QUATERNION_H_

#include "math/vec.h"

struct quaternion
{
	float x;
	float y;
	float z;
	float w;
};


float qlen(struct quaternion q);
struct quaternion qnorm(struct quaternion lhs);
struct quaternion qangle(struct vec3 axis, float angle);
struct quaternion qmul(struct quaternion lhs, struct quaternion rhs);

#endif 




