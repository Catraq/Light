//The point of this program is to compute the inertia of a implicit defined volume on a spherical domain

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//Radius of domain 
#define SPHERE_RADIUS 10.0f

#define SAMPLE_COUNT 100000000

struct vec3{
	float x;
	float y;
	float z;
};


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

struct vec3 v3cross(struct vec3 lhs, struct vec3 rhs)
{
	struct vec3 result;
	result.x = lhs.y*rhs.z - lhs.z*rhs.y;
	result.y = lhs.z*rhs.x - lhs.x*rhs.z;
	result.z = lhs.x*rhs.y - lhs.y*rhs.x;
	return result;
}


//Density of sphere with radius 1 and weight 1 kg
float sphere(struct vec3 c, float r, struct vec3 p)
{
	struct vec3 diff = v3sub(c, p);
	return (r*r - v3dot(diff, diff)) > 0.0f ? 1.0f/(4.0f/3.0f*3.14*1.0*1.0*1.0) : 0.0f;	
}


int main(int argv, char *args[])
{
	srand(time(NULL));

	struct vec3 center = {.x = 0.0f, .y = 0.0f, .z = 0.0f};
		
	float I_xx = 0,
	      I_yy = 0,
	      I_zz = 0,
	      I_xy = 0,
	      I_xz = 0,
	      I_zy = 0,
	      I_yz = 0;
	
	for(uint32_t i = 0; i < SAMPLE_COUNT; i++)
	{
		//Domain on sphere
		float x = (2.0f*(float)rand()/(float)RAND_MAX - 1.0f)*SPHERE_RADIUS;
		float y = (2.0f*(float)rand()/(float)RAND_MAX - 1.0f)*SPHERE_RADIUS;
		float z = (2.0f*(float)rand()/(float)RAND_MAX - 1.0f)*SPHERE_RADIUS;

		
		struct vec3 p = {.x = x, .y = y, .z = z};	

		float density = sphere(center, 1.0f, p);

		I_xx +=density*(y*y+z*z);
		I_yy +=density*(x*x+z*z);
		I_zz +=density*(x*x+y*y);

		I_yz +=density*y*z;
		I_xy +=density*x*y;
		I_xz +=density*x*z;

	}

	float V = 2.0f*2.0f*2.0f*SPHERE_RADIUS*SPHERE_RADIUS*SPHERE_RADIUS;  

	I_xx = I_xx*V/(float)SAMPLE_COUNT;
	I_zz = I_zz*V/(float)SAMPLE_COUNT;
	I_yy = I_yy*V/(float)SAMPLE_COUNT;


	I_xy = I_xy*V/(float)SAMPLE_COUNT;
	I_yz = I_yz*V/(float)SAMPLE_COUNT;
	I_xz = I_xz*V/(float)SAMPLE_COUNT;

	printf("I_xx=%f, I_yy=%f, I_zz=%f, I_xy=%f, I_yz=%f, I_xz=%f", I_xx, I_yy, I_zz, I_xy, I_yz, I_xz);

	return 0;
}
