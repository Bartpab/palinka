#ifndef __MMIX_PROC_H__
#define __MMIX_PROC_H__

#include "../../lib/common/include/types.h"
#include "../sys.h"

#include "./instr.h"
#include "./reg.h"
#include "./op.h"

#define MMIX_START_ADDR 0x8000000000000000
#define MMIX_STACK_BASE 0x6000000000000000

#define MMIX_RESUMING 1

#define MMIX_RESUME_AGAIN 0
#define MMIX_RESUME_CONT 1
#define MMIX_RESUME_SET 2

typedef struct {
  void (*hdlr) (system_t* sys, instr_t* instr);
} mmix_ivte;

typedef struct {
  unsigned int lsize;
  unsigned int lmask;
  unsigned int frequency;
} mmix_cfg_t;

typedef struct {
  octa g[256]; // Global Registers
  octa* l;     // Local Registers

  unsigned int lsize; // Power of two
  unsigned int lmask; // == lring_size - 1
  
  octa *instr_ptr;    // Current instruction pointer
  int rounding_mode;  // Current Rounding Mode for float operations
  
  int S, G, L, O; // Copies of rS, rG, rL, rO
  tetra rop;

  int state; // State of the processor
  octa sclock; // System clock

  unsigned int frequency; // Processor frequency

  mmix_ivte ivt[256]; // Interrupt Vector Table (TRAP)
} mmix_processor_t;

void mmix_cfg_init(mmix_cfg_t* cfg) 
{
  cfg->lsize = 256;
  cfg->lmask = 255;
  cfg->frequency = 500000000; // 500 MHz
}

mmix_processor_t* __get_mmix_proc(system_t* sys)
{
  return (mmix_processor_t*) (sys + 1);
}

octa* __get_mmix_lregs(system_t* sys) 
{
  mmix_processor_t* proc = __get_mmix_proc(sys);
  return (octa*) (proc + 1);
}

#endif