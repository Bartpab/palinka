#ifndef __RISCV_PROC_H__
#define __RISCV_PROC_H__

#include "../../lib/common/include/types.h"
#include "../system.h"
#include "./instr.h"

typedef struct {
  struct {
    bool stall;
  } control;
} riscv_stage_fetch_t;

typedef struct {
  octa pc;
  tetra raw;
  struct {
    bool stall, invalid;
  } control;
} riscv_stage_decoder_t;

typedef struct {
  octa pc;
  struct {
      bool stall, invalid;
      int op;
      octa imm;
      bool write_pc, arg1_is_imm;
      unsigned char src_regs[2]; 
      unsigned char dest_reg; // Address of the destination registers
  } control;
} riscv_stage_read_t;

typedef struct {
  octa    pc;
  octa   args[2];
  struct {
      bool stall, invalid;
      int op;
      octa imm;
      bool write_pc, arg1_is_imm;
      unsigned char src_regs[2]; 
      unsigned char dest_reg; // Address of the destination registers
  } control;
} riscv_stage_execute_t;

typedef struct {
  octa pc;
  octa result;
  struct {
      bool stall, invalid;

      int op;
      unsigned char dest_reg; // Address of the destination registers

      bool store;
      octa store_addr;

      bool load;
      octa load_addr;
  } control;
} riscv_stage_memory_t;

typedef struct {
  octa result;
  struct {
    bool stall, invalid;
    unsigned char dest_reg; // Address of the destination registers
  } control;
} riscv_stage_writeback_t;

typedef struct 
{ 
    // Program counter
    octa pc;

    // Fetch register
    riscv_stage_fetch_t fetch;

    // Fetch/Decode Register
    riscv_stage_decoder_t decoder;

    // Decode/Read Register
    riscv_stage_read_t read;

    // Read/Execute Register
    riscv_stage_execute_t execute;

    // Execute/Memory Register
    riscv_stage_memory_t memory;

    //Memory/Writeback Register
    riscv_stage_writeback_t writeback;

} riscv_pipeline_t;

typedef struct {
    octa regs[32];
    octa csrs[4096];

    octa pc;
    riscv_pipeline_t pipeline;

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

void riscv_pipeline_create(riscv_pipeline_t* pipeline)
{
    pipeline->fetch.control.stall = false;

    pipeline->decoder.pc = 0;
    pipeline->decoder.raw = 0;
    pipeline->decoder.control.stall = false;
    pipeline->decoder.control.invalid = false;

    pipeline->read.pc = 0;
    pipeline->read.control.dest_reg = 0;
    pipeline->read.control.imm = 0;
    pipeline->read.control.src_regs[0] = 0;
    pipeline->read.control.src_regs[1] = 0;
    pipeline->read.control.stall = false;
    pipeline->read.control.write_pc = false;
    pipeline->read.control.invalid = false;
    pipeline->read.control.arg1_is_imm = false;
    
    pipeline->execute.args[0] = 0;
    pipeline->execute.args[1] = 0;
    pipeline->execute.pc = 0;
    pipeline->execute.control.dest_reg = 0;
    pipeline->execute.control.imm = 0;
    pipeline->execute.control.src_regs[0] = 0;
    pipeline->execute.control.src_regs[1] = 0;
    pipeline->execute.control.stall = false;
    pipeline->execute.control.write_pc = false;
    pipeline->execute.control.invalid = false;
    pipeline->execute.control.arg1_is_imm = false;

    pipeline->memory.pc = 0;
    pipeline->memory.result = 0;
    pipeline->memory.control.dest_reg = 0;
    pipeline->memory.control.load = 0;
    pipeline->memory.control.load_addr = 0;
    pipeline->memory.control.store = false;
    pipeline->memory.control.store_addr = 0;
    pipeline->memory.control.stall = false;
    pipeline->memory.control.invalid = false;

    pipeline->writeback.control.dest_reg = 0;
    pipeline->writeback.control.stall = false;
    pipeline->writeback.control.invalid = true;
    pipeline->writeback.result = 0;
}

#endif