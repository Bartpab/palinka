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

static inline bool __fetch(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr);
static inline void __decode(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr);
static inline void __read(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr);
static inline void __execute(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr);
static inline void __memory(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr);
static inline void __write(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr);

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

    proc->current_control.stage = RISCV_FETCH;
}

static inline bool __fetch(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr)
{
    tetra raw;
    void* vaddr = (void*) proc->pc;
    
    if(!sys_load_tetra(sys, vaddr, &raw))
        return false;
    
    instr->raw = raw;
    proc->pc += 4;
    return true;
}

/**
 * 
 * 4 types of RISC-V instructions
 * 
 * R-TYPE
 * 
 * 31       25 24    20 19     15 14       12 11    7 6       0
 * | funct 7  | rs2    | rs1     | funct3    | rd    | opcode |
 * 
 * I-TYPE
 * 
 * 31                   19     15 14       12 11    7 6       0
 * | imm[11:0]         | rs1     | funct3    | rd    | opcode |
 * 
 * S-TYPE
 * 
 * 31        25 24  20 19      15 14       12 11      7 6     0
 * | imm[11:5] | rs2  | rs1      | funct3    |imm[4:0] |opcode|
 * 
 * U-TYPE
 * 
 * 31                                      12 11     7 6      0
 * | imm[32:12]                              | rd     | opcode|
 */
