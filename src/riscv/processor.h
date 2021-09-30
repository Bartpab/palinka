#ifndef __RISCV_PROC_H__
#define __RISCV_PROC_H__

#include "../../lib/common/include/types.h"
#include "../../lib/common/include/alu.h"
#include "../itf/processor.h"
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

typedef struct 
{ 
    // Program counter
    octa pc;

    // Fetch register
    riscv_stage_fetch_t fetch[2];

    // Fetch/Decode Register
    riscv_stage_decode_t decode[2];

    // Decode/Read Register
    riscv_stage_read_t read[2];

    // Read/Execute Register
    riscv_stage_execute_t execute[2];

    // Execute/Memory Register
    riscv_stage_memory_t memory[2];

    //Memory/Writeback Register
    riscv_stage_writeback_t writeback[2];

} riscv_pipeline_t;

typedef struct {
    octa regs[32];
    octa csrs[4096];

    octa pc;

    riscv_pipeline_t pipeline;
    processor_itf_t itf;

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

void __load_csr(system_t* sys, riscv_processor_t* proc, unsigned int addr, octa* out)
{
    if(addr == SIE) {
        *out = proc->csrs[MIE] & proc->csrs[MIDELEG];
    } else {
        *out = proc->csrs[addr % 4096];
    }
}

void __store_csr(system_t* sys, riscv_processor_t* proc, unsigned int addr, const octa value)
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

void riscv_pipeline_create(riscv_pipeline_t* pipeline)
{
  for(unsigned char i = 0; i < 2; i++) 
  {
    riscv_pipeline_stage_fetch_create(&pipeline->fetch[i]);
    riscv_pipeline_stage_decode_create(&pipeline->decode[i]);
    riscv_pipeline_stage_read_create(&pipeline->read[i]);
    riscv_pipeline_stage_execute_create(&pipeline->execute[i]);
    riscv_pipeline_stage_memory_create(&pipeline->memory[i]);
    riscv_pipeline_stage_writeback_create(&pipeline->writeback[i]);
  }   
}


static inline bool riscv_stage_fetch_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline)
{
    riscv_stage_fetch_t* in = &pipeline->fetch[1];
    riscv_stage_decode_t* out = &pipeline->decode[0];

    if(in->control.stall)
        return false;        

    tetra raw;
    void* vaddr = octa_to_voidp(proc->pc);
    
    if(!sys_load_tetra(sys, vaddr, &raw))
    {
        out->pc = proc->pc;
        out->raw = 0; // NOP
        sys_halt(sys);
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
    riscv_stage_decode_t* in = &pipeline->decode[1];
    riscv_stage_read_t* out = &pipeline->read[0];

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
       pipeline->execute[1].control.dregs[0],
       pipeline->execute[1].control.dregs[1],
       pipeline->memory[1].control.dregs[0],
       pipeline->memory[1].control.dregs[1],
       pipeline->writeback[1].control.dregs[0], 
       pipeline->writeback[1].control.dregs[1]
    };

    for(unsigned char i = 0; i < 6; i++) 
    {
        for(unsigned char j = 0; j < 2; j++) 
        {
            if(forward_regs[i].addr == 0 && forward_regs[i].type == 0)
                continue;

            // Data hazard !
            if(forward_regs[i].type == pipeline->read[1].control.sregs[j].type && forward_regs[i].addr == pipeline->read[1].control.sregs[j].addr) 
            {
                pipeline->fetch[0].control.stall = true;
                pipeline->decode[0].control.stall = true;
                pipeline->read[0].control.stall = true;
                break;
            }
        }
    }
}
static inline void riscv_stage_read_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline)
{
    riscv_stage_read_t* in = &pipeline->read[1];
    riscv_stage_execute_t* out = &pipeline->execute[0];

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
    riscv_stage_execute_t* in = &pipeline->execute[1];
    riscv_stage_memory_t* out = &pipeline->memory[0];

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

    riscv_stage_memory_t* in = &pipeline->memory[1];
    riscv_stage_writeback_t* out = &pipeline->writeback[0];

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
   riscv_stage_writeback_t* in = &pipeline->writeback[1];

    if(in->control.stall)
        return;

    if(in->control.invalid)
        return;

    for(unsigned char i = 0; i < 2; i++) 
    {
        octa* reg = in->control.dregs[i].type == 0 ? &proc->regs[in->control.dregs[i].addr] : &proc->csrs[in->control.dregs[i].addr];
        *reg = in->results[i];
    }

    pipeline->fetch[0].control.stall    = false;
    pipeline->decode[0].control.stall   = false;
    pipeline->read[0].control.stall     = false;
}

static inline void riscv_state_check_control_hazard(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline)
{
    riscv_stage_execute_t* in       = &pipeline->execute[1];
    riscv_stage_execute_t* nxt      = &pipeline->execute[0];

    riscv_stage_memory_t* memory    = &pipeline->memory[0];
    riscv_stage_read_t* read        = &pipeline->read[0];
    riscv_stage_decode_t* decode    = &pipeline->decode[0];
    
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
    pipeline->fetch[1]        = pipeline->fetch[0];
    pipeline->decode[1]       = pipeline->decode[0];
    pipeline->read[1]         = pipeline->read[0];
    pipeline->execute[1]      = pipeline->execute[0];
    pipeline->memory[1]       = pipeline->memory[0];
    pipeline->writeback[1]    = pipeline->writeback[0];
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

  // Commit the results
  riscv_pipeline_commit_state(pipeline);

  if(pipeline->writeback[1].simulation.halt && pipeline->writeback[1].control.invalid == false) return sys_halt(sys);
}

#endif