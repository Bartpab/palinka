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
      riscv_decoded_instr_t instr;
      octa imm;
      bool write_pc;
      unsigned char src_regs[2]; 
      unsigned char dest_reg; // Address of the destination registers
  } control;
} riscv_stage_read_t;

typedef struct {
  octa    pc;
  tetra   args[2];
  struct {
      bool stall, invalid;
      riscv_decoded_instr_t instr;
      octa imm;
      bool write_pc;
      unsigned char src_regs[2]; 
      unsigned char dest_reg; // Address of the destination registers
  } control;
} riscv_stage_execute_t;

typedef struct {
  octa pc;
  tetra result;
  struct {
      bool stall, invalid;

      riscv_decoded_instr_t instr;
      unsigned char dest_reg; // Address of the destination registers

      bool store;
      octa store_addr;

      bool load;
      octa load_addr;
  } control;
} riscv_stage_memory_t;

typedef struct {
  tetra result;
  struct {
    bool stall, invalid;
    unsigned char dest_reg; // Address of the destination registers
  } control;
} riscv_stage_writeback_t;

typedef struct {
    octa regs[32];
    octa csrs[4096];

    octa pc;                          // Program counter

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