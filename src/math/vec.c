#include "math/vec.h"

#include <math.h>



struct vec2 v2sub(struct vec2 lhs, struct vec2 rhs )
{
	struct vec2 result;
	result.x = lhs.x - rhs.x;
	result.y = lhs.y - rhs.y;
	return (result);
}

struct vec2 v2scl(struct vec2 lhs, float scale)
{
	struct vec2 result;
	result.x = lhs.x * scale;
	result.y = lhs.y * scale;
	return (result);
}

float v3dot(struct vec3 lhs, struct vec3 rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

struct vec3 v3sub(struct vec3 lhs, struct vec3 rhs)
{
	struct vec3 result;
	result.x = lhs.x - rhs.x;
	result.y = lhs.y - rhs.y;
	result.z = lhs.z - rhs.z;
	return (result);
}


struct vec3 v3add(struct vec3 lhs, struct vec3 rhs)
{
	struct vec3 result;
	result.x = lhs.x + rhs.x;
	result.y = lhs.y + rhs.y;
	result.z = lhs.z + rhs.z;
	return (result);
}


struct vec3 v3scl(struct vec3 lhs, float scale)
{
	struct vec3 result;
	result.x = lhs.x * scale;
	result.y = lhs.y * scale;
	result.z = lhs.z * scale;
	return ( result );
}




float v3len(struct vec3 lhs)
{
	return sqrtf( (lhs.x * lhs.x) + (lhs.z * lhs.z) +(lhs.y * lhs.y)  );
}

struct vec3 v3norm(struct vec3 rhs)
{
	float len = v3len(rhs);
	return v3scl(rhs, 1.0f/len);
}


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

