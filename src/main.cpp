#include <time.h>
#include <stdio.h>
#include <stdint.h>


#include "scene.h"
#include "game.h"


int main(int args, char *argv[])
{
	int result;
	struct game game;
	struct scene_instance s_instance;

	result = platform_initialize();
	if(result < 0)
	{
		printf("Error: could not initialize platform.\n");
		exit(EXIT_FAILURE);
		/* Abort */
	}

	result = scene_initialize(&s_instance);	
    	if(result < 0)
	{
		printf("Error: could not initialize engine.\n");
		exit(EXIT_FAILURE);
	}
	

	result = game_create_instance(&game, &s_instance);
	if(result < 0)
	{
		printf("Error: could not initialize game.\n");
		exit(EXIT_FAILURE);
	}
	
	
	
	clock_t time = clock();
	physic_update(&s_instance.physic, 0.0f);
	while(!platform_exit())
    	{
		
		float deltatime = (float)(clock() - time)/(float)CLOCKS_PER_SEC;
		time = clock();

		
	


		scene_render(&s_instance, deltatime);
		game_update(&game, &s_instance, deltatime);
		platform_update();
    	}
	
	printf( "Exiting");
	platform_deinitialize();
		
	return (result);
}
