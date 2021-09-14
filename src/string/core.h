#ifndef __STRING_CORE_H__
#define __STRING_CORE_H__

#include "../allocator.h"

typedef struct {
    char* base;
    size_t length;
    allocator_t allocator;
} string_t;

string_t str_init = {0, 0, NO_ALLOCATOR};

void string_delete(string_t * str) 
{
    if(str->base != NULL) 
    {
        pfree(&str->allocator, str->base);
        str->base = 0;
        allocator_delete(&str->allocator);
    }
}


#endif