#ifndef __MMIX_API_H__
#define __MMIX_API_H__

#include "../sys.h"
#include "mem.h"
#include "processor.h"
#include "instr.h"
#include "dispatch.h"

void __mmix_init(system_t* sys)
{
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->instr_ptr = (octa*) MMIX_START_ADDR; // Start at 0x8000000000000000 .
  
  for(unsigned char i = 0; i != 0xFF; i++) 
  {
    proc->reg[i] = 0x00;
    proc->ivt[i].hdlr = NULL;
  }
}

system_t* mmix_create(allocator_t* allocator)
{
  system_t* sys = pmalloc(allocator, sizeof(system_t) + sizeof(mmix_processor_t));
  
  if(!sys)
    return NULL;

  __sys_init(sys, allocator);
  __mmix_init(sys);
 
  return sys;
}

void mmix_step(system_t* sys)
{
  mmix_processor_t* proc = __get_mmix_proc(sys);

  // Get the current instruction to execute.
  octa bin_instr;

  // Read the instruction (64-bit)
  MMIX_MEM_READ(sys, proc->instr_ptr, octa, bin_instr);
  
  instr_t instr = op_tl_binary(bin_instr);

  // Dispatch the instruction.
  mmix_dispatch(sys, proc, &instr);
  
  // Increment the instr pointer for the next step.
  proc->instr_ptr++;
}

void mmix_set_interrupt_handler(system_t* sys, byte ircode, void (*hdlr)(system_t* sys, instr_t* instr))
{
  mmix_processor_t* proc = __get_mmix_proc(sys);
  proc->ivt[ircode].hdlr = hdlr;
}

#endif