#ifndef __RISCV_PIPELINE_API_H__
#define __RISCV_PIPELINE_API_H__

#include "../model.h"
#include "../opcode.h"
#include "../instr.h"
#include "../csr.h"
#include "./model.h"

#include "../../../lib/common/include/transaction.h"
#include "../../../lib/common/include/alu.h"

void riscv_pipeline_stage_fetch_create(riscv_stage_fetch_t* fetch);
void riscv_pipeline_stage_decode_create(riscv_stage_decode_t* decode);
void riscv_pipeline_stage_read_create(riscv_stage_read_t* read);
void riscv_pipeline_stage_execute_create(riscv_stage_execute_t* execute);
void riscv_pipeline_stage_memory_create(riscv_stage_memory_t* memory);
void riscv_pipeline_stage_writeback_create(riscv_stage_writeback_t* writeback);

void riscv_pipeline_create(riscv_pipeline_t* pipeline);

static inline void riscv_stage_fetch_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction);
static inline void riscv_stage_decode_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction);
static inline void riscv_stage_read_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction);
static inline void riscv_stage_execute_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction);
static inline void riscv_stage_memory_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction);
static inline void riscv_stage_writeback_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction);

void riscv_pipeline_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction);

void riscv_pipeline_create(riscv_pipeline_t* pipeline)
{
    riscv_pipeline_stage_fetch_create(&pipeline->fetch);
    riscv_pipeline_stage_decode_create(&pipeline->decode);
    riscv_pipeline_stage_read_create(&pipeline->read);
    riscv_pipeline_stage_execute_create(&pipeline->execute);
    riscv_pipeline_stage_memory_create(&pipeline->memory);
    riscv_pipeline_stage_writeback_create(&pipeline->writeback);  
}

void riscv_load_csr(system_t* sys, riscv_processor_t* proc, unsigned int addr, octa* out)
{
    if(addr == SIE) {
        *out = proc->csrs[MIE] & proc->csrs[MIDELEG];
    } else {
        *out = proc->csrs[addr % 4096];
    }
}
void riscv_store_csr(system_t* sys, riscv_processor_t* proc, unsigned int addr, const octa value)
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
    memory->control.wait = true;
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

