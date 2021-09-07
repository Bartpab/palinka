#ifndef __MMIX_PROC_H__
#define __MMIX_PROC_H__

#include "../types.h"
#include "../sys.h"

#include "instr.h"
#include "reg.h"
#include "op.h"

#define MMIX_START_ADDR 0x8000000000000000

typedef struct {
  void (*hdlr) (system_t* sys, instr_t* instr);
} mmix_ivte;

typedef struct {
  octa reg[256];      // Registers
  octa *instr_ptr;    // Current instruction
  int rounding_mode;  // Current Rounding Mode for float operations

  mmix_ivte ivt[256]; // Interrupt Vector Table (TRAP)
} mmix_processor_t;

mmix_processor_t* __get_mmix_proc(system_t* sys)
{
  return (mmix_processor_t*) (sys + 1);
}

#endif