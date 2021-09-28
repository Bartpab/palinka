#ifndef __RISCV_CORE_H__
#define __RISCV_CORE_H__

#include "../../lib/common/include/allocator.h"
#include "../system.h"

#include "./processor.h"
#include "./opcode.h"
#include "./control.h"
#include "./csr.h"

#define RISCV_START_ADDRESS 0x20000000

system_t* riscv_new(allocator_t* allocator, riscv_processor_cfg_t* cfg);
void riscv_alloc_sim_time(system_t* sys, unsigned int ms);
void riscv_step(system_t* sys);

static void __riscv_init(system_t* sys, riscv_processor_cfg_t* cfg);

static inline bool __fetch(system_t* sys, riscv_processor_t* proc);
static inline void __decode(system_t* sys, riscv_processor_t* proc);
static inline void __read(system_t* sys, riscv_processor_t* proc);
static inline void __execute(system_t* sys, riscv_processor_t* proc);
static inline void __memory(system_t* sys, riscv_processor_t* proc);
static inline void __write(system_t* sys, riscv_processor_t* proc);

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
   
    proc->pc        = cfg->boot_address;

    for(int i = 0; i < 32; i++) {
        proc->regs[i] = 0;
    }

    proc->regs[2]   = cfg->memory_size;
    proc->regs[0]   = octa_zero;

    riscv_pipeline_create(&proc->pipeline);
}

static inline bool __fetch(system_t* sys, riscv_processor_t* proc)
{
    riscv_pipeline_t* pipeline  = &proc->pipeline;
    riscv_stage_fetch_t* in     = &pipeline->fetch;
    riscv_stage_decoder_t* out  = &pipeline->decoder;

    if(in->control.stall)
        return false;        

    tetra raw;
    void* vaddr = (void*) proc->pc;
    
    if(!sys_load_tetra(sys, vaddr, &raw))
    {
        out->pc = proc->pc;
        out->raw = 0; // NOP
        sys_halt(sys);
    }
    
    proc->pc += 4;
    out->pc = proc->pc;
    out->raw = raw;
    out->control.invalid = false;

    return true;
}

static inline void __decode(system_t* sys, riscv_processor_t* proc)
{
    riscv_pipeline_t* pipeline      = &proc->pipeline;
    riscv_stage_decoder_t* in       = &pipeline->decoder;
    riscv_stage_read_t* out         = &pipeline->read;

    out->control.invalid = in->control.invalid;

    if(out->control.invalid)
        return;

    if(in->control.stall)
        return;

    // Setup control
    riscv_decoded_instr_t decoded   = decode(in->raw);
    
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
                pipeline->fetch.control.stall   = true;
                pipeline->decoder.control.stall = true;
                pipeline->read.control.stall    = true;
                break;
            }
        }
    }
}

static inline void __read(system_t* sys, riscv_processor_t* proc)
{
    riscv_pipeline_t* pipeline = &proc->pipeline;

    riscv_stage_read_t* in = &pipeline->read;
    riscv_stage_execute_t* out = &pipeline->execute;

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

}

static inline void __execute(system_t* sys, riscv_processor_t* proc)
{
    riscv_pipeline_t* pipeline = &proc->pipeline;

    riscv_stage_execute_t* in = &pipeline->execute;
    riscv_stage_memory_t* out = &pipeline->memory;

    if(in->control.stall)
        return;

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
        case RISCV_AUIPC: result[0] = octa_plus_expr(pc, octa_left_shift_expr(imm, 12)); break;
        // jump
        case RISCV_JAL: result[0] = pc, pc = octa_plus_expr(octa_incr_expr(pc, -4), octa_left_shift_expr(b, 2)); break; // OK
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
        case RISCV_EBREAK: sys_halt(sys);
        case RISCV_FENCE_I: break;
        case RISCV_CSRRW: break;
        default: break;
    }
    
    // Write data
    out->results[0] = result[0];
    out->results[1] = result[1];

    out->pc = pc;
    
    // Write control
    out->control.op = in->control.op;
    out->control.memory_op = memory_op;
    out->control.dregs[0] = in->control.dregs[0];
    out->control.dregs[1] = in->control.dregs[1];

    // Write target
    if(in->control.write_pc && pipeline->read.pc != out->pc) 
    {
        proc->pc = out->pc;
        pipeline->decoder.control.invalid = true;
        pipeline->read.control.invalid = true;
    }
}

static inline void __memory(system_t* sys, riscv_processor_t* proc)
{
    octa result[2];
    void* addr;

    riscv_pipeline_t* pipeline = &proc->pipeline;
    riscv_stage_memory_t* in = &pipeline->memory;
    riscv_stage_writeback_t* out = &pipeline->writeback;

    if(in->control.stall)
        return;

    out->control.invalid = in->control.invalid;

    if(out->control.invalid)
        return;

    result[0] = in->results[0];
    result[1] = in->results[1];
    addr = (void*) in->control.memory_op.addr;

    switch(in->control.op) 
    {
        // load
        case RISCV_LBU: case RISCV_LB: 
            sys_load_byte(sys, addr, (byte*) &result); 
            break;
        case RISCV_LHU: case RISCV_LH: 
            sys_load_word(sys, addr, (word*) &result); 
            break;
        case RISCV_LW: case RISCV_LWU: sys_load_tetra(sys, addr, (tetra*) &result); break;
        case RISCV_LD: sys_load_octa(sys, addr, &result[0]); break;
        // store
        case RISCV_SB: 
            sys_store_byte(sys, addr, result[0]); 
            break;
        case RISCV_SH: sys_store_word(sys, addr, result[0]); break;
        case RISCV_SW: sys_store_tetra(sys, addr, result[0]); break;
        case RISCV_SD: 
            sys_store_octa(sys, addr, result[0]); 
            break;
        default: break;
    }

    out->results[0] = result[0];
    out->results[1] = result[1];
    out->control.dregs[0] = in->control.dregs[0];
    out->control.dregs[1] = in->control.dregs[1];
}

static inline void __write(system_t* sys, riscv_processor_t* proc)
{
    riscv_pipeline_t* pipeline = &proc->pipeline;
    riscv_stage_writeback_t* in = &pipeline->writeback;

    if(in->control.stall)
        return;

    if(in->control.invalid)
        return;
    
    for(unsigned char i = 0; i < 2; i++) 
    {
        octa* reg = in->control.dregs[i].type == 0 ? &proc->regs[in->control.dregs[i].addr] : &proc->csrs[in->control.dregs[i].addr];
        *reg = in->results[i];
    }

    pipeline->fetch.control.stall = false;
    pipeline->decoder.control.stall = false;
    pipeline->read.control.stall = false;
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
    
    __fetch(sys, proc);
    __decode(sys, proc);
    __read(sys, proc);
    __execute(sys, proc);
    __memory(sys, proc);
    __write(sys, proc);

    if(proc->remaining_cycles > 0) proc->remaining_cycles--;
}


#endif