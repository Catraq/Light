#ifndef MAT4X4_H
#define MAT4X4_H


#include "math/vec.h"
#include "math/quaternion.h"

struct mat4x4
{
	float m[16];
};


struct mat4x4 m4x4id(void);
struct mat4x4 m4x4mul(struct mat4x4 lhs, struct mat4x4 rhs);
struct mat4x4 m4x4trs(struct vec3 pos);
struct mat4x4 m4x4scl(struct vec3 scale);
struct mat4x4 m4x4rotq(struct quaternion q);
struct mat4x4 m4x4rote(struct vec3 rotation);
struct mat4x4 m4x4pers(float ratio, float fov, float near, float far);
struct mat4x4 m4x4inv(struct mat4x4 *target, int *result);

#endif 
