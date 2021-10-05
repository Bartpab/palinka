#ifndef __RISCV_API_H__
#define __RISCV_API_H__

#include "./model.h"
#include "./pipeline.h"

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
    data_cache_create(&proc->l1, (data_cache_entry_t*) &proc->__l1_entries, 100);
}

void riscv_alloc_sim_time(system_t* sys, unsigned int ms) {
    riscv_processor_t* proc = __get_riscv_proc(sys);

    float s = (float)(ms) / 1000.0;
    float remaining_cycles = s * (float)(proc->frequency);
    proc->remaining_cycles = (int) remaining_cycles;
}

void riscv_itf_step(riscv_processor_t* proc, transaction_t* transaction)
{
  processor_itf_t* itf = &proc->itf;

  // We have read data
  if(itf->status == PROC_ITF_STATUS_READ) 
  {
    // Update the L1 data cache
    data_cache_update(&proc->l1, itf->mar, itf->mbr, transaction);
    
    // Go back to idling state
    tst_update_byte(transaction, &itf->status, PROC_ITF_STATUS_IDLING);
  }

  processor_itf_step(&proc->itf, transaction);
}

void riscv_step(system_t* sys)
{   
    riscv_processor_t* proc = __get_riscv_proc(sys);
    
    if(proc->remaining_cycles == 0) return sys_halt(sys);

    // Set zero at each cycle
    proc->regs[0] = 0;

    // Interface step
    riscv_itf_step(proc, &sys->transaction);

    // Pipeline step
    riscv_pipeline_step(sys, proc, &proc->pipeline, &sys->transaction);

    if(proc->remaining_cycles > 0) 
        proc->remaining_cycles--;
}

#endif