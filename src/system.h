#ifndef __SYS_H__
#define __SYS_H__

#include "../lib/common/include/allocator.h"
#include "../lib/common/include/transaction.h"
#include "./memory/core.h"

#include <string.h>

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
  float steps;   // Time left
  float atomic_time; // Smallest unit of time possible (unit: s)

  // State transaction
  transaction_t transaction;

  // Keep track of the allocator.
  allocator_t allocator;
  
  // VTable
  void (*step)(struct system_t* sys);
  void (*commit)(struct system_t* sys);
  void (*alloc_sim_time)(struct system_t* sys, unsigned int ms);

} system_t;

void __sys_init(system_t* sys, allocator_t* transaction_allocator)
{
  transaction_create(&sys->transaction, transaction_allocator, 1000);
  sys->state = SYS_READY;
  sys->steps = sys->atomic_time = 0;
  sys->step = 0;
  sys->commit = 0;
  sys->alloc_sim_time = 0;
}

void sys_destroy(system_t* sys)
{
  // Delete the memory.
  transaction_destroy(&sys->transaction);
}

void sys_delete(system_t* sys, allocator_t* allocator)
{ 
  sys_destroy(sys);
  pfree(allocator, sys);
}

void sys_commit(system_t* sys)
{
  tst_commit(&sys->transaction);
}

void sys_step(system_t* sys) 
{
  if(sys->steps > 0) sys->steps--;
  
  if(sys->state == SYS_PANICKED)
    return;

  if(!sys->step) 
    return;
  
  sys->step(sys);
  sys_commit(sys);
}

void sys_loop(system_t* sys)
{
  if(sys->state == SYS_STOPPED)
    return;

  if(sys->state == SYS_HALTED || sys->state == SYS_READY) 
  {
    sys->alloc_sim_time(sys, -1);
    sys->state = SYS_RUNNING;
  }

  while(sys->state == SYS_RUNNING) 
    sys_step(sys);
}

void sys_stop(system_t* sys)
{
  sys->state = SYS_STOPPED;
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

void sys_run(system_t* sys, unsigned int ms)
{
  if(sys->state == SYS_STOPPED)
    return;

  if(sys->state == SYS_HALTED || sys->state == SYS_READY) 
  {
    sys->state = SYS_RUNNING;
    sys->steps += (int)(ms * 1000 / sys->atomic_time);
  }

  while(sys->state == SYS_RUNNING && sys->steps > 0) 
  {
    sys_step(sys);
  }

  sys_halt(sys);
}

#endif