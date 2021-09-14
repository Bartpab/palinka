#ifndef __STREAM_BUFFER_H__
#define __STREAM_BUFFER_H__


#include <string.h>

#include "../types.h"
#include "../allocator.h"

#include "core.h"

typedef struct {
    stream_t* stream;
    size_t capacity;   
    void* raw;
    size_t size;
    allocator_t allocator;
} wbuffer_t;

const wbuffer_t wbuffer_init = {0, 0, 0, 0, NO_ALLOCATOR};

bool wbuffer_create(wbuffer_t* buffer, stream_t* stream, size_t capacity, allocator_t* allocator);
void wbuffer_delete(wbuffer_t* buffer);
bool wbuffer_is_full(wbuffer_t* buffer);
void wbuffer_delete(wbuffer_t* buffer);
size_t wbuffer_write(wbuffer_t* buffer, const void* src, size_t len);
bool wbuffer_flush(wbuffer_t* buffer);

bool wbuffer_create(wbuffer_t* buffer, stream_t* stream, size_t capacity, allocator_t* allocator)
{
    void* raw = pmalloc(allocator, capacity);
    
    if(raw == NULL)
        return false;
   
    buffer->raw = raw;
    buffer->allocator = allocator->cpy(allocator);
    buffer->capacity = capacity;
    buffer->size = 0;
    buffer->stream = stream;
    
    return true;
}

void wbuffer_delete(wbuffer_t* buffer) 
{
    if(buffer->stream != NULL && buffer->raw != NULL && buffer->size > 0) 
        wbuffer_flush(buffer);

    buffer->size = 0;
    buffer->stream = NULL;

    if(buffer->raw != NULL) 
    {
        pfree(&buffer->allocator, buffer->raw);
        buffer->raw = NULL;
        allocator_delete(&buffer->allocator);
    }
}

bool wbuffer_is_full(wbuffer_t* buffer) 
{
    return buffer->size == buffer->capacity;
}


size_t wbuffer_write(wbuffer_t* buffer, const void* src, size_t len) 
{
    size_t left = buffer->capacity - buffer->size;
    size_t dlen = left > len ? len : left;
    
    if(dlen == 0)
        return 0;
    
    memcpy(buffer->raw + buffer->size, src, dlen);
    buffer->size += dlen;

    return dlen;
}


bool wbuffer_flush(wbuffer_t* buffer) 
{
    if(!stream_is_opened(buffer->stream))
        return false;

    if(buffer->size == 0)
        return true;
    
    size_t written = stream_write(buffer->stream, buffer->raw, buffer->size);
    
    // Couldn't write into the stream...
    if(written < 1)
        return false;

    size_t nsize = buffer->size - written;
    
    if(nsize > 0) memmove(buffer->raw, buffer->raw + written, nsize);

    buffer->size = nsize;

    return true;
}

typedef struct {
    stream_t* stream;
    size_t capacity;
    void* raw;
    size_t size;
    bool exhausted;
    allocator_t allocator;
} rbuffer_t;

const rbuffer_t rbuffer_init = {0, 0, 0, 0, 0, NO_ALLOCATOR};

bool rbuffer_is_exhausted(rbuffer_t* buffer) 
{
    return buffer->exhausted;
}

bool rbuffer_create(rbuffer_t* buffer, stream_t* stream, size_t capacity, allocator_t* allocator) 
{
    buffer->stream = stream;
    buffer->exhausted = false;
    buffer->allocator = allocator->cpy(allocator);
    buffer->size = 0;
    buffer->capacity = capacity;
    buffer->raw = pmalloc(allocator, capacity);
    
    if(!buffer->raw)
        return false;
    
    return true;
}

bool rbuffer_fetch(rbuffer_t* buffer) 
{
    if(rbuffer_is_exhausted(buffer) || !stream_is_opened(buffer->stream)) 
        return false;

    size_t size = stream_read(
        buffer->stream, 
        buffer->raw, 
        buffer->capacity
    );
    
    if (size < buffer->capacity) 
        buffer->exhausted = true;

    buffer->size = size;

    return true;
} 

void rbuffer_delete(rbuffer_t* buffer) 
{
    buffer->stream = NULL;

    if(buffer->raw != NULL) 
    {
        pfree(&buffer->allocator, buffer->raw);
        buffer->raw = NULL;
        allocator_delete(&buffer->allocator);
    }
}

typedef struct {
    void* base;
    size_t length;
} buffer_t;

const buffer_t buffer_init = {0, 0};

/**
 * \brief Read the whole stream and store its content in a dynamic buffer.
 * 
 * \return The buffer with all the content of the stream
 */
buffer_t stream_exhaust(stream_t* stream, size_t capacity, allocator_t* allocator)
{
    rbuffer_t rbuffer = rbuffer_init;
    rbuffer_create(&rbuffer, stream, capacity, allocator);
    return buffer_exhaust(&rbuffer, capacity);
}

buffer_t stream_exhaust_str(stream_t* stream, size_t capacity, allocator_t* allocator)
{
    rbuffer_t rbuffer = rbuffer_init;
    rbuffer_create(&rbuffer, stream, capacity, allocator);
    return buffer_exhaust_str(&rbuffer, capacity);
}

/**
 * \brief Read the whole content of the stream and store it in a buffer
 */
buffer_t buffer_exhaust(rbuffer_t* buffer, size_t capacity) 
{
    buffer_t tmp;
    void *base, *curr;
    size_t length, offset;
    
    base = malloc(capacity);
    length = 0;

    while(rbuffer_fetch(buffer)) 
    {
        offset = length;
        length += buffer->size;
        
        if(length >= capacity) 
        {
            while(length >= capacity) capacity <<= 1;
            base = realloc(base, capacity);
        }

        curr = (void*)(base + offset);
        memcpy(curr, buffer->raw, buffer->size);
    }

    tmp.base = base;
    tmp.length = length;
    
    return tmp;
}

/**
 * \brief Same as buffer_exhaust but add the '\0' character at the end.
 */
buffer_t buffer_exhaust_str(rbuffer_t* rbuffer, size_t initial)
{
    buffer_t buffer = buffer_exhaust(rbuffer, initial);
    buffer.base = realloc(buffer.base, buffer.length + 1);
    *(char*)(buffer.base + buffer.length) = 0;
    buffer.length++;
    return buffer;
}

void buffer_delete(buffer_t* buffer) 
{
    if(buffer->base != NULL)
        free(buffer->base);
    
    buffer->base = NULL;
}

#endif