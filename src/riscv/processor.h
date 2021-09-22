#ifndef __RISCV_PROC_H__
#define __RISCV_PROC_H__

#include "../../lib/common/include/types.h"
#include "../system.h"
#include "./control.h"

typedef enum {
  RISCV_FETCH,
  RISCV_DECODE,
  RISCV_READ,
  RISCV_EXECUTE,
  RISCV_MEMORY,
  RISCV_WRITE
} riscv_stage_t;

typedef struct {
    octa regs[32];
    octa csrs[4096];

    octa pc;                          // Program counter
    riscv_control_t current_control;  // Current control state
    int stage;                        // State

    // Simulation
    unsigned int frequency; // Hz
    int remaining_cycles;


} riscv_processor_t;

riscv_processor_t* __get_riscv_proc(system_t* sys)
{
  return (riscv_processor_t*) (sys + 1);
}


#endif