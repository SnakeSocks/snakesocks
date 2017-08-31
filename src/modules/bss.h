#ifndef Q_INTERFACE_BSS_H
#define Q_INTERFACE_BSS_H

#include <stdint.h>
#include <stdlib.h>

typedef struct _binary_safe_string
{
    bool null_terminated; //If true, it's OK to either regard str as a null-terminated string, or as a binary_safe_string.
    uint32_t length; //length(empty_string) must be 0.
    char *str; //This string must be created by malloc/calloc, released by free. Last user must free it.
} binary_safe_string;

#endif
