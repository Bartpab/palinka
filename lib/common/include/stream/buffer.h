#ifndef __STREAM_BUFFER_H__
#define __STREAM_BUFFER_H__


#include <string.h>

#include "../types.h"
#include "../allocator.h"
#include "../buffer.h"
#include "../string/core.h"

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
void wbuffer_destruct(wbuffer_t* buffer);
bool wbuffer_is_full(wbuffer_t* buffer);
void wbuffer_destruct(wbuffer_t* buffer);
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

void wbuffer_destruct(wbuffer_t* buffer) 
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

bool rbuffer_create(rbuffer_t* buffer, stream_t* stream, size_t capacity, allocator_t* allocator);
bool rbuffer_fetch(rbuffer_t* buffer);
bool rbuffer_is_exhausted(rbuffer_t* buffer);
void rbuffer_destruct(rbuffer_t* buffer);

buffer_t rbuffer_read_all(rbuffer_t* buffer, size_t capacity, allocator_t* allocator);
string_t rbuffer_read_all_str(rbuffer_t* rbuffer, size_t initial, allocator_t* allocator);

bool rbuffer_is_exhausted(rbuffer_t* buffer) 
{
    return buffer->exhausted;
}

bool rbuffer_create(rbuffer_t* buffer, stream_t* stream, size_t capacity, allocator_t* allocator) 
{
    buffer->stream = stream;
    buffer->exhausted = false;
    buffer->allocator = allocator_copy(allocator);
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

void rbuffer_destruct(rbuffer_t* buffer) 
{
    buffer->stream = NULL;

    if(buffer->raw != NULL) 
    {
        pfree(&buffer->allocator, buffer->raw);
        buffer->raw = NULL;
        allocator_delete(&buffer->allocator);
    }
}

/**
 * \brief Read the whole content of the stream and store it in a buffer
 */
buffer_t rbuffer_read_all(rbuffer_t* buffer, size_t capacity, allocator_t* allocator) 
{
    buffer_t tmp;

    tmp.allocator = allocator_copy(allocator);    
    tmp.base = pmalloc(allocator, capacity);
    tmp.length = 0;

    while(rbuffer_fetch(buffer)) 
    {
        if(!buffer_write(&tmp,  buffer->raw, buffer->size))
            return tmp;
    }
    
    return tmp;
}

/**
 * \brief Same as buffer_exhaust but add the '\0' character at the end.
 */
string_t rbuffer_read_all_str(rbuffer_t* rbuffer, size_t initial, allocator_t* allocator)
{
    string_t str = string_init;
    buffer_t buffer = rbuffer_read_all(rbuffer, initial, allocator);
    buffer_move_to_string(&str, &buffer);
    return str;
}

bool stream_exhaust(buffer_t* buffer, stream_t* stream, size_t capacity, allocator_t* allocator);
/**
 * \brief Read the whole stream and store its content in a dynamic buffer.
 * 
 * \return The buffer with all the content of the stream
 */
bool stream_exhaust(buffer_t* buff, stream_t* stream, size_t capacity, allocator_t* allocator)
{
    size_t length;    
    void* raw = pmalloc(allocator, capacity);
    
    if(raw == false)
        return false;

    for(length = stream_read(stream, &raw, capacity); length > 0; length = stream_read(stream, &raw, capacity))
    {
        if(!buffer_write(buff, raw, length))
        {
            pfree(allocator, raw);
            return false;
        }
    }

    pfree(allocator, raw);
    return true;
}

#endif