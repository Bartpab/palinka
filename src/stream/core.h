#ifndef __STREAM_CORE_H__
#define __STREAM_CORE_H__

#include <errno.h>
#include <stdio.h>

typedef struct {
    FILE* impl;
} stream_t;

stream_t stream_init = {0};

stream_t stream_create();
bool stream_open_file(const char* fp, const char* mode, stream_t* stream);
bool stream_is_opened(stream_t* stream);
void stream_close(stream_t* stream);

size_t stream_read(stream_t* stream, void* dest, size_t capacity);
size_t stream_write(stream_t* stream, const void* src, size_t size);

stream_t stream_create() 
{
    stream_t stream;
    stream.impl = 0;
    return stream;
}

bool stream_open_file(const char* fp, const char* mode, stream_t* stream)
{
    stream->impl = NULL;

    FILE* fd = fopen(fp, mode);
    
    if(fd == NULL)
        return false;

    stream->impl = fd;
    return true;
}

bool stream_is_opened(stream_t* stream) 
{
    return stream != NULL && stream->impl != NULL;
}

void stream_close(stream_t* stream)
{
    if(stream->impl == NULL)
        return;
    
    fclose(stream->impl);
    stream->impl = NULL;
}

size_t stream_read(stream_t* stream, void* dest, size_t capacity)
{
    return fread(dest, sizeof(char), capacity, stream->impl);
}

size_t stream_write(stream_t* stream, const void* src, size_t size) 
{
    return fwrite(src, sizeof(char), size, stream->impl);
}

#endif