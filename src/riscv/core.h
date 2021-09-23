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
    proc->regs[2]   = cfg->memory_size;
    proc->regs[0]   = octa_zero;

}

static inline bool __fetch(system_t* sys, riscv_processor_t* proc)
{
    riscv_stage_fetch_t* in = &proc->fetch;

    if(in->control.stall) return false;

    riscv_stage_decoder_t* out = &proc->decoder;

    tetra raw;
    void* vaddr = (void*) proc->pc;
    
    if(!sys_load_tetra(sys, vaddr, &raw))
    {
        out->pc = proc->pc + 4;
        out->raw = 0;
    }
    
    out->pc = proc->pc + 4;
    out->raw = raw;
    
    in->control.stall = true;

    return true;
}

static inline void __decode(system_t* sys, riscv_processor_t* proc)
{
    // Setup control
    proc->read.control.instr        = decode(proc->decoder.raw);
    proc->read.control.src_regs[0]  = proc->read.control.instr.src_regs[0];
    proc->read.control.src_regs[1]  = proc->read.control.instr.src_regs[1];
    proc->read.control.dest_reg     = proc->read.control.instr.dest_reg;
    proc->read.pc                   = proc->decoder.pc;
    proc->read.control.imm          = proc->read.control.instr.imm;

    // Check any data hazards
    unsigned char write_regs[3] = {
       proc->execute.control.dest_reg,
       proc->memory.control.dest_reg,
       proc->writeback.control.dest_reg 
    };

    for(unsigned char i = 0; i < 3; i++) 
    {
        for(unsigned char j = 0; j < 2; j++) 
        {
            if(write_regs[i] == 0)
                continue;

            // Data hazard !
            if(write_regs[i] == proc->read.control.src_regs[j]) 
            {
                proc->fetch.control.stall = true;
                proc->decoder.control.stall = true;
                proc->read.control.stall = true;
                break;
            }
        }
    }
}

static inline void __read(system_t* sys, riscv_processor_t* proc)
{
    riscv_stage_read_t* in = &proc->read;
    riscv_stage_execute_t* out = &proc->execute;

    // Read registers
    out->args[0] = proc->regs[in->control.src_regs[0]];
    out->args[1] = proc->regs[in->control.src_regs[1]];

    // Transfer control to control
    out->control.instr = in->control.instr;
    out->control.src_regs[0] = in->control.src_regs[0];
    out->control.src_regs[1] = in->control.src_regs[1];
    out->control.dest_reg = in->control.dest_reg;
    out->control.imm = in->control.imm;

}

static inline void __execute(system_t* sys, riscv_processor_t* proc)
{
    riscv_stage_execute_t* in = &proc->execute;
    riscv_stage_memory_t* out = &proc->memory;

    octa result, a, b, pc, store_addr, load_addr, imm;
    bool store, load;

    a = in->args[0];
    b = in->args[1];

    pc = in->pc;
    imm = in->control.imm;
    result  = 0;

    store = load = false;
    store_addr = load_addr = 0;

    switch(in->control.instr.op) 
    {
        case RISCV_LUI: result = imm; break; // OK
        case RISCV_AUIPC: result = pc + (imm << 12); break; // OK
        // jump
        case RISCV_JAL: result = pc + 4, pc = pc + (imm << 1); break; // OK
        case RISCV_JALR: result = pc, pc = (pc + imm) & (~1); break; // OK
        // branch
        case RISCV_BEQ: if(a == b) pc += imm + 4; break; // OK
        case RISCV_BNE: if(a != b) pc += imm + 4; break; // OK
        case RISCV_BLTU: case RISCV_BLT: if(a < b) pc += imm + 4; break; // OK
        case RISCV_BGEU: case RISCV_BGE: if(a >= b) pc += imm + 4; break; // OK
        // load
        case RISCV_LBU: case RISCV_LB:
        case RISCV_LHU: case RISCV_LH:
        case RISCV_LW: case RISCV_LWU:
        case RISCV_LD: load = true, load_addr = a + b; break;
        // store
        case RISCV_SB: case RISCV_SH: case RISCV_SW: case RISCV_SD: store = true, store_addr = a + b; break;
        // add
        case RISCV_ADD:  case RISCV_ADDW: result = a + b; break;
        case RISCV_ADDI: case RISCV_ADDIW: result = a + imm; break;
        // sub
        case RISCV_SUB: case RISCV_SUBW: result = a - b; break;
        // compare lt
        case RISCV_SLTU:  result = a < b; break;         
        case RISCV_SLTIU: case RISCV_SLTI: result = a < imm; break;
        // xor
        case RISCV_XOR: result =  a ^ b; break;
        case RISCV_XORI: result = a ^ imm; break;
        // or
        case RISCV_OR: result = a | b; break;
        case RISCV_ORI: result = a | imm; break;
        // and
        case RISCV_AND:  result = a & b; break;
        case RISCV_ANDI: result = a & b; break;
        // shift left
        case RISCV_SLL: case RISCV_SLLW:  result = a << b; break;
        case RISCV_SLLI: case RISCV_SLLIW: result = a << imm; break;
        // shift right
        case RISCV_SRL: case RISCV_SRA: case RISCV_SRAW: result = a >> b; break;
        case RISCV_SRLI: case RISCV_SRLIW: case RISCV_SRAIW: case RISCV_SRAI: result = a >> imm; break;
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
    out->control.instr = in->control.instr;
    out->control.dest_reg = in->control.dest_reg;

    // Write target
    proc->pc = out->pc;

    // Unstall
    proc->fetch.control.stall = false;
}

static inline void __memory(system_t* sys, riscv_processor_t* proc)
{
    riscv_stage_memory_t* in = &proc->memory;
    riscv_stage_writeback_t* out = &proc->writeback;

    octa result = in->result;
    octa addr = 0;

    if(in->control.store) addr = in->control.store_addr;
    if(in->control.load) addr = in->control.load_addr;

    switch(in->control.instr.op) 
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
    riscv_stage_writeback_t* in = &proc->writeback;
    proc->regs[in->control.dest_reg] = in->result;

    proc->fetch.control.stall   = false;
    proc->decoder.control.stall = false;
    proc->read.control.stall    = false;
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