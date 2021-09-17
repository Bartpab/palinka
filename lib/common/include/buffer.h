#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stddef.h>
#include <string.h>

#include "allocator.h"

typedef struct {
    void* base;
    size_t capacity;
    size_t length;
    allocator_t allocator;
} buffer_t;

const buffer_t buffer_init = {0, 0};

buffer_t buffer(size_t capacity, allocator_t* allocator);
bool buffer_create(buffer_t* buffer, size_t capacity, allocator_t* allocator);
void buffer_reset(buffer_t* buffer);
bool buffer_pop_char(buffer_t* buffer);
bool buffer_copy(buffer_t* dest, const buffer_t* src, allocator_t* allocator);
bool buffer_write(buffer_t* buffer, const void* dest, size_t len);
bool buffer_write_char(buffer_t* buffer, const char c);
void buffer_delete(buffer_t* buffer);

static bool __buffer_inc_capacity(buffer_t* buffer, size_t new_capacity) {
    void* base = prealloc(&buffer->allocator, buffer->base, new_capacity);

    if(base == NULL)
        return false;

    buffer->base = base;
    buffer->capacity = new_capacity;

    return true;
}

buffer_t buffer(size_t capacity, allocator_t* allocator)
{
    buffer_t tmp;
    buffer_create(&tmp, capacity, allocator);
    return tmp;
}

bool buffer_create(buffer_t* buffer, size_t capacity, allocator_t* allocator)
{
    buffer->base = pmalloc(allocator, capacity);
    
    if(buffer->base == NULL)
        return false;

    buffer->capacity = capacity;
    buffer->length = 0;
    buffer->allocator = allocator_copy(allocator);
    return true;
}

void buffer_reset(buffer_t* buffer)
{
    buffer->length = 0;
}

bool buffer_pop_char(buffer_t* buffer)
{
    buffer->length -= sizeof(char);
    return true;
}

bool buffer_write(buffer_t* buffer, const void* dest, size_t len)
{
    void* curr;
    size_t offset = buffer->length;
    size_t capacity = buffer->capacity;

    if(buffer->length + len >= capacity) 
    {
        while(buffer->length + len >= capacity) capacity <<= 1;
        
        if(!__buffer_inc_capacity(buffer, capacity))
            return false;
    }
    
    buffer->length += len;
    curr = (void*)(buffer->base + offset);
    memcpy(curr, dest, len);
    
    return true;
}

bool buffer_write_char(buffer_t* buffer, const char c)
{
    return buffer_write(buffer, &c, sizeof(char));
}

void buffer_delete(buffer_t* buffer) 
{
    if(buffer->base != NULL)
    {
        pfree(&buffer->allocator, buffer->base);
        buffer->base = NULL;
        allocator_delete(&buffer->allocator);
    }

    *buffer = buffer_init;
}

bool buffer_copy(buffer_t* dest, const buffer_t* src, allocator_t* allocator)
{
    buffer_delete(dest);

    void* base = pmalloc(allocator, src->capacity);
    
    if(base == NULL)
        return false;
    
    memcpy(base, src->base, src->capacity);

    dest->base = base;
    dest->length = src->length;
    dest->capacity = src->capacity;
    dest->allocator = allocator_copy(allocator);

    return true;
}

#endif