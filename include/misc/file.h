#ifndef MISC_FILE_H
#define MISC_FILE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


size_t light_file_read_buffer(const char *file, uint8_t *buffer, size_t buffer_size);

#endif 
