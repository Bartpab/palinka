#ifndef __MMIX_MEM_H__
#define __MMIX_MEM_H__

#include "../types.h"
#include "../sys.h"
#include "../mem.h"

#include "processor.h"

#define PAGE_NOT_PRESENT_BIT (1 << 19);

#define MMIX_MEM_ACCESS_FAILBACK(sys, addr, out, on_failure) if(!mmix_memtl(sys, addr, &out)) { sys_panic(sys); on_failure; }

#define MMIX_MEM_READ_FAILBACK(sys, addr, out, on_failure) {octa* __out; MMIX_MEM_ACCESS_FAILBACK(sys, addr, __out, on_failure); out = *__out; }

#define MMIX_MEM_WRITE_FAILBACK(sys, addr, val, on_failure) {octa* __out; if(!mmix_memtl(sys, addr, &__out)) {sys_panic(sys); on_failure;} *__out = val;}

#define MMIX_MEM_ACCESS(sys, addr, out) MMIX_MEM_ACCESS_FAILBACK(sys, addr, out, return)
#define MMIX_MEM_READ(sys, addr, out) MMIX_MEM_READ_FAILBACK(sys, addr, out, return)
#define MMIX_MEM_WRITE(sys, addr, val) MMIX_MEM_WRITE_FAILBACK(sys, addr, val, return)

/*
* \brief Get the real addr.
*/
bool mmix_memtl(system_t* sys, octa* addr, octa** out)
{
  mmix_processor_t* proc = __get_mmix_proc(sys);
  
  void* _out;
  char exceptions = 0;

  bool retres = mem_tl(&sys->mem, addr, &_out, &exceptions);
  
  if(retres)
    *out = (octa*) _out;
  
  proc->reg[rA] |= exceptions << 19; // Set the value

  return retres;
}


#endif