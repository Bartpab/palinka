#ifndef __SYS_H__
#define __SYS_H__

#include "allocator.h"
#include "mem.h"

typedef enum {
  SYS_READY,
  SYS_STOPPED,
  SYS_HALTED,
  SYS_RUNNING,
  SYS_PANICKED
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
  void (*alloc_sim_time)(struct system_t* sys, unsigned int ms);

  // Some stats

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

void sys_step(system_t* sys) 
{
  assert(sys->state != SYS_PANICKED);
  sys->step(sys);
}

void sys_run(system_t* sys, unsigned int ms)
{
  if(sys->state == SYS_STOPPED)
    return;

  if(sys->state == SYS_HALTED || sys->state == SYS_READY) 
  {
    sys->alloc_sim_time(sys, ms);
    sys->state = SYS_RUNNING;
  }

  while(sys->state != SYS_RUNNING)
    sys_step(sys);
}

void sys_halt(system_t* sys) 
{
  sys->state = SYS_HALTED;
}

// Trigger system panic
void sys_panic(system_t* sys)
{
  sys->state = SYS_PANICKED;
}

#endif