static inline void __decode(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr)
{
    tetra raw = instr->raw;

    byte opcode = raw & 0x7f;
    byte rd = (raw >> 7) & 0x1F;
    byte funct3 = (raw >> 12) & 0x7;
    byte rs1 = (raw >> 15) & 0x1F;
    byte rs2 = (raw >> 20) & 0x1F;
    byte funct7 = (raw >> 25) & 0x7F;

    instr->decoded.opcode = opcode;
    instr->decoded.rd = rd;
    instr->decoded.funct3 = funct3;
    instr->decoded.funct7 = funct7;

    instr->decoded.rs1 = rs1;
    instr->decoded.rs2 = rs2;
    instr->decoded.shamt = 0;
    
    instr->op = 0;

    for(unsigned char i = 0; i < 3; i++) 
    {
        instr->out[i].flag = false;
        instr->out[i].ptr = NULL;
        instr->out[i].value = 0;
    }

    for(unsigned char i = 0; i < 4; i++)
    {
        instr->args[i].flag = false;
        instr->args[i].ptr   = NULL;
        instr->args[i].value = 0;
    }

    switch(instr->decoded.opcode) {
        case 0b0110111: instr->op = RISCV_LUI; goto u_type;
        case 0b0010111: instr->op = RISCV_AUIPC; goto u_type;
        case 0b1101111: instr->op = RISCV_JAL; goto j_type;
        case 0b1100111: instr->op = RISCV_JALR; goto b_type;
        case 0b1100011:
            switch(instr->decoded.funct3) {
                case 0b000: instr->op = RISCV_BEQ; goto b_type;
                case 0b001: instr->op = RISCV_BNE; goto b_type;
                case 0b100: instr->op = RISCV_BLT; goto b_type;
                case 0b101: instr->op = RISCV_BGE; goto b_type;
                case 0b110: instr->op = RISCV_BLTU; goto b_type;
                case 0b111: instr->op = RISCV_BGEU; goto b_type;
                default: goto __end;
            }
            break;
        case 0b0000011:
            switch(instr->decoded.funct3) {
                case 0b000: instr->op = RISCV_LB; goto i_type;
                case 0b001: instr->op = RISCV_LH; goto i_type;
                case 0b010: instr->op = RISCV_LW; goto i_type;
                case 0b100: instr->op = RISCV_LBU; goto i_type;
                case 0b101: instr->op = RISCV_LHU; goto i_type;
                default: goto __end;
            }
        case 0b0100011:
            switch(instr->decoded.funct3) {
                case 0b000: instr->op = RISCV_SB; goto s_type;
                case 0b001: instr->op = RISCV_SH; goto s_type;
                case 0b010: instr->op = RISCV_SW; goto s_type;
                default: goto __end;
            } 
            break;
        case 0b0010011:
            switch(instr->decoded.funct3) {
                case 0b000: instr->op = RISCV_ADDI; goto i_type;
                case 0b010: instr->op = RISCV_SLTI; goto i_type;
                case 0b011: instr->op = RISCV_SLTIU; goto i_type;
                case 0b100: instr->op = RISCV_XORI; goto i_type;
                case 0b110: instr->op = RISCV_ORI; goto i_type;
                case 0b111: instr->op = RISCV_ANDI; goto i_type;
                case 0b001:
                    switch(instr->decoded.funct7) {
                        case 0b0000000: instr->op = RISCV_SLLI; goto __end;
                        default: goto __end;
                    }
                case 0b101:
                    switch(instr->decoded.funct7) {
                        case 0b0000000: instr->op = RISCV_SRLI; goto __end;
                        case 0b0100000: instr->op = RISCV_SRAI; goto __end;
                        default: goto __end;
                    }
                default: goto __end;
            }
        case 0b0110011:
            switch(instr->decoded.funct3) {
                case 0b000:
                    switch(instr->decoded.funct7) {
                        case 0b0000000: instr->op = RISCV_ADD; goto __end;
                        case 0b0100000: instr->op = RISCV_SUB; goto __end;
                        default: goto __end;
                    }
                case 0b001:
                    switch(instr->decoded.funct7) {
                        case 0b0000000: instr->op = RISCV_SLL; goto __end;
                        default: goto __end;
                    }
                case 0b010:
                    switch(instr->decoded.funct7) {
                        case 0b0000000: instr->op = RISCV_SLT; goto __end;
                        default: goto __end;
                    }
                case 0b011:
                    switch(instr->decoded.funct7) {
                        case 0b0000000: instr->op = RISCV_SLTU; goto __end;
                        default: goto __end;
                    }
                case 0b100:
                    switch(instr->decoded.funct7) {
                        case 0b0000000: instr->op = RISCV_XOR; goto __end;
                        default: goto __end;
                    }
                case 0b101:
                    switch(instr->decoded.funct7) {
                        case 0b0000000: instr->op = RISCV_SRL; goto __end;
                        case 0b0100000: instr->op = RISCV_SRA; goto __end;
                        default: goto __end;
                    }
                case 0b110:
                    switch(instr->decoded.funct7) {
                        case 0b0000000: instr->op = RISCV_OR; goto __end;
                        default: goto __end;
                    }
                case 0b111:
                    switch(instr->decoded.funct7) {
                        case 0b0000000: instr->op = RISCV_AND; goto __end;
                        default: goto __end;
                    } 
                default: goto __end;
            }
            break;
        case 0b0001111: instr->op = RISCV_FENCE; goto __end;
        case 0b1110011:
            if(instr->decoded.rd == 0 && instr->decoded.funct3 == 0 && instr->decoded.rs1 == 0) {
                switch(instr->decoded.rs2) {
                    case 0x0: instr->op = RISCV_ECALL; goto __end;
                    case 0x1: instr->op = RISCV_EBREAK; goto __end;
                    default: goto __end;
                }
            }
            goto __end;
        case 0b0011011:
            switch(instr->decoded.funct3) {
                case 0b000: instr->op = RISCV_ADDIW; goto __end;
                case 0b001: instr->op = RISCV_SLLIW; goto __end;
                case 0b101: 
                    switch(instr->decoded.funct7) {
                        case 0x00: instr->op = RISCV_SRLIW; goto __end;
                        case 0x20: instr->op = RISCV_SRAIW; goto __end;
                        default: goto __end;
                    }
                default: goto __end;
            }
            break;

        i_type: 
            instr->decoded.imm = (instr->raw >> 20) & 1;
            instr->decoded.imm |= (((octa)(instr->raw) >> 21) & 0x7ff) << 1;
            break;
        s_type:
            instr->decoded.imm = (instr->raw >> 7) & 1;
            instr->decoded.imm |= ((instr->raw >> 8) & 0xf) << 1;
            instr->decoded.imm |= ((instr->raw >> 25) & 0x40) << 5;
            break;
        b_type:
            instr->decoded.imm = ((instr->raw >> 8) & 0xF) << 1;
            instr->decoded.imm |= ((instr->raw >> 25) & 0x3F) << 5;
            instr->decoded.imm |= ((instr->raw >> 7) & 1) << 11;
            instr->decoded.imm |= ((instr->raw >> 31) & 1) << 12;
            break;
        u_type:
            instr->decoded.imm = ((instr->raw >> 12) & 0xFF) << 12;
            instr->decoded.imm |= ((instr->raw >> 20) & 0x7FF) << 20;
            instr->decoded.imm |= ((instr->raw >> 31) & 1) << 31;
            break;
        j_type:
            instr->decoded.imm = ((instr->raw >> 21) & 0x3FF) << 1;
            instr->decoded.imm |= ((instr->raw >> 20) & 1) << 11;
            instr->decoded.imm |= ((instr->raw >> 12) & 0x7F) << 12;
            instr->decoded.imm |= (((octa)(instr->raw) >> 32) & 1) << 31;
            break;
        __end:
            break;
    }

    instr->infos =  &riscv_instr_infos[instr->op];

    int flags = instr->infos->flags;
    
    // Set controls info for ARG0
    if((flags & ARG0_IS_RS1) == ARG0_IS_RS1)
    {
        instr->args[0].flag = true;
        instr->args[0].value = proc->regs[instr->decoded.rs1];
        instr->args[0].ptr = &proc->regs[instr->decoded.rs1];
    }

    else if((flags & ARG0_IS_RS2) == ARG0_IS_RS2) {
        instr->args[0].value = proc->regs[instr->decoded.rs2];
        instr->args[0].ptr  = &proc->regs[instr->decoded.rs2];
        instr->args[0].flag = true;
    }
        
    else if((flags & ARG0_IS_PC) == ARG0_IS_PC) {
        instr->args[0].flag = true;
        instr->args[0].value = proc->pc;
        instr->args[0].ptr = &proc->pc;
    }

    // Set controls info for ARG1
    if((flags & ARG1_IS_RS1) == ARG1_IS_RS1) {
        instr->args[1].flag = true;
        instr->args[1].value = proc->regs[instr->decoded.rs1];
        instr->args[1].ptr = &proc->regs[instr->decoded.rs1];   
    }
    else if((flags & ARG1_IS_RS2) == ARG1_IS_RS2)
    {
        instr->args[1].value = proc->regs[instr->decoded.rs2];
        instr->args[1].ptr  = &proc->regs[instr->decoded.rs2];
        instr->args[1].flag = true;
    }
    else if((flags & ARG1_IS_PC) == ARG1_IS_PC)
    {
        instr->args[1].flag = true;
        instr->args[1].value = proc->pc;
        instr->args[1].ptr = &proc->pc;
    }
    // Set controls info for ARG2
    if((flags & ARG2_IS_RS1) == ARG2_IS_RS1)
    {
        instr->args[2].value = proc->regs[instr->decoded.rs1];
        instr->args[2].ptr = &proc->regs[instr->decoded.rs1];   
    }
    else if((flags & ARG2_IS_RS2) == ARG2_IS_RS2)
    {
        instr->args[2].ptr  = &proc->regs[instr->decoded.rs2];
        instr->args[2].flag = true;   
    }
    else if((flags & ARG2_IS_PC) == ARG2_IS_PC)
    {
        instr->args[2].flag = true;
        instr->args[2].ptr = &proc->pc;        
    }

    // Set controls info for ARG3
    if((flags & ARG3_IS_RS1) == ARG3_IS_RS1)
    {
        instr->args[3].flag = true;
        instr->args[3].ptr = &proc->regs[instr->decoded.rs1];           
    }
    else if((flags & ARG3_IS_RS2) == ARG3_IS_RS2)
    {
        instr->args[3].ptr  = &proc->regs[instr->decoded.rs2];
        instr->args[3].flag = true;  
    }
    else if((flags & ARG3_IS_PC) == ARG3_IS_PC)
    {
        instr->args[2].flag = true;
        instr->args[2].ptr = &proc->pc;  
    }

    // Set controls info for OUT0
    if((flags & OUT0_IS_RD) == OUT0_IS_RD) 
    {
        instr->out[0].flag = true;
        instr->out[0].ptr = &proc->regs[instr->decoded.rd];
    }
    else if((flags & OUT0_IS_PC) == OUT0_IS_PC) 
    {
        instr->out[0].flag = true;
        instr->out[0].ptr = &proc->pc;  
    }
    
    // Set controls info for OUT1
    if((flags & OUT1_IS_RD) == OUT1_IS_RD) 
    {
        instr->out[1].flag = true;
        instr->out[1].ptr = &proc->regs[instr->decoded.rd];
    }
    else if((flags & OUT1_IS_PC) == OUT1_IS_PC) 
    {
        instr->out[1].ptr = &proc->pc;
        instr->out[1].flag = true;
    } 
}

