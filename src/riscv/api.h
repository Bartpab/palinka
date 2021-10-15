#ifndef __RISCV_API_H__
#define __RISCV_API_H__

#include "./model.h"
#include "./pipeline.h"

system_t* riscv_new(allocator_t* allocator, riscv_processor_cfg_t* cfg);
void riscv_step(system_t* sys);

static void __riscv_init(system_t* sys, riscv_processor_cfg_t* cfg);

static void __riscv_on_itf_interrupt(system_t* sys, processor_itf_t* itf, transaction_t* transaction, processor_itf_event_payload_t payload);
static void __riscv_on_itf_data_read(system_t* sys, processor_itf_t* itf, transaction_t* transaction, processor_itf_event_payload_t payload);

static void __riscv_on_l1_send(system_t* sys, data_cache_t* l1, transaction_t* transaction, data_cache_event_payload_t payload);
static void __riscv_on_l1_fetch(system_t* sys, data_cache_t* l1, transaction_t* transaction, data_cache_event_payload_t payload);

system_t* riscv_new(allocator_t* allocator, riscv_processor_cfg_t* cfg)
{
  system_t* sys = (system_t*) pmalloc(
    allocator, 
    sizeof(system_t) + sizeof(riscv_processor_t) 
  );
  
  if(!sys) return NULL;

  __sys_init(sys, allocator);
  __riscv_init(sys, cfg);
 
  return sys;
}

static void __riscv_init(system_t* sys, riscv_processor_cfg_t* cfg) 
{
    riscv_processor_t* proc = __get_riscv_proc(sys);

    // V-Table
    sys->vtable.step = riscv_step;
    
    proc->frequency = cfg->frequency;
    proc->pc        = cfg->boot_address;

    for(unsigned int i = 0; i < 32; i++)   proc->regs[i] = 0;
    for(unsigned int i = 0; i < 4096; i++) proc->csrs[i] = 0;

    //proc->regs[2]   = cfg->memory_size;
    proc->regs[0]   = octa_zero;
    
    sys->atomic_time = 1.0 / (float)(cfg->frequency);

    // Setup the pipeline
    riscv_pipeline_create(&proc->pipeline);

    // Setup the interface
    processor_itf_create(&proc->itf);
    
    // Setup a handler for the READ/INTERRUPT events from proc itf.
    proc->itf.event_handlers[PROC_ITF_EVENT_READ].self = sys;
    proc->itf.event_handlers[PROC_ITF_EVENT_READ].hdlr = (processor_itf_event_handler_t) __riscv_on_itf_data_read;
  
    proc->itf.event_handlers[PROC_ITF_EVENT_INTERRUPT].self = sys;
    proc->itf.event_handlers[PROC_ITF_EVENT_INTERRUPT].hdlr = (processor_itf_event_handler_t) __riscv_on_itf_interrupt;

    // Setup the L1 cache
    data_cache_create(&proc->l1, (data_cache_entry_t*) &proc->__l1_entries, sizeof(proc->__l1_entries) / sizeof(data_cache_entry_t));

    // Setup a handler for the FETCH/SEND events from proc L1 cache.
    proc->l1.event_handlers[DATA_CACHE_EVENT_FETCH].self = sys;
    proc->l1.event_handlers[DATA_CACHE_EVENT_FETCH].hdlr = (data_cache_event_handler_t) __riscv_on_l1_fetch;

    proc->l1.event_handlers[DATA_CACHE_EVENT_SEND].self = sys;
    proc->l1.event_handlers[DATA_CACHE_EVENT_SEND].hdlr = (data_cache_event_handler_t) __riscv_on_l1_send;
}

static void __riscv_on_itf_interrupt(system_t* sys, processor_itf_t* itf, transaction_t* transaction, processor_itf_event_payload_t payload)
{
  //riscv_processor_t* proc = __get_riscv_proc(sys);
}

static void __riscv_on_itf_data_read(system_t* sys, processor_itf_t* itf, transaction_t* transaction, processor_itf_event_payload_t payload)
{
  riscv_processor_t* proc = __get_riscv_proc(sys);

  // Update the data cache
  data_cache_update(&proc->l1, payload.read.addr, payload.read.data, transaction);
}

static void __riscv_on_l1_send(system_t* sys, data_cache_t* l1, transaction_t* transaction, data_cache_event_payload_t payload)
{
  //riscv_processor_t* proc = __get_riscv_proc(sys);

}

static void __riscv_on_l1_fetch(system_t* sys, data_cache_t* l1, transaction_t* transaction, data_cache_event_payload_t payload)
{
  //riscv_processor_t* proc = __get_riscv_proc(sys);
}

void riscv_step(system_t* sys)
{   
  riscv_processor_t* proc = __get_riscv_proc(sys);

  // Set zero at each cycle
  proc->regs[0] = 0;

  // Data cache step
  data_cache_step(&proc->l1, &sys->transaction);

  // Interface step
  processor_itf_step(&proc->itf, &sys->transaction);

  // Pipeline step
  riscv_pipeline_step(sys, proc, &proc->pipeline, &sys->transaction);
}

#endif