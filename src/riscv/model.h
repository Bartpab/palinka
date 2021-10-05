#ifndef __RISCV_CORE_H__
#define __RISCV_CORE_H__

#include "../../lib/common/include/allocator.h"
#include "../../lib/common/include/transaction.h"
#include "../processor/cache.h"
#include "../processor/itf.h"
#include "../system.h"
#include "./pipeline/model.h"

#define RISCV_START_ADDRESS 0x20000000

typedef struct {
    octa regs[32];
    octa csrs[4096];

    octa pc;

    riscv_pipeline_t pipeline;
    processor_itf_t itf;

    // L1 cache
    data_cache_t l1;
    data_cache_entry_t __l1_entries[1000000]; // 1 mo of cache 

    // Simulation
    unsigned int frequency; // Hz
    int remaining_cycles;
} riscv_processor_t;

typedef struct {
  unsigned int frequency;
  unsigned int memory_size;
  unsigned int boot_address;
} riscv_processor_cfg_t;

riscv_processor_t* __get_riscv_proc(system_t* sys)
{
  return (riscv_processor_t*) (sys + 1);
}

#endif