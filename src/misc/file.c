#include "misc/file.h"


size_t light_file_read_buffer(const char *file, uint8_t *buffer, size_t buffer_size)
{
	FILE *fp = fopen(file, "r");
	if(fp == NULL){
		fprintf(stderr, "Could not open file %s \n", file);
		return 0;
	}
	
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	rewind(fp);

	if(size > buffer_size){
		fprintf(stderr, "file content of %s (%lu bytes)  greater than buffer size(%lu bytes). \n", file, size, buffer_size);
		return 0;
	}

	size_t read = fread(buffer, 1, size, fp);
	if(read != size){
		fprintf(stderr, "Expected to read (%lu bytes) of %s, but got (%lu bytes). \n", size, file, buffer_size);
		return 0;
	}


	return read;

}
