#ifndef __RISCV_CORE_H__
#define __RISCV_CORE_H__

#include "../../lib/common/include/allocator.h"
#include "../processor/cache.h"
#include "../processor/itf.h"
#include "../system.h"
#include "./pipeline.h"

#define RISCV_START_ADDRESS 0x20000000

typedef struct {
    octa regs[32];
    octa csrs[4096];

    octa pc;

    riscv_pipeline_t pipeline;
    processor_itf_t itf;

    // L1 cache
    data_cache_t l1;
    data_cache_entry_t __l1_entries[2][100];

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

system_t* riscv_new(allocator_t* allocator, riscv_processor_cfg_t* cfg);
void riscv_alloc_sim_time(system_t* sys, unsigned int ms);
void riscv_step(system_t* sys);

static void __riscv_init(system_t* sys, riscv_processor_cfg_t* cfg);

system_t* riscv_new(allocator_t* allocator, riscv_processor_cfg_t* cfg)
{
  system_t* sys = (system_t*) pmalloc(
    allocator, 
    sizeof(system_t) + sizeof(riscv_processor_t) 
  );
  
  if(!sys)
    return NULL;

  __sys_init(sys, allocator);
  __riscv_init(sys, cfg);
 
  return sys;
}

static void __riscv_init(system_t* sys, riscv_processor_cfg_t* cfg) 
{
    riscv_processor_t* proc = __get_riscv_proc(sys);

    sys->step           = riscv_step;
    sys->alloc_sim_time = riscv_alloc_sim_time; 
    
    proc->frequency = cfg->frequency; //500MHz
    proc->remaining_cycles = 0;

    proc->pc = cfg->boot_address;

    for(int i = 0; i < 32; i++) proc->regs[i] = 0;
    for(int i = 0; i < 4096; i++) proc->csrs[i] = 0;

    proc->regs[2]   = cfg->memory_size;
    proc->regs[0]   = octa_zero;

    // Setup the pipeline
    riscv_pipeline_create(&proc->pipeline);

    // Setup the L1 cache
    data_cache_create(&proc->l1, (data_cache_entry_t*) &proc->__l1_entries[0], (data_cache_entry_t*) &proc->__l1_entries[1], 100);
}

void riscv_alloc_sim_time(system_t* sys, unsigned int ms) {
    riscv_processor_t* proc = __get_riscv_proc(sys);

    float s = (float)(ms) / 1000.0;
    float remaining_cycles = s * (float)(proc->frequency);
    proc->remaining_cycles = (int) remaining_cycles;
}

void riscv_itf_step(riscv_processor_t* proc)
{
  processor_itf_state_t* cur = &proc->itf.state[0];
  processor_itf_state_t* nxt = & proc->itf.state[1];

  // We have read data
  if(cur->status == PROC_ITF_STATUS_READ) 
  {
    // Update the data cache
    data_cache_update(&proc->l1, cur->mar, cur->mbr);
    
    // Go back to idling state
    nxt->status = PROC_ITF_STATUS_IDLING;
  }

  processor_itf_step(&proc->itf);
}

void riscv_step(system_t* sys)
{   
    riscv_processor_t* proc = __get_riscv_proc(sys);
    
    if(proc->remaining_cycles == 0) return sys_halt(sys);

    // Set zero at each cycle
    proc->regs[0] = 0;

    // Interface step
    riscv_itf_step(proc);

    // Pipeline step
    riscv_pipeline_step(sys, proc, &proc->pipeline);

    if(proc->remaining_cycles > 0) 
        proc->remaining_cycles--;
}


#endif