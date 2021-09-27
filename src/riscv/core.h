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
    
    out->control.op           = decoded.op;
    out->control.src_regs[0]  = decoded.src_regs[0];
    out->control.src_regs[1]  = decoded.src_regs[1];
    out->control.dest_reg     = decoded.dest_reg;
    out->control.arg1_is_imm  = decoded.arg1_is_imm;

    // Fill the control block
    out->pc                     = in->pc;
    out->control.imm            = decoded.imm;
    out->control.write_pc       = decoded.write_pc;
    
    // Check any data hazards
    unsigned char write_regs[3] = {
       pipeline->execute.control.dest_reg,
       pipeline->memory.control.dest_reg,
       pipeline->writeback.control.dest_reg 
    };

    for(unsigned char i = 0; i < 3; i++) 
    {
        for(unsigned char j = 0; j < 2; j++) 
        {
            if(write_regs[i] == 0)
                continue;

            // Data hazard !
            if(write_regs[i] == pipeline->read.control.src_regs[j]) 
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
    out->args[0] = proc->regs[in->control.src_regs[0]];
    out->args[1] = proc->regs[in->control.src_regs[1]];

    // Copy target
    out->pc = in->pc;

    // Transfer control to control
    out->control.op          = in->control.op;
    out->control.src_regs[0] = in->control.src_regs[0];
    out->control.src_regs[1] = in->control.src_regs[1];
    out->control.dest_reg    = in->control.dest_reg;
    out->control.imm         = in->control.imm;
    out->control.write_pc    = in->control.write_pc;
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

    octa result, a, b, pc, store_addr, load_addr, imm;
    bool store, load;

    a = in->args[0];
    b = in->control.arg1_is_imm ? in->control.imm : in->args[1];

    pc  = in->pc;
    imm = in->control.imm;
    result  = 0;

    store = load = false;
    store_addr = load_addr = 0;

    switch(in->control.op) 
    {
        case RISCV_LUI: result = imm; break; // OK
        case RISCV_AUIPC: 
            result = pc + (imm << 12); break; // OK
        // jump
        case RISCV_JAL: 
            result = pc, pc = pc - 4 + (b << 2); 
            break; // OK
        case RISCV_JALR: 
            result = pc, pc = (a + (b << 2)) & (~3); 
            break; // OK
        // branch
        case RISCV_BEQ: 
            if(a == b) pc += (imm << 2); 
            break; // OK
        case RISCV_BNE: 
            if(a != b) pc += (imm << 2); 
            break; // OK
        case RISCV_BLTU: case RISCV_BLT: if(a < b) pc += (imm << 2); break; // OK
        case RISCV_BGEU: case RISCV_BGE: if(a >= b) pc += (imm << 2); break; // OK
        // load
        case RISCV_LBU: case RISCV_LB:
        case RISCV_LHU: case RISCV_LH:
        case RISCV_LW: case RISCV_LWU:
        case RISCV_LD: load = true, load_addr = a + b; break;
        // store
        case RISCV_SB: case RISCV_SH: case RISCV_SW: case RISCV_SD: store = true, store_addr = a + b; break;
        // add
        case RISCV_ADD:  case RISCV_ADDW: case RISCV_ADDI: case RISCV_ADDIW: result = a + b; break;
        // sub
        case RISCV_SUB: case RISCV_SUBW: result = a - b; break;
        // compare lt
        case RISCV_SLTU: case RISCV_SLTIU: case RISCV_SLTI: result = a < b; break;         
        // xor
        case RISCV_XORI: case RISCV_XOR: result =  a ^ b; break;
        // or
        case RISCV_ORI: case RISCV_OR: result = a | b; break;
        // and
        case RISCV_ANDI: case RISCV_AND:  result = a & b; break;
        // shift left
        case RISCV_SLL: case RISCV_SLLW: case RISCV_SLLI: case RISCV_SLLIW: result = a << b; break;
        // shift right
        case RISCV_SRLI: case RISCV_SRLIW: case RISCV_SRAIW: case RISCV_SRAI: case RISCV_SRL: case RISCV_SRA: case RISCV_SRAW: result = a >> b; break;
        // Halt the simulation
        case RISCV_EBREAK: sys_halt(sys);
        default: break;
    }
    
    // Write data
    out->result = result;
    out->pc = pc;
    
    // Write control
    out->control.store = store;
    out->control.store_addr = store_addr;
    out->control.load = load;
    out->control.load_addr = load_addr;
    out->control.dest_reg = in->control.dest_reg;

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
    octa result, addr;

    riscv_pipeline_t* pipeline = &proc->pipeline;
    riscv_stage_memory_t* in = &pipeline->memory;
    riscv_stage_writeback_t* out = &pipeline->writeback;

    if(in->control.stall)
        return;

    out->control.invalid = in->control.invalid;

    if(out->control.invalid)
        return;

    result = in->result;
    addr = 0;

    if(in->control.store) addr = in->control.store_addr;
    if(in->control.load) addr = in->control.load_addr;

    switch(in->control.op) 
    {
        // load
        case RISCV_LBU: case RISCV_LB: sys_load_byte(sys, (void*) addr, (byte*) &result); break;
        case RISCV_LHU: case RISCV_LH: sys_load_word(sys, (void*) addr, (word*) &result); break;
        case RISCV_LW: case RISCV_LWU: sys_load_tetra(sys, (void*) addr, (tetra*) &result); break;
        case RISCV_LD: sys_load_octa(sys, (void*) addr, &result); break;
        // store
        case RISCV_SB: sys_store_byte(sys, (void*) addr, result); break;
        case RISCV_SH: sys_store_word(sys, (void*) addr, result); break;
        case RISCV_SW: sys_store_tetra(sys, (void*) addr, result); break;
        case RISCV_SD: sys_store_octa(sys, (void*) addr, result); break;
        default: break;
    }

    out->result = result;
    out->control.dest_reg = in->control.dest_reg;
}

static inline void __write(system_t* sys, riscv_processor_t* proc)
{
    riscv_pipeline_t* pipeline = &proc->pipeline;
    riscv_stage_writeback_t* in = &pipeline->writeback;

    if(in->control.stall)
        return;

    if(in->control.invalid)
        return;

    proc->regs[in->control.dest_reg] = in->result;

    pipeline->fetch.control.stall   = false;
    pipeline->decoder.control.stall = false;
    pipeline->read.control.stall    = false;
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