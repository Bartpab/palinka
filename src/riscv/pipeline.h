#ifndef __RISCV_PROC_H__
#define __RISCV_PROC_H__

#include "../../lib/common/include/types.h"
#include "../../lib/common/include/alu.h"

#include "../system.h"
#include "./instr.h"
#include "./csr.h"
#include "./opcode.h"

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
      bool stall, invalid;
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

typedef struct {
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

} riscv_pipeline_state_t;

typedef struct 
{ 
    riscv_pipeline_state_t state[2];

} riscv_pipeline_t;

void riscv_pipeline_create(riscv_pipeline_t* pipeline)
{
  for(unsigned char i = 0; i < 2; i++) 
  {
    riscv_pipeline_stage_fetch_create(&pipeline->state[i].fetch);
    riscv_pipeline_stage_decode_create(&pipeline->state[i].decode);
    riscv_pipeline_stage_read_create(&pipeline->state[i].read);
    riscv_pipeline_stage_execute_create(&pipeline->state[i].execute);
    riscv_pipeline_stage_memory_create(&pipeline->state[i].memory);
    riscv_pipeline_stage_writeback_create(&pipeline->state[i].writeback);
  }   
}

static void __load_csr(system_t* sys, riscv_processor_t* proc, unsigned int addr, octa* out)
{
    if(addr == SIE) {
        *out = proc->csrs[MIE] & proc->csrs[MIDELEG];
    } else {
        *out = proc->csrs[addr % 4096];
    }
}
static void __store_csr(system_t* sys, riscv_processor_t* proc, unsigned int addr, const octa value)
{
    if(addr == SIE) {
        proc->csrs[MIE] = (proc->csrs[MIE] & ~proc->csrs[MIDELEG]) | (value & proc->csrs[MIDELEG]);
    } else {
        proc->csrs[addr % 4096] = value;
    }
}

void riscv_pipeline_stage_fetch_create(riscv_stage_fetch_t* fetch)
{
    fetch->control.stall = false;
}
void riscv_pipeline_stage_decode_create(riscv_stage_decode_t* decode)
{
  decode->pc = 0;
  decode->raw = 0;
  decode->control.stall = false;
  decode->control.invalid = false;
  decode->debug.current_pc = 0;
}
void riscv_pipeline_stage_read_create(riscv_stage_read_t* read)
{
    read->pc = 0;
    read->control.imm = 0;
    read->control.sregs[0].addr = 0;
    read->control.sregs[0].type = 0;
    read->control.sregs[1].addr = 0;
    read->control.sregs[1].type = 0;
    read->control.dregs[0].addr = 0;
    read->control.dregs[0].type = 0;
    read->control.dregs[1].addr = 0;
    read->control.dregs[1].type = 0;
    read->control.stall = false;
    read->control.write_pc = false;
    read->control.invalid = false;
    read->control.arg1_is_imm = false; 
    read->debug.current_pc = 0;
}
void riscv_pipeline_stage_execute_create(riscv_stage_execute_t* execute)
{
    execute->args[0] = 0;
    execute->args[1] = 0;
    execute->pc = 0;
    execute->control.imm = 0;
    execute->control.op = 0;
    execute->control.sregs[0].addr = 0;
    execute->control.sregs[0].type = 0;
    execute->control.sregs[1].addr = 0;
    execute->control.sregs[1].type = 0;
    execute->control.dregs[0].addr = 0;
    execute->control.dregs[0].type = 0;
    execute->control.dregs[1].addr = 0;
    execute->control.dregs[1].type = 0;
    execute->control.stall = false;
    execute->control.write_pc = false;
    execute->control.invalid = false;
    execute->control.arg1_is_imm = false;
    execute->debug.current_pc = 0;
}
void riscv_pipeline_stage_memory_create(riscv_stage_memory_t* memory)
{
    memory->pc = 0;
    memory->results[0] = 0;
    memory->results[1] = 0;
    memory->control.op = 0;
    memory->control.memory_op.addr = 0;
    memory->control.memory_op.op = 0;
    memory->control.stall = false;
    memory->control.invalid = false;
    memory->control.dregs[0].addr = 0;
    memory->control.dregs[0].type = 0;
    memory->control.dregs[1].addr = 0;
    memory->control.dregs[1].type = 0;
    memory->debug.current_pc = 0;
    memory->simulation.halt = false;
}   
void riscv_pipeline_stage_writeback_create(riscv_stage_writeback_t* writeback)
{
    writeback->control.dregs[0].addr = 0;
    writeback->control.dregs[0].type = 0;
    writeback->control.dregs[1].addr = 0;
    writeback->control.dregs[1].type = 0;
    writeback->control.stall = false;
    writeback->control.invalid = true;
    writeback->results[0] = 0;
    writeback->results[1] = 0; 
    writeback->debug.current_pc = 0;
    writeback->simulation.halt = false;
}

