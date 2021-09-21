#ifndef __RISCV_PROC_H__
#define __RISCV_PROC_H__

#include "../../lib/common/include/types.h"

typedef struct {
    octa regs[32];
    octa csrs[4096];

    octa pc; // Program counter
} riscv_processor_t;

inline riscv_processor_t* __get_riscv_proc(system_t* sys)
{
  return (riscv_processor_t*) (sys + 1);
}


#endif