#ifndef VEC_H
#define VEC_H

#include <math.h>

struct vec2 
{
	float x,y;
};


inline struct vec2 v2sub(struct vec2 lhs, struct vec2 rhs )
{
	struct vec2 result;
	result.x = lhs.x - rhs.x;
	result.y = lhs.y - rhs.y;
	return (result);
}

inline struct vec2 v2scl(struct vec2 lhs, float scale)
{
	struct vec2 result;
	result.x = lhs.x * scale;
	result.y = lhs.y * scale;
	return (result);
}

struct vec3
{
	float x,y,z;
};

inline struct vec3 v3sub(struct vec3 lhs, struct vec3 rhs)
{
	struct vec3 result;
	result.x = lhs.x - rhs.x;
	result.y = lhs.y - rhs.y;
	result.z = lhs.z - rhs.z;
	return (result);
}


inline struct vec3 v3add(struct vec3 lhs, struct vec3 rhs)
{
	struct vec3 result;
	result.x = lhs.x + rhs.x;
	result.y = lhs.y + rhs.y;
	result.z = lhs.z + rhs.z;
	return (result);
}


inline struct vec3 v3add(struct vec3 lhs, float rhs)
{
	struct vec3 result;
	result.x = lhs.x + rhs;
	result.y = lhs.y + rhs;
	result.z = lhs.z + rhs;
	return ( result );
}

inline struct vec3 v3scl(struct vec3 lhs, float scale)
{
	struct vec3 result;
	result.x = lhs.x * scale;
	result.y = lhs.y * scale;
	result.z = lhs.z * scale;
	return ( result );
}




inline float v3len(vec3 lhs)
{
	return sqrtf( (lhs.x * lhs.x) + (lhs.z * lhs.z) +(lhs.y * lhs.y)  );
}


struct vec4
{
	float x,y,z,w;
};

struct vec4 vec4_substract(struct vec4 lhs, struct vec4 rhs)
{
	struct vec4 result;
	result.x = lhs.x - rhs.x;
	result.y = lhs.y - rhs.y;
	result.z = lhs.z - rhs.z;
	result.w = lhs.w - rhs.w;
	return ( result );
}

float vec4_length(struct vec4 lhs)
{
	return sqrtf( (lhs.x * lhs.x) + (lhs.z * lhs.z) + (lhs.y * lhs.y) + (lhs.w * lhs.w)  );
}

#endif //VEC3_H