static inline bool riscv_stage_fetch_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline)
{
    riscv_stage_fetch_t* in     = &pipeline->state[0].fetch;
    riscv_stage_decode_t* out   = &pipeline->state[1].decode;

    if(in->control.stall)
        return false;        

    tetra raw;

    bool cache_miss = riscv_data_cache_lookup(proc, proc->pc, (octa*) &raw);

    // We have a cache miss, we need to fetch data
    if(cache_miss) 
    {
        out->raw = 0;
        return;
    }

    proc->pc += 4;
    out->debug.current_pc = proc->pc - 4;
    out->pc = proc->pc;
    out->raw = raw;
    out->control.invalid = false;

    return true;
}
static inline void riscv_stage_decode_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline)
{
    riscv_stage_decode_t* in = &pipeline->state[0].decode;
    riscv_stage_read_t* out = &pipeline->state[1].read;

    if(in->control.stall)
        return;

    out->control.invalid = in->control.invalid;

    if(out->control.invalid)
        return;

    // Setup control
    riscv_decoded_instr_t decoded = decode(in->raw);
    
    out->control.op = decoded.op;
    out->control.sregs[0].type = decoded.sregs[0].type;
    out->control.sregs[0].addr = decoded.sregs[0].addr;
    out->control.sregs[1].type = decoded.sregs[1].type;
    out->control.sregs[1].addr = decoded.sregs[1].addr;
    out->control.dregs[0].type = decoded.dregs[0].type;
    out->control.dregs[0].addr = decoded.dregs[0].addr;
    out->control.dregs[1].type = decoded.dregs[1].type;
    out->control.dregs[1].addr = decoded.dregs[1].addr;
    out->control.arg1_is_imm = decoded.arg1_is_imm;

    // Fill the control block
    out->pc = in->pc;
    out->control.imm = decoded.imm;
    out->control.write_pc = decoded.write_pc;

    // Fill the debug block
    out->debug.current_pc = in->debug.current_pc;

    // Check any data hazards
    riscv_reg_addr_t forward_regs[6] = {
       pipeline->state[0].execute.control.dregs[0],
       pipeline->state[0].execute.control.dregs[1],
       pipeline->state[0].memory.control.dregs[0],
       pipeline->state[0].memory.control.dregs[1],
       pipeline->state[0].writeback.control.dregs[0], 
       pipeline->state[0].writeback.control.dregs[1]
    };

    for(unsigned char i = 0; i < 6; i++) 
    {
        for(unsigned char j = 0; j < 2; j++) 
        {
            if(forward_regs[i].addr == 0 && forward_regs[i].type == 0)
                continue;

            // Data hazard !
            if(forward_regs[i].type == pipeline->state[0].read.control.sregs[j].type && forward_regs[i].addr == pipeline->state[0].read.control.sregs[j].addr) 
            {
                pipeline->state[1].fetch.control.stall = true;
                pipeline->state[1].decode.control.stall = true;
                pipeline->state[1].read.control.stall = true;
                break;
            }
        }
    }
}
static inline void riscv_stage_read_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline)
{
    riscv_stage_read_t* in = &pipeline->state[0].read;
    riscv_stage_execute_t* out = &pipeline->state[1].execute;

    if(in->control.stall)
        return;

    out->control.invalid = in->control.invalid;

    if(out->control.invalid) 
        return;

    // Read registers
    out->args[0] = in->control.sregs[0].type == 0 ? proc->regs[in->control.sregs[0].addr]: proc->csrs[in->control.sregs[0].addr];
    out->args[1] = in->control.sregs[1].type == 0 ? proc->regs[in->control.sregs[1].addr]: proc->csrs[in->control.sregs[1].addr];

    // Copy target
    out->pc = in->pc;

    // Transfer control to control
    out->control.op = in->control.op;
    out->control.sregs[0] = in->control.sregs[0];
    out->control.sregs[1] = in->control.sregs[1];
    out->control.dregs[0] = in->control.dregs[0];
    out->control.dregs[1] = in->control.dregs[1];
    out->control.imm = in->control.imm;
    out->control.write_pc = in->control.write_pc;
    out->control.arg1_is_imm = in->control.arg1_is_imm;

    // Fill the debug block
    out->debug.current_pc = in->debug.current_pc;

}
static inline void riscv_stage_execute_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline)
{
    riscv_stage_execute_t* in = &pipeline->state[0].execute;
    riscv_stage_memory_t* out = &pipeline->state[1].memory;

    if(in->control.stall)
        return;

    out->simulation.halt = false;
    out->control.invalid = in->control.invalid;

    if(out->control.invalid)
        return;

    octa result[2], a, b, pc, imm;
    riscv_memory_op_t memory_op = {0, 0};

    a = in->args[0];
    b = in->control.arg1_is_imm ? in->control.imm : in->args[1];

    pc  = in->pc;
    imm = in->control.imm;
    result[0] = result[1] = 0;

    switch(in->control.op) 
    {
        case RISCV_LUI: result[0] = imm; break; // OK
        case RISCV_AUIPC: result[0] = octa_plus_expr(pc - 4 , octa_left_shift_expr(imm, 12)); break;
        // jump
        case RISCV_JAL: 
            result[0] = pc, pc = octa_plus_expr(octa_incr_expr(pc, -4), octa_left_shift_expr(b, 2)); 
        break; // OK
        case RISCV_JALR: result[0] = pc, pc = octa_and_expr(octa_plus_expr(a, octa_left_shift_expr(b, 2)), octa_compl_expr(3)); break; // OK
        // branch
        case RISCV_BEQ: if(octa_eq_expr(a, b) == true) pc = octa_plus_expr(pc, octa_left_shift_expr(imm, 2)); break;
        case RISCV_BNE: if(octa_eq_expr(a, b) == false) pc = octa_plus_expr(pc, octa_left_shift_expr(imm, 2)); break; // OK
        case RISCV_BLT: if(octa_signed_cmp_expr(a, b) == -1) pc = octa_plus_expr(pc, octa_left_shift_expr(imm, 2)); break;
        case RISCV_BLTU: if(octa_unsigned_cmp_expr(a, b) == -1) pc = octa_plus_expr(pc, octa_left_shift_expr(imm, 2)); break;
        case RISCV_BGE: if(octa_signed_cmp_expr(a, b) >= 0) pc = octa_plus_expr(pc, octa_left_shift_expr(imm, 2)); break;
        case RISCV_BGEU: if(octa_unsigned_cmp_expr(a, b) >= 0) pc = octa_plus_expr(pc, octa_left_shift_expr(imm, 2)); break;
        // load
        case RISCV_LBU: case RISCV_LB: case RISCV_LHU: case RISCV_LH: case RISCV_LW: case RISCV_LWU: case RISCV_LD: memory_op.op = 2, memory_op.addr = octa_plus_expr(a, b); break;
        // store
        case RISCV_SB: case RISCV_SH: case RISCV_SW: case RISCV_SD: memory_op.op = 1, memory_op.addr = octa_plus_expr(a, imm), result[0] = b; break;
        // add
        case RISCV_ADD:  case RISCV_ADDW: case RISCV_ADDI: case RISCV_ADDIW: result[0] = octa_plus_expr(a, b); break;
        // sub
        case RISCV_SUB: case RISCV_SUBW: result[0] = octa_minus_expr(a, b); break;
        // compare lt
        case RISCV_SLT: case RISCV_SLTI: result[0] = octa_signed_cmp_expr(a, b) == -1; break;         
        case RISCV_SLTU: case RISCV_SLTIU: result[0] = octa_unsigned_cmp_expr(a, b) == -1; break;
        // xor
        case RISCV_XORI: case RISCV_XOR: result[0] =  octa_xor_expr(a, b); break;
        // or
        case RISCV_ORI: case RISCV_OR: result[0] = octa_or_expr(a, b); break;
        // and
        case RISCV_ANDI: case RISCV_AND:  result[0] = octa_and_expr(a, b); break;
        // shift left
        case RISCV_SLL: case RISCV_SLLW:  result[0] = octa_left_shift_expr(a, b); break;
        case RISCV_SLLI: case RISCV_SLLIW: result[0] = octa_left_shift_expr(a, octa_and_expr(b, 0x1f)); break;
        // shift right
        case RISCV_SRL: case RISCV_SRA: case RISCV_SRAW: result[0] = octa_right_shift_expr(a, b, 0); break;
        case RISCV_SRLIW: case RISCV_SRAIW: case RISCV_SRAI: case RISCV_SRLI: result[0] = octa_right_shift_expr(a, octa_and_expr(b, 0x1f), 0); break;
        // Halt the simulation
        case RISCV_EBREAK: out->simulation.halt = true;
        case RISCV_FENCE_I: break;
        case RISCV_CSRRW: 
            result[0] = b, result[1] = a; 
            break;
        default: break;
    }
    
    // Write data
    out->results[0] = result[0];
    out->results[1] = result[1];
    out->pc = pc;
    
    // Write control
    out->control.op         = in->control.op;
    out->control.memory_op  = memory_op;
    out->control.dregs[0]   = in->control.dregs[0];
    out->control.dregs[1]   = in->control.dregs[1];

    // Write debug block
    out->debug.current_pc   = in->debug.current_pc;

}
static inline void riscv_stage_memory_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline)
{
    octa result[2];
    void* addr;

    riscv_stage_memory_t* in = &pipeline->state[0].memory;
    riscv_stage_writeback_t* out = &pipeline->state[1].writeback;

    if(in->control.stall)
        return;

    out->simulation.halt = in->simulation.halt;
    out->control.invalid = in->control.invalid;

    if(out->control.invalid)
        return;

    result[0] = in->results[0];
    result[1] = in->results[1];
    
    addr = octa_to_voidp(in->control.memory_op.addr);

    switch(in->control.op) 
    {
        // load
        case RISCV_LBU: case RISCV_LB: 
            sys_load_byte(sys, addr, (byte*) &result[0]); 
            break;
        case RISCV_LHU: case RISCV_LH: sys_load_word(sys, addr, (word*) &result[0]); break;
        case RISCV_LW: case RISCV_LWU: sys_load_tetra(sys, addr, (tetra*) &result[0]); break;
        case RISCV_LD: sys_load_octa(sys, addr, &result[0]); break;
        // store
        case RISCV_SB: sys_store_byte(sys, addr, result[0]); break;
        case RISCV_SH: sys_store_word(sys, addr, result[0]); break;
        case RISCV_SW: sys_store_tetra(sys, addr, result[0]); break;
        case RISCV_SD: sys_store_octa(sys, addr, result[0]); break;
    }

    out->results[0] = result[0];
    out->results[1] = result[1];
    
    // Write control
    out->control.dregs[0] = in->control.dregs[0];
    out->control.dregs[1] = in->control.dregs[1];

    // Write debug
    out->debug.current_pc = in->debug.current_pc;
}
static inline void riscv_stage_writeback_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline)
{
   riscv_stage_writeback_t* in = &pipeline->state[0].writeback;

    if(in->control.stall)
        return;

    if(in->control.invalid)
        return;

    for(unsigned char i = 0; i < 2; i++) 
    {
        octa* reg = in->control.dregs[i].type == 0 ? &proc->regs[in->control.dregs[i].addr] : &proc->csrs[in->control.dregs[i].addr];
        *reg = in->results[i];
    }

    pipeline->state[1].fetch.control.stall    = false;
    pipeline->state[1].decode.control.stall   = false;
    pipeline->state[1].read.control.stall     = false;
}

