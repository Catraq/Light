//The point of this program is to compute the inertia of a implicit defined volume on a spherical domain

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//Radius of domain 
#define BOX_HALF_WIDTH 10.0f

#define SAMPLE_COUNT 10000

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
	return (sqrtf(v3dot(p, p)) - r); 
}


int main(int argv, char *args[])
{
	srand(time(NULL));

	struct vec3 center = {.x = 0.0f, .y = 0.0f, .z = 0.0f};
	
	float mass = 1.0;

	uint32_t I_xx = 0,
	      I_yy = 0,
	      I_zz = 0,
	      I_xy = 0,
	      I_xz = 0,
	      I_zy = 0,
	      I_yz = 0;

		
	uint32_t inside = 0;	
	float V_sample = 2.0 * 2.0 * 2.0 * BOX_HALF_WIDTH*BOX_HALF_WIDTH*BOX_HALF_WIDTH;
	for(uint32_t i = 0; i < SAMPLE_COUNT; i++)
	{
		//Domain on sphere
		float x = (2.0f*(float)rand()/(float)RAND_MAX - 1.0f)*BOX_HALF_WIDTH;
		float y = (2.0f*(float)rand()/(float)RAND_MAX - 1.0f)*BOX_HALF_WIDTH;
		float z = (2.0f*(float)rand()/(float)RAND_MAX - 1.0f)*BOX_HALF_WIDTH;

		struct vec3 p = {.x = x, .y = y, .z = z};	
		inside += (sphere(center, 1.0f, p) <= 0);
	}

	float V = (float)inside/(float)SAMPLE_COUNT * V_sample;
	printf("Volume=%f, exact=%f \n", V, (float)4/(float)3 * M_PI );
	
	uint32_t sample = 0;
	for(uint32_t i = 0; i < SAMPLE_COUNT; i++)
	{

		//Domain on sphere
		float x = (2.0f*(float)rand()/(float)RAND_MAX - 1.0f)*BOX_HALF_WIDTH;
		float y = (2.0f*(float)rand()/(float)RAND_MAX - 1.0f)*BOX_HALF_WIDTH;
		float z = (2.0f*(float)rand()/(float)RAND_MAX - 1.0f)*BOX_HALF_WIDTH;
		
		struct vec3 p = {.x = x, .y = y, .z = z};	

		uint32_t in = (sphere(center, 1.0f, p) < 0) ? 1 : 0;

		I_xx += in*1000*(y*y+z*z);
		I_yy += in*1000*(x*x+z*z);
		I_zz += in*1000*(x*x+y*y);

		I_yz += in*1000*y*z;
		I_xy += in*1000*x*y;
		I_xz += in*1000*x*z;

	}


	float fI_xx = mass/V*I_xx*V_sample/((float)(1000*SAMPLE_COUNT));
	float fI_zz = mass/V* I_zz*V_sample/((float)(1000*SAMPLE_COUNT));
	float fI_yy = mass/V* I_yy*V_sample/((float)(1000*SAMPLE_COUNT));


	float fI_xy =mass/V * I_xy*V_sample/((float)1000*SAMPLE_COUNT);
	float fI_yz =mass/V * I_yz*V_sample/((float)1000*SAMPLE_COUNT);
	float fI_xz =mass/V * I_xz*V_sample/((float)1000*SAMPLE_COUNT);

	printf("I_xx=%f, I_yy=%f, I_zz=%f, I_xy=%f, I_yz=%f, I_xz=%f", fI_xx, fI_yy, fI_zz, fI_xy, fI_yz, fI_xz);

	return 0;
}
