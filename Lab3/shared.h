#ifndef SHARED_H
#define SHARED_H

#include <stddef.h>

#define BUFFER_SIZE 1024

typedef struct
{
    size_t len;
    int end_of_line;
    int finished;
    char data[BUFFER_SIZE];
} Shared_data;

#endif