#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include <stdlib.h> 
#include "types.h"

typedef enum {
  NOP,
  DEFAULT
} allocator_type_t;

typedef struct allocator_t{
  char type;

  // vTable
  struct allocator_t (*cpy)(struct allocator_t* allocator);
  void (*del)(struct allocator_t* allocator);
} allocator_t;

void* prealloc(allocator_t* allocator, void* src, size_t new_size)
{
  if (allocator->type == DEFAULT) 
    return realloc(src, new_size);

  return NULL;
}

void* pmalloc(allocator_t* allocator, size_t len)
{
  if (allocator->type == DEFAULT) 
      return malloc(len);
  
  return NULL;
}

void pfree(allocator_t* allocator, void* block)
{
  if(allocator->type == DEFAULT) 
    free(block);
}

allocator_t default_allocator_cpy(allocator_t* allocator)
{
  return *allocator;
}

const allocator_t GLOBAL_ALLOCATOR = { DEFAULT, default_allocator_cpy, 0};
const allocator_t NO_ALLOCATOR = { NOP, 0, 0 };

allocator_t allocator_copy(allocator_t* allocator)
{
  if(allocator->cpy)
    return allocator->cpy(allocator);

  return NO_ALLOCATOR;
}


void allocator_delete(allocator_t* allocator)
{
  if(allocator->del) 
    allocator->del(allocator);
}



#endif