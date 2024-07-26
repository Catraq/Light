#ifndef LIGHT_CONFIG_H
#define LIGHT_CONFIG_H

/* Config file with the user defined sizes of different attributes in the project */

/* Max size of a shader file. Used for buffer init on the heap */
#define LIGHT_SHADER_FILE_MAX_SIZE 8192*2

/* Maximum number of implicit functions that can be loaded from shader files */
#define LIGHT_IMPLICIT_FUNCTION_MAX_COUNT 8


#define LIGHT_IMPLICIT_PHYSIC_FUNCTION_SIZE 8192*4

/* Number of samples used to calculate the volume and inerita of the implicit function */
#define LIGHT_SCENE_IMPLICIT_INERITA_SAMPLE_COUNT 256

#endif 
