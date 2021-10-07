#ifndef __RISCV_PIPELINE_MODEL_H__
#define __RISCV_PIPELINE_MODEL_H__

#include "../../../lib/common/include/types.h"
#include "../../../lib/common/include/transaction.h"

typedef struct {
  int addr;
  char type; // 0: Normal reg, 1: CSR
} riscv_reg_addr_t;

typedef struct {
  char op; // 0: NOTHING, 1: STORE, 2: LOAD
  octa addr;
} riscv_memory_op_t;

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
  struct {
      octa current_pc;
  } debug;
} riscv_stage_decode_t;

typedef struct {
  octa pc;
  struct {
      bool stall, invalid;
      int op;
      octa imm;
      bool write_pc, arg1_is_imm;
      riscv_reg_addr_t sregs[2];
      riscv_reg_addr_t dregs[2];
  } control;
  struct {
      octa current_pc;
  } debug;
} riscv_stage_read_t;

typedef struct {
  octa    pc;
  octa   args[2];
  struct {
      bool stall, invalid;
      int op;
      octa imm;
      bool write_pc, arg1_is_imm;
      riscv_reg_addr_t sregs[2];
      riscv_reg_addr_t dregs[2];
  } control;
  struct {
      octa current_pc;
  } debug;
} riscv_stage_execute_t;

typedef struct {
  octa pc;
  octa results[2];
  struct {
      bool stall, invalid, wait;
      int op;
      riscv_reg_addr_t dregs[2];
      riscv_memory_op_t memory_op;
  } control;
  struct {
      octa current_pc;
  } debug;
  struct {
      bool halt;
  } simulation;
} riscv_stage_memory_t;

typedef struct {
  octa results[2];
  struct {
    bool stall, invalid;
    riscv_reg_addr_t dregs[2];
  } control;
  struct {
      octa current_pc;
  } debug;
  struct {
      bool halt;
  } simulation;
} riscv_stage_writeback_t;

typedef struct 
{ 
    // Fetch register
    riscv_stage_fetch_t fetch;

    // Fetch/Decode Register
    riscv_stage_decode_t decode;

    // Decode/Read Register
    riscv_stage_read_t read;

    // Read/Execute Register
    riscv_stage_execute_t execute;

    // Execute/Memory Register
    riscv_stage_memory_t memory;

    //Memory/Writeback Register
    riscv_stage_writeback_t writeback;

} riscv_pipeline_t;

decl_tst_update_type(riscv_stage_fetch_t, riscv_stage_fetch)
decl_tst_update_type(riscv_stage_decode_t, riscv_stage_decode)
decl_tst_update_type(riscv_stage_read_t, riscv_stage_read)
decl_tst_update_type(riscv_stage_execute_t, riscv_stage_execute)
decl_tst_update_type(riscv_stage_memory_t, riscv_stage_memory)
decl_tst_update_type(riscv_stage_writeback_t, riscv_stage_writeback)

#endif