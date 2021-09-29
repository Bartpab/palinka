#ifndef __RISCV_CORE_H__
#define __RISCV_CORE_H__

#include "../../lib/common/include/allocator.h"
#include "../system.h"
#include "./processor.h"

#define RISCV_START_ADDRESS 0x20000000

system_t* riscv_new(allocator_t* allocator, riscv_processor_cfg_t* cfg);
void riscv_alloc_sim_time(system_t* sys, unsigned int ms);
void riscv_step(system_t* sys);

static void __riscv_init(system_t* sys, riscv_processor_cfg_t* cfg);

system_t* riscv_new(allocator_t* allocator, riscv_processor_cfg_t* cfg)
{
  system_t* sys = (system_t*) pmalloc(
    allocator, 
    sizeof(system_t) 
      + sizeof(riscv_processor_t) 
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

    // Init the bus interface
    proc->bus_interface.fsb = 0; // Connected to nothing

    proc->pc = cfg->boot_address;

    for(int i = 0; i < 32; i++) proc->regs[i] = 0;
    for(int i = 0; i < 4096; i++) proc->csrs[i] = 0;

    proc->regs[2]   = cfg->memory_size;
    proc->regs[0]   = octa_zero;
    

    riscv_pipeline_create(&proc->pipeline);
}

void riscv_alloc_sim_time(system_t* sys, unsigned int ms) {
    riscv_processor_t* proc = __get_riscv_proc(sys);

    float s = (float)(ms) / 1000.0;
    float remaining_cycles = s * (float)(proc->frequency);
    proc->remaining_cycles = (int) remaining_cycles;
}

void riscv_step(system_t* sys)
{   
    riscv_processor_t* proc = __get_riscv_proc(sys);
    
    if(proc->remaining_cycles == 0) 
        return sys_halt(sys);

    // Set zero at each cycle
    proc->regs[0] = 0;

    riscv_pipeline_step(sys, proc, &proc->pipeline);

    if(proc->remaining_cycles > 0) 
        proc->remaining_cycles--;
}


#endif