static inline void riscv_stage_fetch_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction)
{
    riscv_stage_fetch_t* in     = &pipeline->fetch;
    riscv_stage_decode_t* out   = &pipeline->decode;

    if(in->control.stall)
        return;        

    tetra raw;

    bool cache_miss = data_cache_read_tetra(&proc->l1, proc->pc, &raw, transaction);

    // We have a cache miss, we need to fetch data
    if(cache_miss) 
    {
        tst_update_tetra(transaction, &out->raw, 0);
    } else {
        tst_update_octa(transaction, &proc->pc, proc->pc + 4);
        tst_update_octa(transaction, &out->debug.current_pc, proc->pc);
        tst_update_octa(transaction, &out->pc, proc->pc + 4);
        tst_update_tetra(transaction, &out->raw, raw);
        tst_update_bool(transaction, &out->control.invalid, false);
    }
}
static inline void riscv_stage_decode_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction)
{
    riscv_stage_decode_t* in = &pipeline->decode;
    riscv_stage_read_t* out = &pipeline->read;

    if(in->control.stall)
        return;

    tst_update_bool(transaction, &out->control.invalid, in->control.invalid);

    if(in->control.invalid)
        return;

    // Setup control
    riscv_decoded_instr_t decoded = decode(in->raw);
    
    tst_update_int(transaction, &out->control.op, decoded.op);
    tst_update_char(transaction, &out->control.sregs[0].type, decoded.sregs[0].type);
    tst_update_int(transaction, &out->control.sregs[0].addr, decoded.sregs[0].addr);
    tst_update_char(transaction, &out->control.sregs[1].type, decoded.sregs[1].type);
    tst_update_int(transaction, &out->control.sregs[1].addr, decoded.sregs[1].addr);
    tst_update_char(transaction, &out->control.dregs[0].type, decoded.dregs[0].type);
    tst_update_int(transaction, &out->control.dregs[0].addr, decoded.dregs[0].addr);
    tst_update_char(transaction, &out->control.dregs[1].type, decoded.dregs[1].type);
    tst_update_int(transaction, &out->control.dregs[1].addr, decoded.dregs[1].addr);
    tst_update_bool(transaction, &out->control.arg1_is_imm, decoded.arg1_is_imm);

    // Fill the control block
    tst_update_octa(transaction, &out->pc, in->pc);
    tst_update_octa(transaction, &out->control.imm, decoded.imm);
    tst_update_bool(transaction, &out->control.write_pc, decoded.write_pc);

    // Fill the debug block
    tst_update_octa(transaction, &out->debug.current_pc, in->debug.current_pc);

    // Check any data hazards
    riscv_reg_addr_t forward_regs[6] = {
       pipeline->execute.control.dregs[0],
       pipeline->execute.control.dregs[1],
       pipeline->memory.control.dregs[0],
       pipeline->memory.control.dregs[1],
       pipeline->writeback.control.dregs[0], 
       pipeline->writeback.control.dregs[1]
    };

    for(unsigned char i = 0; i < 6; i++) 
    {
        for(unsigned char j = 0; j < 2; j++) 
        {
            if(forward_regs[i].addr == 0 && forward_regs[i].type == 0)
                continue;
 
            // Data hazard !
            if(forward_regs[i].type == pipeline->read.control.sregs[j].type && forward_regs[i].addr == pipeline->read.control.sregs[j].addr) 
            {
                tst_update_bool(transaction, &pipeline->fetch.control.stall,    true);
                tst_update_bool(transaction, &pipeline->decode.control.stall,   true);
                tst_update_bool(transaction, &pipeline->read.control.stall,     true);
                break;
            }
        }
    }
}
static inline void riscv_stage_read_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction)
{
    riscv_stage_read_t* in = &pipeline->read;
    riscv_stage_execute_t* out = &pipeline->execute;

    if(in->control.stall)
        return;

    tst_update_bool(transaction, &out->control.invalid, in->control.invalid);

    if(in->control.invalid) 
        return;

    // Read registers
    tst_update_octa(transaction, &out->args[0], in->control.sregs[0].type == 0 ? proc->regs[in->control.sregs[0].addr]: proc->csrs[in->control.sregs[0].addr]);
    tst_update_octa(transaction, &out->args[1], in->control.sregs[1].type == 0 ? proc->regs[in->control.sregs[1].addr]: proc->csrs[in->control.sregs[1].addr]);

    // Copy target
    tst_update_octa(transaction, &out->pc, in->pc);

    // Transfer control to control
    tst_update_int(transaction, &out->control.op, in->control.op);

    tst_update_char(transaction, &out->control.sregs[0].type, in->control.sregs[0].type);
    tst_update_int(transaction, &out->control.sregs[0].addr, in->control.sregs[0].addr);
    tst_update_char(transaction, &out->control.sregs[1].type, in->control.sregs[1].type);
    tst_update_int(transaction, &out->control.sregs[1].addr, in->control.sregs[1].addr);
    tst_update_char(transaction, &out->control.dregs[0].type, in->control.dregs[0].type);
    tst_update_int(transaction, &out->control.dregs[0].addr, in->control.dregs[0].addr);
    tst_update_char(transaction, &out->control.dregs[1].type, in->control.dregs[1].type);
    tst_update_int(transaction, &out->control.dregs[1].addr, in->control.dregs[1].addr);
    tst_update_octa(transaction, &out->control.imm, in->control.imm);
    tst_update_bool(transaction, &out->control.write_pc, in->control.write_pc);
    tst_update_bool(transaction, &out->control.arg1_is_imm, in->control.arg1_is_imm);

    // Fill the debug block
    tst_update_octa(transaction, &out->debug.current_pc, in->debug.current_pc);

}
static inline void riscv_stage_execute_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction)
{
    riscv_stage_execute_t* in = &pipeline->execute;
    riscv_stage_memory_t* out = &pipeline->memory;

    if(in->control.stall)
        return;

    tst_update_bool(transaction, &out->simulation.halt, false);
    tst_update_bool(transaction, &out->control.invalid, in->control.invalid);

    if(in->control.invalid)
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
        case RISCV_AUIPC: 
            result[0] = octa_plus_expr(pc - 4 , octa_left_shift_expr(imm, 12)); 
        break;
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
        case RISCV_EBREAK: tst_update_bool(transaction, &out->simulation.halt, true);
        case RISCV_FENCE_I: break;
        case RISCV_CSRRW: 
            result[0] = b, result[1] = a; 
            break;
        default: break;
    }
    
    // Write data
    tst_update_octa(transaction, &out->results[0], result[0]);
    tst_update_octa(transaction, &out->results[1], result[1]);
    tst_update_octa(transaction, &out->pc, pc);
    
    // Write control
    tst_update_int(transaction, &out->control.op, in->control.op);
    tst_update_char(transaction, &out->control.memory_op.op, memory_op.op);
    tst_update_octa(transaction, &out->control.memory_op.addr, memory_op.addr);

    tst_update_char(transaction, &out->control.dregs[0].type, in->control.dregs[0].type);
    tst_update_int(transaction, &out->control.dregs[0].addr,  in->control.dregs[0].addr);
    tst_update_char(transaction, &out->control.dregs[1].type, in->control.dregs[1].type);
    tst_update_int(transaction, &out->control.dregs[1].addr,  in->control.dregs[1].addr);

    // Fill the debug block
    tst_update_octa(transaction, &out->debug.current_pc, in->debug.current_pc);

}
static inline void riscv_stage_memory_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction)
{
    octa result[2];
    octa addr;

    riscv_stage_memory_t* in = &pipeline->memory;
    riscv_stage_writeback_t* out = &pipeline->writeback;

    if(in->control.stall)
        return;

    tst_update_bool(transaction, &out->simulation.halt, false);
    tst_update_bool(transaction, &out->control.invalid, in->control.invalid);

    if(out->control.invalid)
        return;

    result[0] = in->results[0];
    result[1] = in->results[1];
    
    addr = in->control.memory_op.addr;

    bool cache_miss = false;

    switch(in->control.op) 
    {
        // load
        case RISCV_LBU: case RISCV_LB: cache_miss = data_cache_read(&proc->l1, addr, (byte*) &result[0], transaction); break;
        case RISCV_LHU: case RISCV_LH: cache_miss = data_cache_read_word(&proc->l1, addr, (word*) &result[0], transaction); break;
        case RISCV_LW: case RISCV_LWU: cache_miss = data_cache_read_tetra(&proc->l1, addr, (tetra*) &result[0], transaction); break;
        case RISCV_LD: cache_miss = data_cache_read_octa(&proc->l1, addr, &result[0], transaction); break;
        // store
        case RISCV_SB: cache_miss = data_cache_write(&proc->l1, addr, (byte) result[0], transaction); break;
        case RISCV_SH: cache_miss = data_cache_write_word(&proc->l1, addr, (word) result[0], transaction); break;
        case RISCV_SW: cache_miss = data_cache_write_tetra(&proc->l1, addr, (tetra) result[0], transaction); break;
        case RISCV_SD: cache_miss = data_cache_write_octa(&proc->l1, addr, result[0], transaction); break;
    }

    // We need to wait
    if(cache_miss) {
        in->control.wait = true;
        // Invalid the rest of the pipeline
        tst_update_bool(transaction, &out->control.invalid, true);
        
        tst_update_octa(transaction, &out->results[0], 0);
        tst_update_octa(transaction, &out->results[1], 0);
        
        // Write control
        tst_update_char(transaction, &out->control.dregs[0].type, 0);
        tst_update_int(transaction, &out->control.dregs[0].addr,  0);
        tst_update_char(transaction, &out->control.dregs[1].type, 0);
        tst_update_int(transaction, &out->control.dregs[1].addr,  0);

        // Write debug
        tst_update_octa(transaction, &out->debug.current_pc, 0);
    } else {
        in->control.wait = false;
        //
        tst_update_octa(transaction, &out->results[0], result[0]);
        tst_update_octa(transaction, &out->results[1], result[1]);
        
        // Write control
        tst_update_char(transaction, &out->control.dregs[0].type, in->control.dregs[0].type);
        tst_update_int(transaction, &out->control.dregs[0].addr,  in->control.dregs[0].addr);
        tst_update_char(transaction, &out->control.dregs[1].type, in->control.dregs[1].type);
        tst_update_int(transaction, &out->control.dregs[1].addr,  in->control.dregs[1].addr);

        // Write debug
        tst_update_octa(transaction, &out->debug.current_pc, in->debug.current_pc);
    }
}
static inline void riscv_stage_writeback_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction)
{
   riscv_stage_writeback_t* in = &pipeline->writeback;

    if(in->control.stall)
        return;

    if(in->control.invalid)
        return;

    for(unsigned char i = 0; i < 2; i++) 
    {
        octa* reg = in->control.dregs[i].type == 0 ? &proc->regs[in->control.dregs[i].addr] : &proc->csrs[in->control.dregs[i].addr];
        tst_update_octa(transaction, reg, in->results[i]);
    }
}

