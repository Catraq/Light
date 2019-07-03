#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define ARRAY_SIZE 1024*1024


struct vec3
{
	float x,y,z;
};

vec3 vec3_add_ptr(const vec3* lhs, const vec3* rhs )
{
	vec3 result;
	result.x = lhs->x + rhs->x;
	result.y = lhs->y + rhs->y;
	result.z = lhs->z + rhs->z;
	return ( result );
}

vec3 vec3_add_ref(const vec3 lhs, const vec3 rhs)
{
	vec3 result;
	result.x = lhs.x + rhs.x;
	result.y = lhs.y + rhs.y;
	result.z = lhs.z + rhs.z;
	return result;
}

void vec3_ptr(vec3 *inputA, vec3 *inputB, vec3 *output)
{
  	
	for(uint32_t i = 0; i < ARRAY_SIZE; i++)
	{
		output[i] = vec3_add_ptr(&inputA[i], &inputB[i]);
	}
	
}

void vec3_ref(vec3 *inputA, vec3 *inputB, vec3 *output)
{
 	
	for(uint32_t i = 0; i < ARRAY_SIZE; i++)
	{
		output[i] = vec3_add_ref(inputA[i], inputB[i]);
	}
	
}


int main(int args, char *argv[])
{
	/* Keep warnings away */
	args;
	argv;
	
	vec3 *inputA = (struct vec3 *)malloc(sizeof(struct vec3) * ARRAY_SIZE);
	vec3 *inputB = (struct vec3 *)malloc(sizeof(struct vec3) * ARRAY_SIZE);
	vec3 *output = (struct vec3 *)malloc(sizeof(struct vec3) * ARRAY_SIZE);
		
	for(uint32_t i = 0; i < ARRAY_SIZE; i++ )
	{
		inputA[i] = vec3{1,1,1};
		inputB[i] = vec3{1,1,1};
	}
 

	fprintf(stdout, "Vector add by pointer test. \n");	
	clock_t t = clock();
	for( uint32_t i = 0; i < 100; i++)
	{
		vec3_ptr(inputA, inputB, output);
	}
	
	t = clock() - t;
	printf ("Time: %f \n",(((float)t)/CLOCKS_PER_SEC)/100.0f);



	fprintf(stdout, "Vector add by ref test. \n");	
	t = clock();
	for( uint32_t i = 0; i < 100; i++)
	{
		vec3_ref(inputA, inputB, output);
	}
	
	t = clock() - t;
	printf ("Time: %f \n",(((float)t)/CLOCKS_PER_SEC)/100.0f);
	

	free(inputA);
	free(inputB);
	free(output);
	
	return 0;	
}

