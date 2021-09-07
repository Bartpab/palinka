#ifndef __MMIX_MEM_H__
#define __MMIX_MEM_H__

#include "../types.h"
#include "../sys.h"
#include "../mem.h"

#include "processor.h"

#define PAGE_NOT_PRESENT_BIT (1 << 19);

#define MMIX_MEM_ACCESS_FAILBACK(sys, addr, type, out, on_failure) {\
  void* __out;\
  if(!mmix_memtl(sys, addr, &__out)) {\
    sys_panic(sys);\
    on_failure;\
  }\
  out = (type*) __out;\
}

#define MMIX_MEM_READ_FAILBACK(sys, addr, type, out, on_failure) {\
  type* __out2; \
  MMIX_MEM_ACCESS_FAILBACK(sys, addr, type, __out2, on_failure);\
  out = *(type*) __out2; \
}

#define MMIX_MEM_WRITE_FAILBACK(sys, addr, type, val, on_failure) {\
  void* __out;\
  if(!mmix_memtl(sys, addr, &__out)) {\
    sys_panic(sys); \
    on_failure; \
  \} \
  *(type*) __out = val;}

#define MMIX_MEM_ACCESS(sys, addr, out) MMIX_MEM_ACCESS_FAILBACK(sys, addr, type, out, return)
#define MMIX_MEM_READ(sys, addr, type, out) MMIX_MEM_READ_FAILBACK(sys, addr, type, out, return)
#define MMIX_MEM_WRITE(sys, addr, type, val) MMIX_MEM_WRITE_FAILBACK(sys, addr, type, val, return)

/*
* \brief Get the real addr.
*/
bool mmix_memtl(system_t* sys, void* addr, void** out)
{
  mmix_processor_t* proc = __get_mmix_proc(sys);

  char exceptions = 0;

  bool retres = mem_tl(&sys->mem, addr, out, &exceptions);
  
  if(!retres)
    proc->reg[rA] |= exceptions << 19; // Set the value

  return retres;
}


#endif