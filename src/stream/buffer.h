#ifndef __STREAM_BUFFER_H__
#define __STREAM_BUFFER_H__

#include <stdio.h>

#include "../types.h"
#include "../allocator.h"

typedef struct {
    FILE* stream;
    const size_t max_len;
    size_t buf_len;
    void* raw;
    bool exhausted;
    allocator_t allocator;
} buffer_t;

typedef struct buffer_chunk_t {
    void* raw;
    size_t len;
}

bool buffer_from_file(const char* fp, char* mode, buffer_t* buffer, size_t len, int* error) 
{
    FILE* stream = fopen(fp, mode);
    
    if(fp == NULL)
        return false;
    
    *buffer = buffer_create(stream, len, &GLOBAL_ALLOCATOR);

    return true;
}

buffer_t buffer_create(FILE* stream, size_t len, allocator_t* allocator) 
{
    void* raw = pmalloc(allocator, len);
    allocator_t callocator = allocator->cpy(allocator);
    buffer_t buffer = {stream, len, 0, raw, false, callocator};
    return buffer;
}

bool buffer_read(buffer_t* buffer, buffer_chunk_t* chunk) 
{
    if(buffer->exhausted) 
    {
        chunk->raw = 0, chunk->len = 0;

        if(buffer->stream != NULL) 
        {
            fclose(buffer->stream);
            buffer->stream = NULL;
        }
        
        return false;
    }

    size_t read = fread(buffer->raw, sizeof(char), buffer->max_len, buffer->stream);
    
    chunk->len = read;
    chunk->raw = buffer->raw;

    if (read < buffer->max_len) 
    {
        buffer->exhausted = true;
    }

    return true;
} 

void buffer_delete(buffer_t* buffer) {
    if(buffer->stream != NULL) 
    {
        fclose(buffer->stream);
        buffer->stream = NULL;
    }

    if(buffer->raw != NULL) 
    {
        pfree(&buffer->allocator, buffer->raw);
        buffer->raw = NULL;
        buffer->allocator.del(&buffer->allocator);
    }
}

#endif