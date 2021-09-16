#ifndef __STRING_PROCESS_H__
#define __STRING_PROCESS_H__

#include "./core.h"
#include "./vector.h"
#include "./iterator.h"

bool string_join_char(string_t* s, string_iterator_t* it, const char c)
{
    buffer_t buffer;
    allocator_t allocator = GLOBAL_ALLOCATOR;

    bool first = true;

    if(!buffer_create(&buffer, 64, &allocator))
        return false;
    
    while(it->next(it)) 
    {   
        if(!first){
            if(!buffer_write_char(&buffer, c))
                return false;
        } else {
            first = false;
        }

        if(!buffer_write_string(&buffer, it->get(it)))
            return false;
    }

    buffer_move_to_string(s, &buffer);
    buffer_delete(&buffer);

    return true;
}

bool string_split_char(string_vector_t *vec, string_t* s, const char c)
{
    buffer_t buffer;
    string_t str;
    allocator_t allocator = GLOBAL_ALLOCATOR;

    if(!buffer_create(&buffer, 64, &allocator))
        return false;

    const char *it = string_raw(s);
    const char* limit = it + string_length(s);
    
    for(;it <= limit; it++) 
    {
        // Split !
        if(*it == c) {
            buffer_copy_to_string(&str, &buffer, &allocator);

            if(!string_vector_move_add(vec, &str))
                return false;

            buffer_reset(&buffer);

            while(*it == c) it++;
            it--;

        } else {
            buffer_write_char(&buffer, *it);
        }
    }
    
    if(buffer.length > 0) 
    {
        buffer_copy_to_string(&str, &buffer, &allocator);
        if(!string_vector_move_add(vec, &str))
            return false;        
    }

    buffer_delete(&buffer);
    string_delete(&str);
    
    return true;
}

#endif