static inline void __fetch_failure(system_t* sys, riscv_processor_t* proc, void* addr) 
{

}

static inline void __store_failure(system_t* sys, riscv_processor_t* proc, void* addr) 
{

}

static inline void __read(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr)
{
    for(unsigned int i = 0; i < 4; i++) 
    {
        if(instr->args[i].ptr)
            instr->args[i].value = *instr->args[i].ptr;
    }
}

static inline void __write(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr)
{
    for(unsigned int i = 0; i < 2; i++) 
    {
        if(instr->out[i].ptr != NULL) *instr->out[i].ptr = instr->out[i].value;
    }
}

static inline void __execute(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr)
{
    octa out0, out1, a, b, c;
    int flags = instr->infos->flags;

    if((flags & ARG0_IS_IMMEDIATE) == ARG0_IS_IMMEDIATE)          instr->args[0].value = instr->decoded.imm;
    if((flags & ARG1_IS_IMMEDIATE) == ARG1_IS_IMMEDIATE)          instr->args[1].value = instr->decoded.imm;
    if((flags & ARG2_IS_IMMEDIATE) == ARG2_IS_IMMEDIATE)          instr->args[2].value = instr->decoded.imm;
    else if((flags & ARG2_COMPUTE_PLUS) == ARG2_COMPUTE_PLUS)     instr->args[2].value = instr->args[0].value + instr->args[1].value;
    if((flags & ARG3_IS_IMMEDIATE) == ARG3_IS_IMMEDIATE)          instr->args[3].value = instr->decoded.imm;

    a = instr->args[0].value;
    b = instr->args[1].value;
    c = instr->args[2].value;

    out0 = out1 = 0;

    switch(instr->op) 
    {
        case RISCV_LUI: out0 = a << 12; goto __store; // OK
        case RISCV_AUIPC: out0 = b + (a << 12); goto __store; // OK
        // jump
        case RISCV_JAL: out0 = b + 4, out1 = c + (a << 1); goto __store; // OK
        case RISCV_JALR: out0 = c, out1 = (a + b) & (~1); goto __store; // OK
        // branch
        case RISCV_BEQ: out0 = a; if(b == c) goto __store; break; // OK
        case RISCV_BNE: out0 = a; if(b != c) goto __store; break; // OK
        case RISCV_BLTU: case RISCV_BLT: out0 = a; if(b < c) goto __store; break; // OK
        case RISCV_BGEU: case RISCV_BGE: out0 = a; if(b >= c) goto __store; break; // OK
        // add
        case RISCV_ADD:  case RISCV_ADDI: case RISCV_ADDW: case RISCV_ADDIW: out0 = a + b; goto __store; 
        // sub
        case RISCV_SUB: case RISCV_SUBW: out0 = a - b; goto __store;
        // compare lt
        case RISCV_SLTU: case RISCV_SLTIU: case RISCV_SLTI: out0 = a < b; goto __store;         
        // xor
        case RISCV_XOR: case RISCV_XORI: out0 =  a ^ b; goto __store;
        // or
        case RISCV_OR: case RISCV_ORI: out0 = a | b; goto __store;
        // and
        case RISCV_AND: case RISCV_ANDI: out0 = a & b; goto __store;
        // shift left
        case RISCV_SLL: case RISCV_SLLI: case RISCV_SLLW: case RISCV_SLLIW: out0 = a << b; goto __store;
        // shift right
        case RISCV_SRL: case RISCV_SRLI: case RISCV_SRA: case RISCV_SRAI: case RISCV_SRLIW: case RISCV_SRAW: case RISCV_SRAIW: out0 = a >> b; goto __store;
        default: break;
        __store:
            instr->out[0].value = out0;
            instr->out[1].value = out1;
    }
}

