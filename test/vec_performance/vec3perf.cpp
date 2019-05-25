#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define ARRAY_SIZE 1024*1024


struct vec3
{
	float x,y,z;
};

vec3 vec3_add( const vec3* lhs, const vec3* rhs )
{
	vec3 result;
	result.x = lhs->x + rhs->x;
	result.y = lhs->y + rhs->y;
	result.z = lhs->z + rhs->z;
	return ( result );
}

void vec3perf(vec3 *inputA, vec3 *inputB, vec3 *outputA)
{
	
	uint32_t i; 
	for( i = 0; i < ARRAY_SIZE; i++ )
	{
		inputA[i] = vec3{ 1,1,1 };
		inputB[i] = vec3{ 1,1,1 };
	}
  	
	for( i = 0; i < ARRAY_SIZE; i++ )
	{
		outputA[i] = vec3_add( &inputA[i], &inputB[i] );
	}
	
}


int main(int args, char *argv[])
{
	//No warnings
	args;
	argv;
	
	int result = 0;
	vec3 *inputA = (vec3 *)malloc( sizeof(vec3) * ARRAY_SIZE );
	vec3 *inputB = (vec3 *)malloc( sizeof(vec3) * ARRAY_SIZE );
	vec3 *outputA = (vec3 *)malloc( sizeof(vec3) * ARRAY_SIZE );
	
	clock_t t = clock();
	
	for( uint32_t i = 0; i < 100; i++)
	{
		vec3perf( inputA, inputB, outputA);
	}
	
	t = clock() - t;
	printf ("Time: %f \n",(((float)t)/CLOCKS_PER_SEC)/100.0f);
	
	free( inputA );
	free( inputB );
	free( outputA );
	
	return (result);
}

