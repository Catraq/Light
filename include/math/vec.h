#ifndef VEC_H
#define VEC_H

#include <math.h>

struct vec2 
{
	float x,y;
};

struct vec3
{
	float x,y,z;
};


struct vec4
{
	float x,y,z,w;
};



struct vec2 v2sub(struct vec2 lhs, struct vec2 rhs);
struct vec2 v2scl(struct vec2 lhs, float scale);

struct 	vec3 v3sub(struct vec3 lhs, struct vec3 rhs);
float 	v3dot(struct vec3 lhs, struct vec3 rhs);
struct 	vec3 v3add(struct vec3 lhs, struct vec3 rhs);
struct 	vec3 v3scl(struct vec3 lhs, float scale);
float 	v3len(struct vec3 lhs);

struct vec3 v3norm(struct vec3 rhs);

struct vec4 vec4_substract(struct vec4 lhs, struct vec4 rhs);
float vec4_length(struct vec4 lhs);


#endif //VEC3_H