static inline void riscv_state_check_control_hazard(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline)
{
    riscv_stage_execute_t* in       = &pipeline->state[0].execute;
    riscv_stage_execute_t* nxt      = &pipeline->state[1].execute;

    riscv_stage_memory_t* memory    = &pipeline->state[0].memory;
    riscv_stage_read_t* read        = &pipeline->state[0].read;
    riscv_stage_decode_t* decode    = &pipeline->state[0].decode;
    
    // Write target
    if(in->control.write_pc == true && nxt->pc - 4 != memory->pc)
    {
        proc->pc = memory->pc;
        decode->control.invalid = true;
        read->control.invalid   = true;
        nxt->control.invalid = true;
    }
}

void riscv_pipeline_commit_state(riscv_pipeline_t* pipeline)
{
    pipeline->state[0] = pipeline->state[1];
}
void riscv_pipeline_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline)
{
  riscv_stage_fetch_step(sys, proc, pipeline);
  riscv_stage_decode_step(sys, proc, pipeline);
  riscv_stage_read_step(sys, proc, pipeline);
  riscv_stage_execute_step(sys, proc, pipeline);
  riscv_stage_memory_step(sys, proc, pipeline);
  riscv_stage_writeback_step(sys, proc, pipeline);

  riscv_state_check_control_hazard(sys, proc, pipeline);

  if(pipeline->state[0].writeback.simulation.halt && pipeline->state[0].writeback.control.invalid == false) sys_halt(sys);
}

#endif