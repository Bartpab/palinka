#ifndef __COMMON_STRING_IT_H__
#define __COMMON_STRING_IT_H__

#include "../container/iterator.h"
#include "../buffer.h"
#include "./core.h"

typedef struct string_iterator_t {
    DECL_ITERATOR(struct string_iterator_t, string_t)
} string_iterator_t;

bool string_concat_it(string_t* dest, string_iterator_t* it, allocator_t* allocator);

// IMPL //

bool string_concat_it(string_t* dest, string_iterator_t* it, allocator_t* allocator)
{
    buffer_t buffer;
    string_destruct(dest);
   
    if(!buffer_create(&buffer, 2, allocator))
        return false;

    while(it->next(it) == true) 
    {
        if(false == buffer_write_string(&buffer, it->get(it)))
        {
            buffer_destruct(&buffer);
            return false;
        }
    }

    buffer_move_to_string(dest, &buffer);
    buffer_destruct(&buffer);
    
    return true;
}

#endif