static inline void __memory(system_t* sys, riscv_processor_t* proc, riscv_control_t* instr)
{
    octa a; void* c;
    a = instr->args[3].value;
    c = (void*)(instr->args[2].value);
    
    switch(instr->op) 
    {
        // load
        case RISCV_LBU: case RISCV_LB: if(sys_load_byte(sys, c, (byte*) &instr->out[0].value)) return __fetch_failure(sys, proc, c); break;
        case RISCV_LHU: case RISCV_LH: if(sys_load_word(sys, c, (word*) &instr->out[0].value)) return __fetch_failure(sys, proc, c); break;
        case RISCV_LW: case RISCV_LWU: if(sys_load_tetra(sys, c, (tetra*) &instr->out[0].value)) return __fetch_failure(sys, proc, c); break;
        case RISCV_LD: if(sys_load_octa(sys, c, &instr->out[0].value)) return __fetch_failure(sys, proc, c); break;
        // store
        case RISCV_SB: if(!sys_store_byte(sys, c, a))  return __store_failure(sys, proc, c); break;
        case RISCV_SH: if(!sys_store_word(sys, c, a))  return __store_failure(sys, proc, c); break;
        case RISCV_SW: if(!sys_store_tetra(sys, c, a)) return __store_failure(sys, proc, c); break;
        case RISCV_SD: if(!sys_store_octa(sys, c, a))  return __store_failure(sys, proc, c); break;
        default: break;
    }
}