static inline void riscv_check_memory_wait(riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction)
{
    riscv_stage_memory_t* memory    = &pipeline->memory;
    riscv_stage_execute_t* execute  = &pipeline->execute;
    riscv_stage_read_t* read        = &pipeline->read;
    riscv_stage_decode_t* decode    = &pipeline->decode;
    riscv_stage_fetch_t* fetch      = &pipeline->fetch;

    if(memory->control.wait) 
    {
        // Invalid all changes
        tst_update_octa(transaction, &proc->pc, proc->pc);
        tst_update_riscv_stage_fetch(transaction, fetch, *fetch);
        tst_update_riscv_stage_decode(transaction, decode, *decode);
        tst_update_riscv_stage_read(transaction, read, *read);
        tst_update_riscv_stage_execute(transaction, execute, *execute);
        tst_update_riscv_stage_memory(transaction, memory, *memory);
    }
}

static inline void riscv_check_control_hazard(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction)
{
    riscv_stage_execute_t* execute  = &pipeline->execute;
    riscv_stage_memory_t* memory    = &pipeline->memory;
    riscv_stage_read_t* read        = &pipeline->read;
    riscv_stage_decode_t* decode    = &pipeline->decode;
    
    // Write target
    if(execute->control.write_pc == true && read->pc - 4 != memory->pc)
    {
        tst_update_octa(transaction, &proc->pc, memory->pc);
        tst_update_bool(transaction, &decode->control.invalid, true);
        tst_update_bool(transaction, &read->control.invalid, true);
        tst_update_bool(transaction, &execute->control.invalid, true);
    }
}

void riscv_pipeline_step(system_t* sys, riscv_processor_t* proc, riscv_pipeline_t* pipeline, transaction_t* transaction)
{
  riscv_stage_fetch_step(sys, proc, pipeline, transaction);
  riscv_stage_decode_step(sys, proc, pipeline, transaction);
  riscv_stage_read_step(sys, proc, pipeline, transaction);
  riscv_stage_execute_step(sys, proc, pipeline, transaction);
  riscv_stage_memory_step(sys, proc, pipeline, transaction);
  riscv_stage_writeback_step(sys, proc, pipeline, transaction);

  riscv_check_control_hazard(sys, proc, pipeline, transaction);
  riscv_check_memory_wait(proc, pipeline, transaction);

  if(pipeline->writeback.simulation.halt && pipeline->writeback.control.invalid == false) sys_halt(sys);
}


#endif