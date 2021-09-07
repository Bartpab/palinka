#ifndef __SYS_H__
#define __SYS_H__

#include "allocator.h"
#include "mem.h"

typedef enum {
  STOPPED,
  PAUSED,
  RUNNING,
  PANIC
} system_state_t;

typedef struct system_t
{
  // System state
  int state;

  // The memory of the system
  memory_t mem;
  
  // Keep track of the allocator.
  allocator_t allocator;
  
  // VTable
  void (*step)(struct system_t* sys);

} system_t;

void __sys_init(system_t* sys, allocator_t* allocator)
{
  __mem_init(&sys->mem, NULL, allocator);
  sys->allocator = allocator_copy(allocator);
}

void sys_delete(system_t* sys)
{ 
  // Delete the memory.
  mem_delete(&sys->mem);

  // Copy the allocator locally.
  allocator_t allocator = allocator_copy(&sys->allocator);

  // Delete the owned allocator.
  allocator_delete(&sys->allocator);
  
  // Free the system
  pfree(&allocator, sys);

  // Delete the allocator.
  allocator_delete(&allocator);
}

// Trigger system panic
void sys_panic(system_t* sys)
{
  sys->state = PANIC;
}

#endif