void riscv_alloc_sim_time(system_t* sys, unsigned int ms) {
    riscv_processor_t* proc = __get_riscv_proc(sys);

    unsigned int s = ms * 1000;
    int remaining_cycles = s * proc->frequency;
    proc->remaining_cycles = remaining_cycles;
}

/**
 * Simple cycling; no pipeline
 * 
 * fetch
 * decode
 * read
 * execute
 * memory
 * write
 */
void riscv_step(system_t* sys)
{   
    riscv_processor_t* proc = __get_riscv_proc(sys);

    if(proc->remaining_cycles == 0) return sys_halt(sys);
    
    if(proc->current_control.stage == RISCV_FETCH && proc->pc >= proc->regs[2])   
        return sys_stop(sys);
    
    switch(proc->current_control.stage) 
    {
        case RISCV_FETCH:
            if(!__fetch(sys, proc, &proc->current_control))
                return sys_panic(sys);
            
            break;
        case RISCV_DECODE:
            __decode(sys, proc , &proc->current_control);
            break;
        case RISCV_READ:
            __read(sys, proc, &proc->current_control);
            break;
        case RISCV_EXECUTE:
            __execute(sys, proc, &proc->current_control);
            break;
        case RISCV_MEMORY:
            __memory(sys, proc, &proc->current_control);
            break;
        case RISCV_WRITE:
            __write(sys, proc, &proc->current_control); 
            break;
    }

    proc->current_control.stage = (proc->current_control.stage + 1) % 6;

    if(proc->remaining_cycles > 0) proc->remaining_cycles--;
}


#endif