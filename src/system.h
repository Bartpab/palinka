#ifndef __SYS_H__
#define __SYS_H__

#include "../lib/common/include/allocator.h"
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
  float atomic_time; // Smallest unit of time possible

  // The memory of the system
  memory_t mem;
  
  // Sub-systems
  struct system_t* subs[32]; 
  struct system_t** last_sub;

  // Keep track of the allocator.
  allocator_t allocator;
  
  // VTable
  void (*step)(struct system_t* sys);
  void (*alloc_sim_time)(struct system_t* sys, unsigned int ms);

} system_t;

void __sys_init(system_t* sys, allocator_t* page_node_allocator)
{
  __mem_init(&sys->mem, page_node_allocator);
  sys->state = SYS_READY;
}

bool sys_add_subsystem(system_t* parent, system_t* sub)
{
  if(parent->last_sub == &parent->subs[31])
    return false;
  
  if(parent->last_sub == 0)
    parent->last_sub = &parent->subs[0];
  else
    parent->last_sub++;
  
  *parent->last_sub = sub;

  return true;
}

void sys_destroy(system_t* sys)
{
  // Delete the memory.
  mem_destroy(&sys->mem);
}

void sys_delete(system_t* sys, allocator_t* allocator)
{ 
  sys_destroy(sys);
  pfree(allocator, sys);
}

bool sys_load_byte(system_t* sys, void* addr, byte* ptr)
{
  char exceptions;
  void* base;

  if(!mem_tl(&sys->mem, addr, &base, &exceptions))
    return false;
  
  *ptr = *(byte*) base;

  return true;
}

bool sys_load_word(system_t* sys, void* addr, word* word)
{
  byte bytes[2];

  if(!sys_load_byte(sys, addr, &bytes[0]) || !sys_load_byte(sys, addr + 1, &bytes[1]))
    return false;
  
  *word = (bytes[1] << 8) | bytes[0];

  return true;
}

bool sys_load_tetra(system_t* sys, void* addr, tetra* tetra)
{
  word words[2];
  
  if(!sys_load_word(sys, addr, &words[0]) || !sys_load_word(sys, addr + 2, &words[1]))
    return false;
  
  *tetra = (words[1] << 16) | words[0];
  return true;
}

bool sys_load_octa(system_t* sys, void* addr, octa* o)
{
  tetra tetras[2];
  
  if(!sys_load_tetra(sys, addr, &tetras[0]) || !sys_load_tetra(sys, addr + 4, &tetras[1]))
    return false;
  
  *o = ((octa)(tetras[1]) << 32) | tetras[0];
  return true;
}

bool sys_store_byte(system_t* sys, void* addr, const byte value)
{
  char exceptions;
  void* base;

  if(false == mem_tl(&sys->mem, addr, &base, &exceptions))
    return false;

  *(byte*)(base) = value;

  return true;
}

bool sys_store_word(system_t* sys, void* addr, const word wd)
{
  byte bytes[2] = {
    wd & 0xFF,
    (wd >> 8) & 0xFF
  };

  return sys_store_byte(sys, addr, bytes[0]) && sys_store_byte(sys, addr + 1, bytes[1]);
}

bool sys_store_tetra(system_t* sys, void* addr, const tetra t)
{
  word words[2] = {
    t & 0xFFFF,
    (t >> 16) & 0xFFFF
  };

  return sys_store_word(sys, addr, words[0]) && sys_store_word(sys, addr + 2, words[1]);
}

bool sys_store_octa(system_t* sys, void* addr, const octa o)
{
  tetra tetras[2] = {
    o & 0xFFFFFFFF,
    (o >> 32) & 0xFFFFFFFF
  };

  return sys_store_tetra(sys, addr, tetras[0]) && sys_store_tetra(sys, addr + 4, tetras[1]);
}

void* sys_add_memory(system_t* sys, allocator_t* allocator, void* vbase, size_t len)
{
  return mem_alloc_managed(&sys->mem, allocator, vbase, len);
}

void sys_step(system_t* sys) 
{
  if(sys->state == SYS_PANICKED)
    return;

  sys->step(sys);
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

  while(sys->state == SYS_RUNNING) sys_step(sys);
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

#endif