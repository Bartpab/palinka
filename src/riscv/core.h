#ifndef __RISCV_CORE_H__
#define __RISCV_CORE_H__

#include "../../lib/common/include/allocator.h"
#include "../../sys.h"

#include "./processor.h"

typedef enum {
    UNKNOWN,
    // RV32I Instruction Set
    LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU, LB, LH, LW, LBU, LHU, SB, SH, SW,
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND, FENCE, ECALL, EBREAK,
    // RV64I Instruction Set
    LWU, LD, SD, SLLI, SRLI, SRAI, ADDIW, SLLIW, SRLIW, SRAIW, ADDW, SUBW, SLLW, SRLW, SRAW,
    // RV32/RV64 Zifencei Standard Extension
    FENCE_I,
    // RV32/RV64 Zicsr Standard Extension
    CSRRW, CSRRS, CSRRC, CSRRWI, CSRRSI, CSRRCI,
    // RV32M Standard Extension
    MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU,
    // RV64M Standard Extension
    MULW, DIVW, DIVUW, REMW, REMUW 
} riscv_opcode_t

#define A_IS_IMMEDIATE 0b001
#define A_IS_RS1 0b010 
#define A_IS_RS2 0b100 

#define B_IS_IMMEDIATE 0b001 << 3
#define B_IS_RS1 0b100 << 3
#define B_IS_RS2 0b010 << 3

#define RD_IS_TETRA 0b001 << 6

typedef struct {
    const char* name;
    int flags;
} riscv_instr_info_t;

riscv_instr_info_t infos[] {
    {"UNKNOWN", 0},
    {"LUI", A_IS_IMMEDIATE}, {"AUIPC", A_IS_IMMEDIATE}, 
    {"JAL", A_IS_IMMEDIATE}, {"JALR", A_IS_IMMEDIATE},
    {"BEQ", A_IS_IMMEDIATE}, {"BNE", A_IS_IMMEDIATE}, {"BLT", A_IS_IMMEDIATE}, {"BGE", A_IS_IMMEDIATE}, {"BLTU", A_IS_IMMEDIATE}, {"BGEU", A_IS_IMMEDIATE},
    {"LB", A_IS_IMMEDIATE}, {"LH", A_IS_IMMEDIATE}, {"LW", A_IS_IMMEDIATE}, {"LBU", A_IS_IMMEDIATE}, {"LHU", A_IS_IMMEDIATE},
    {"SB", A_IS_IMMEDIATE}, {"SH", A_IS_IMMEDIATE}, {"SW", A_IS_IMMEDIATE},
    {"ADDI", A_IS_RS1 | B_IS_IMMEDIATE}, {"SLTI", A_IS_RS1 | B_IS_IMMEDIATE}, {"SLTIU", A_IS_RS1 | B_IS_IMMEDIATE}, {"XORI", A_IS_RS1 | B_IS_IMMEDIATE},
    {"ORI", A_IS_RS1 | B_IS_IMMEDIATE}, {"ANDI", A_IS_RS1 | B_IS_IMMEDIATE}, {"SLLI", A_IS_RS1 | B_IS_IMMEDIATE}, {"SRLI", A_IS_RS1 | B_IS_IMMEDIATE},
    {"SRAI", A_IS_RS1 | B_IS_IMMEDIATE},
    {"ADD", A_IS_RS1 | B_IS_RS2}, {"SUB", A_IS_RS1 | B_IS_RS2}, {"SLL", A_IS_RS1 | B_IS_RS2}, {"SLT", A_IS_RS1 | B_IS_RS2}, 
    {"SLTU", A_IS_RS1 | B_IS_RS2}, {"XOR", A_IS_RS1 | B_IS_RS2}, {"SRL", A_IS_RS1 | B_IS_RS2}, {"SRA", A_IS_RS1 | B_IS_RS2}, 
    {"OR", A_IS_RS1 | B_IS_RS2}, {"AND", A_IS_RS1 | B_IS_RS2}, {"FENCE", 0}, {"ECALL", 0}, {"EBREAK", 0},
}


typedef enum {
    // Machine info registers
    MVENDORID = 0xF11, // Vendor ID
    MARCHID = 0xF12, // Architecture ID
    MIMPID = 0xF13, // Implementation ID
    MHARTID = 0xF14, // Hardware thread ID
    // Machine trap setup
    MSTATUS = 0x300, // Machine status register
    MISA = 0x301, // ISA and extensions
    MEDELEG = 0x302, // Machine exception delegation register
    MIDELEG = 0x303, // Machine interrupt delegation register
    MIE = 0x304, // Machine interrupt-enable register
    MTVEC = 0x305, // Machine trap-handler base address
    MTCOUNTEREN = 0x306, // Machine counter enable
    MSTATUSH = 0x310, // Additional machine status register, RV32 only
    // Machine trap handling
    MSCRATCH = 0x340, // Machine status register
    MEPC = 0x341, // Machine exception program counter
    MCAUSE = 0x342, // Machine trap cause
    MTVAL = 0x343, // Machine bad address or instruction
    MIP = 0x344, // Machine interrupt pending
    MTINST = 0x34A, // Machine trap instruction (transformed)
    MTVAL2 = 0x34B, // Machine bad guest physical address
    // Machine memory protection
    PMPCFG0 = 0x3A0, // Physical memory protection config.
    PMPCFG1 = 0x3A1, // Physical memory protection config, RV32 only
    PMPCFG2 = 0x3A2, // Physical memory protection config.    
    PMPCFG3 = 0x3A3, // Physical memory protection config, RV32 only
    PMPCFG4 = 0x3A4, // Physical memory protection config.   
    PMPCFG5 = 0x3A5, // Physical memory protection config, RV32 only
    PMPCFG6 = 0x3A6, // Physical memory protection config.
    PMPCFG7 = 0x3A7, // Physical memory protection config, RV32 only
    PMPCFG8 = 0x3A8, // Physical memory protection config.    
    PMPCFG9 = 0x3A9, // Physical memory protection config, RV32 only
    PMPCFG10 = 0x3AA, // Physical memory protection config.   
    PMPCFG11 = 0x3AB, // Physical memory protection config, RV32 only
    PMPCFG12 = 0x3AC, // Physical memory protection config.
    PMPCFG13 = 0x3AD, // Physical memory protection config, RV32 only
    PMPCFG14 = 0x3AE, // Physical memory protection config.    
    PMPCFG15 = 0x3AF, // Physical memory protection config, RV32 only
    PMPADDR0 = 0x3B0, // Physical memory protection address register
    PMPADDR1 = 0x3B1, // Physical memory protection address register
    PMPADDR2 = 0x3B2, // Physical memory protection address register
    PMPADDR3 = 0x3B3, // Physical memory protection address register
    PMPADDR4 = 0x3B4, // Physical memory protection address register
    PMPADDR5 = 0x3B5, // Physical memory protection address register
    PMPADDR6 = 0x3B6, // Physical memory protection address register
    PMPADDR7 = 0x3B7, // Physical memory protection address register
    PMPADDR8 = 0x3B8, // Physical memory protection address register
    PMPADDR9 = 0x3B9, // Physical memory protection address register
    PMPADDR10 = 0x3BA, // Physical memory protection address register
    PMPADDR11 = 0x3BB, // Physical memory protection address register
    PMPADDR12 = 0x3BC, // Physical memory protection address register
    PMPADDR13 = 0x3BD, // Physical memory protection address register
    PMPADDR14 = 0x3BE, // Physical memory protection address register
    PMPADDR15 = 0x3BF, // Physical memory protection address register
    PMPADDR16 = 0x3C0, // Physical memory protection address register
    PMPADDR17 = 0x3C1, // Physical memory protection address register
    PMPADDR18 = 0x3C2, // Physical memory protection address register
    PMPADDR19 = 0x3C3, // Physical memory protection address register
    PMPADDR20 = 0x3C4, // Physical memory protection address register
    PMPADDR21 = 0x3C5, // Physical memory protection address register
    PMPADDR22 = 0x3C6, // Physical memory protection address register
    PMPADDR23 = 0x3C7, // Physical memory protection address register
    PMPADDR24 = 0x3C8, // Physical memory protection address register
    PMPADDR25 = 0x3C9, // Physical memory protection address register
    PMPADDR26 = 0x3CA, // Physical memory protection address register
    PMPADDR27 = 0x3CB, // Physical memory protection address register
    PMPADDR28 = 0x3CC, // Physical memory protection address register
    PMPADDR29 = 0x3CD, // Physical memory protection address register
    PMPADDR30 = 0x3CE, // Physical memory protection address register
    PMPADDR31 = 0x3CF, // Physical memory protection address register
    PMPADDR32 = 0x3D0, // Physical memory protection address register
    PMPADDR33 = 0x3D1, // Physical memory protection address register
    PMPADDR34 = 0x3D2, // Physical memory protection address register
    PMPADDR35 = 0x3D3, // Physical memory protection address register
    PMPADDR36 = 0x3D4, // Physical memory protection address register
    PMPADDR37 = 0x3D5, // Physical memory protection address register
    PMPADDR38 = 0x3D6, // Physical memory protection address register
    PMPADDR39 = 0x3D7, // Physical memory protection address register
    PMPADDR40 = 0x3D8, // Physical memory protection address register
    PMPADDR41 = 0x3D9, // Physical memory protection address register
    PMPADDR42 = 0x3DA, // Physical memory protection address register
    PMPADDR43 = 0x3DB, // Physical memory protection address register
    PMPADDR44 = 0x3DC, // Physical memory protection address register
    PMPADDR45 = 0x3DE, // Physical memory protection address register
    PMPADDR46 = 0x3DF, // Physical memory protection address register
    PMPADDR47 = 0x3E0, // Physical memory protection address register
    PMPADDR48 = 0x3E1, // Physical memory protection address register
    PMPADDR49 = 0x3E2, // Physical memory protection address register
    PMPADDR50 = 0x3E3, // Physical memory protection address register
    PMPADDR51 = 0x3E4, // Physical memory protection address register
    PMPADDR52 = 0x3E5, // Physical memory protection address register
    PMPADDR53 = 0x3E6, // Physical memory protection address register
    PMPADDR54 = 0x3E7, // Physical memory protection address register
    PMPADDR55 = 0x3E8, // Physical memory protection address register
    PMPADDR56 = 0x3E9, // Physical memory protection address register
    PMPADDR57 = 0x3EA, // Physical memory protection address register
    PMPADDR58 = 0x3EB, // Physical memory protection address register
    PMPADDR59 = 0x3EC, // Physical memory protection address register
    PMPADDR60 = 0x3ED, // Physical memory protection address register
    PMPADDR61 = 0x3EF, // Physical memory protection address register
    PMPADDR62 = 0x3F0, // Physical memory protection address register
    PMPADDR63 = 0x3F1 // Physical memory protection address register
    // Machine counter and timers
    MCYCLE = 0xB00, // Machine cycle counter
    MINSTRET = 0xB02, // Machine instructions-retired counter
    MHPMCOUNTER3 = 0xB03, // Machine performance monitoring counter
    MHPMCOUNTER4 = 0xB04, 
    MHPMCOUNTER5 = 0xB05,
    MHPMCOUNTER6 = 0xB06,
    MHPMCOUNTER7 = 0xB07,
    MHPMCOUNTER8 = 0xB08,
    MHPMCOUNTER9 = 0xB09,
    MHPMCOUNTER10 = 0xB0A, 
    MHPMCOUNTER11 = 0xB0B, 
    MHPMCOUNTER12 = 0xB0C,
    MHPMCOUNTER13 = 0xB0D,
    MHPMCOUNTER14 = 0xB0E,
    MHPMCOUNTER15 = 0xB0F,
    MHPMCOUNTER16 = 0xB10,
    MHPMCOUNTER17 = 0xB11, // Machine performance monitoring counter
    MHPMCOUNTER18 = 0xB12, 
    MHPMCOUNTER19 = 0xB13,
    MHPMCOUNTER20 = 0xB14,
    MHPMCOUNTER21 = 0xB15,
    MHPMCOUNTER22 = 0xB16,
    MHPMCOUNTER23 = 0xB17,
    MHPMCOUNTER24 = 0xB18, 
    MHPMCOUNTER25 = 0xB19, 
    MHPMCOUNTER26 = 0xB1A,
    MHPMCOUNTER27 = 0xB1B,
    MHPMCOUNTER28 = 0xB1C,
    MHPMCOUNTER29 = 0xB1D,
    MHPMCOUNTER30 = 0xB1E,
    MHPMCOUNTER31 = 0xB1F,
    MCYCLEH = 0xB80, // Upper 32 bits of mycle, RV32 only
    MINSTRETH = 0xB82, // Upper 32 bits of minstret, RV32 only
    MHPMCOUNTER3H = 0xB83, // Machine performance monitoring counter
    MHPMCOUNTER4H = 0xB84, 
    MHPMCOUNTER5H = 0xB85,
    MHPMCOUNTER6H = 0xB86,
    MHPMCOUNTER7H = 0xB87,
    MHPMCOUNTER8H = 0xB88,
    MHPMCOUNTER9H = 0xB89,
    MHPMCOUNTER10H = 0xB8A, 
    MHPMCOUNTER11H = 0xB8B, 
    MHPMCOUNTER12H = 0xB8C,
    MHPMCOUNTER13H = 0xB8D,
    MHPMCOUNTER14H = 0xB8E,
    MHPMCOUNTER15H = 0xB8F,
    MHPMCOUNTER16H = 0xB90,
    MHPMCOUNTER17H = 0xB91, // Machine performance monitoring counter
    MHPMCOUNTER18H = 0xB92, 
    MHPMCOUNTER19H = 0xB93,
    MHPMCOUNTER20H = 0xB94,
    MHPMCOUNTER21H = 0xB95,
    MHPMCOUNTER22H = 0xB96,
    MHPMCOUNTER23H = 0xB97,
    MHPMCOUNTER24H = 0xB98, 
    MHPMCOUNTER25H = 0xB99, 
    MHPMCOUNTER26H = 0xB9A,
    MHPMCOUNTER27H = 0xB9B,
    MHPMCOUNTER28H = 0xB9C,
    MHPMCOUNTER29H = 0xB9D,
    MHPMCOUNTER30H = 0xB9E,
    MHPMCOUNTER31H = 0xB9F,
    // Machine counter setup
    MCOUNTINHIBIT = 0x320,
    MHPMEVENT3 = 0x323, // Machine performance monitoring event selector
    MHPMEVENT4 = 0x324, 
    MHPMEVENT5 = 0x325,
    MHPMEVENT6 = 0x326,
    MHPMEVENT7 = 0x327,
    MHPMEVENT8 = 0x328,
    MHPMEVENT9 = 0x329,
    MHPMEVENT10 = 0x32A, 
    MHPMEVENT11 = 0x32B, 
    MHPMEVENT12 = 0x32C,
    MHPMEVENT13 = 0x32D,
    MHPMEVENT14 = 0x32E,
    MHPMEVENT15 = 0x32F,
    MHPMEVENT16 = 0x330,
    MHPMEVENT17 = 0x331, // Machine performance monitoring counter
    MHPMEVENT18 = 0x332, 
    MHPMEVENT19 = 0x333,
    MHPMEVENT20 = 0x334,
    MHPMEVENT21 = 0x335,
    MHPMEVENT22 = 0x336,
    MHPMEVENT23 = 0x337,
    MHPMEVENT24 = 0x338, 
    MHPMEVENT25 = 0x339, 
    MHPMEVENT26 = 0x33A,
    MHPMEVENT27 = 0x33B,
    MHPMEVENT28 = 0x33C,
    MHPMEVENT29 = 0x33D,
    MHPMEVENT30 = 0x33E,
    MHPMEVENT31 = 0x34F,
    // Debug/Trace registers
    TSELECT = 0X7A0,
    TDATA1 = 0x7A1,
    TDATA2 = 0x7A2,
    TDATA3 = 0x7A3,
    // Debug mode registers
    DCSR = 0x7B0,
    DPC = 0x7B1,
    DSCRATCH0 = 0x7B2,
    DSCRATCH1 = 0x7B3,
    // Supervisor trap setup
    SSTATUS = 0x100, // Supervisor status register
    SEDELEG = 0x102, // Supervisor exception delegation register
    SIDELEG = 0x103, // Supervisor interrupt delegation register
    SIE = 0x104, // Supervisor interrupt-enable register
    STVEC = 0x105, // Supervisor trap handler base address
    SCOUNTEREN = 0x106, // Supervisor counter enable
    // Supervisor trap handling
    SSCRATCH = 0x140, // Scratch register for supervisor trap handlers
    SEPC = 0x141, // Supervisor exception program counter
    SCAUSE = 0x142, // Supervisor trap cause,
    STVAL = 0x143, // Supervisor bad address or instruction
    SIP = 0x144, // Supervisor interrupt pending
    // Supervisor protection and translation
    SATP = 0x180 // Supervisor protection and translation
} riscv_csrs_t;

typedef struct {
    tetra raw;
    byte opcode;
    byte rd;
    byte funct3;
    byte funct7;
    byte rs1;
    byte rs2;
    octa shamt;
    octa imm;
    unsigned int op;
} riscv_instr_t;

system_t* riscv_new(allocator_t* allocator);
void riscv_alloc_sim_time(system_t* sys, unsigned int ms);
void riscv_step(system_t* sys);

static void __riscv_init(system_t* sys);

static bool __fetch(system_t* sys, riscv_processor_t* proc, tetra* instr);
static void __decode(system_t* sys, riscv_processor_t* proc, tetra* raw, riscv_instr_t* instr);
static void __execute(system_t* sys, riscv_processor_t* proc, riscv_instr_t* instr);

static void __load_csr(system_t* sys, riscv_processor_t* proc, unsigned int addr, octa* out)
{
    if(addr == SIE) {
        *out = proc->csrs[MIE] & proc->csrs[MIDELEG];
    } else {
        *out = proc->csrs[addr % 4096];
    }
}

static void __store_csr(system_t* sys, riscv_processor_t* proc, unsigned int, const octa value)
{
    if(addr == SIE) {
        proc->csrs[MIE] = (proc->csrs[MIE] & ~proc->csrs[MIDELEG]) | (value & proc->csrs[MIDELEG]);
    } else {
        proc->csrs[addr % 4096] = value;
    }
}

system_t* riscv_new(allocator_t* allocator)
{
  system_t* sys = (system_t*) pmalloc(
    allocator, 
    sizeof(system_t) 
      + sizeof(riscv_processor_t) 
  );
  
  if(!sys)
    return NULL;

  __sys_init(sys, allocator);
  __riscv_init(sys);
 
  return sys;
}

static void __riscv_init(system_t* sys) 
{
    sys->step = riscv_step;
    sys->alloc_sim_time = riscv_alloc_sim_time; 

    riscv_processor_t* proc = __get_riscv_proc(sys);

    // Set the pc
    proc->pc = 0x80000000;

    // Set memory size to x2 register.
    proc->regs[2] = octa_uint_max;

    //
    proc->regs[0] = octa_zero;
}

static bool __fetch(system_t* sys, riscv_processor_t* proc, tetra* instr)
{
    void* vaddr = (void*) proc->pc;
    return sys_load_tetra(sys, vaddr, &instr);
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
static void __decode(system_t* sys, riscv_processor_t* proc, tetra* raw, riscv_instr_t* instr)
{
    byte opcode = raw & 0x7F;
    byte rd = (raw >> 7) & 0x1F;
    byte funct3 = (raw >> 12) & 0x7;
    byte rs1 = (raw >> 15) & 0x1F;
    byte rs2 = (raw >> 20) & 0x1F;
    byte funct7 = (raw >> 25) & 0x7F;
    word imm12 = (raw >> 20) & 0xFFF;
    byte shamt12 = imm12 & 0x3f;
    byte imm5 = (raw >> 7) & 0x1F;
    byte imm7 = (raw >> 25) & 0x7F;
    tetra imm20 = (raw >> 12) & 0xFFFFF;
    byte shamt20 = imm20 & 0x1f;

    instr->raw = *raw;
    instr->opcode = opcode;
    instr->rd = rd;
    instr->funct3 = funct3;
    instr->funct7 = funct7;

    instr->rs1 = rs1;
    instr->rs2 = rs2;
    instr->shamt = 0;
    
    instr->op = 0;

    switch(instr->opcode) {
        case 0b0110111: instr->op = LUI; goto u_type;
        case 0b0010111: instr->op = AUIPC; goto u_type;
        case 0b1101111: 
            instr->op = JAL,instr->immediate = true; goto j_type;
        case 0b1100111: instr->op = JALR; goto b_type;
        case 0b1100011:
            switch(instr->funct3) {
                case 0b000: instr->op = BEQ; 
                case 0b001: instr->op = BNE;
                case 0b100: instr->op = BLT;
                case 0b101: instr->op = BGE;
                case 0b110: instr->op = BLTU;
                case 0b111: instr->op = BGEU;
                default: goto b_type;
            }
            break;
        case 0b0000011:
            switch(instr->funct3) {
                case 0b000: instr->op = LB;
                case 0b001: instr->op = LH; 
                case 0b010: instr->op = LW; 
                case 0b100: instr->op = LBU;
                case 0b101: instr->op = LHU;
                default: goto i_type;
            }
        case 0b0100011:
            switch(instr->funct3) {
                case 0b000: instr->op = SB;
                case 0b001: instr->op = SH;
                case 0b010: instr->op = SW;
                default: goto s_type;
            } 
            break;
        case 0b0010011:
            switch(instr->funct3) {
                case 0b000: instr->op = ADDI; goto i_type;
                case 0b010: instr->op = SLTI; goto i_type;
                case 0b011: instr->op = SLTIU; goto i_type;
                case 0b100: instr->op = XORI; goto i_type;
                case 0b110: instr->op = ORI; goto i_type;
                case 0b111: instr->op = ANDI; goto i_type;
                case 0b001:
                    switch(instr->funct7) {
                        case 0b0000000: instr->op = SLLI; 
                        default: instr->shamt = instr->rs2; goto __end;
                    }
                case 0b101:
                    switch(instr->funct7) {
                        case 0b0000000: instr->op = SRLI; 
                        case 0b0100000: instr->op = SRAI;
                        default: instr->shamt = instr->rs2; goto __end;
                    }
                default: goto __end;
            }
        case 0b0110011:
            switch(instr->funct3) {
                case 0b000:
                    switch(instr->funct7) {
                        case 0b0000000: instr->op = ADD;
                        case 0b0100000: instr->op = SUB; 
                        default: goto __end;
                    }
                case 0b001:
                    switch(instr->funct7) {
                        case 0b0000000: instr->op = SLL;
                        default: goto __end;
                    }
                case 0b010:
                    switch(instr->funct7) {
                        case 0b0000000: instr->op = SLT;
                        default: goto __end;
                    }
                case 0b011:
                    switch(instr->funct7) {
                        case 0b0000000: instr->op = SLTU;
                        default: goto __end;
                    }
                case 0b100:
                    switch(instr->funct7) {
                        case 0b0000000: instr->op = XOR;
                        default: goto __end;
                    }
                case 0b101:
                    switch(instr->funct7) {
                        case 0b0000000: instr->op = SRL; 
                        case 0b0100000: instr->op = SRA;
                        default: goto __end;
                    }
                case 0b110:
                    switch(instr->funct7) {
                        case 0b0000000: instr->op = OR;
                        default: goto __end;
                    }
                case 0b111:
                    switch(instr->funct7) {
                        case 0b0000000: instr->op = AND;
                        default: goto __end;
                    } 
                default: goto __end;
            }
            break;
        case 0b0001111: instr->op = FENCE; goto __end;
        case 0b1110011:
            if(instr->rd == 0 && instr->funct3 == 0 && instr->rs1 == 0) {
                switch(instr->imm12) {
                    case 0x0: instr->op = ECALL; goto __end;
                    case 0x1: instr->op = EBREAK; goto __end;
                    default: goto __end;
                }
            }
            goto __end;
        case 0b0011011:
            switch(instr->funct3) {
                case 0b000: instr->op = ADDIW; 
                case 0b001: instr->op = SLLIW;
                case 0b101: 
                    switch(instr->funct7) {
                        case 0x00: instr->op = SRLIW;
                        case 0x20: instr->op = SRAIW;
                        default: goto __end;
                    }
                default: goto __end;
            }
            break;

        i_type: 
            instr->imm = (instr->raw >> 20) & 1;
            instr->imm |= ((instr->raw >> 21) & ‭0x7FF‬) << 1;
            break;
        s_type:
            instr->imm = (instr->raw >> 7) & 1;
            instr->imm |= ((instr->raw >> 8) & 0xF) << 1;
            instr->imm |= ((instr->raw >> 25) & 0x40) << 5;
            break;
        b_type:
            instr->imm = ((instr->raw >> 8) & 0xF) << 1;
            instr->imm |= ((instr->raw >> 25) & 0x3F) << 5;
            instr->imm |= ((instr->raw >> 7) & 1) << 11;
            instr->imm |= ((instr->raw >> 31) & 1) << 12;
            break;
        u_type:
            instr->imm = ((instr->raw >> 12) & 0xFF) << 12;
            instr->imm |= ((instr->raw >> 20) & 0x7FF) << 20;
            instr->imm |= ((instr->raw >> 31) & 1) << 31;
            break;
        j_type:
            instr->imm = ((instr->raw >> 21) & 0x3FF) << 1;
            instr->imm |= ((instr->raw >> 20) & 1) << 11;
            instr->imm |= ((instr->raw >> 12) & 0x7F) << 12;
            instr->imm |= ((instr->raw >> 32) & 1) << 31;
            break;
        __end:
            break;
    }
}

static void __fetch_failure(system_t* sys, riscv_processor_t* proc, void* addr) 
{

}

static void __store_failure(system_t* sys, riscv_processor_t* proc, void* addr) 
{

}

static inline void __execute(system_t* sys, riscv_processor_t* proc, riscv_instr_t* instr)
{
    octa rd, a, b, pc, rs1, rs2;
    void* c;
    rd = pc = a = b = 0;

    pc = proc->pc;
    rs1 = proc->regs[instr->rs1];
    rs2 = proc->regs[instr->rs2];



    switch(instr->op) 
    {
        case LUI: rd = instr->imm << 12; goto __store_rd; // OK
        case AUIPC: rd = pc + (instr->imm << 12); goto __store_rd; // OK
        // jump
        case JAL: rd = pc + 4, pc += (instr->imm << 1); goto __store_rd_pc; // OK
        case JALR: rd = pc, pc = (rs1 + instr->imm) & (~1); goto __store_rd_pc; // OK
        // branch
        case BEQ: case BNE: case BLT: case BGE: case BLTU: case BGEU: pc = instr->imm;
        case BEQ: if(rs1 == rs2) goto __store_pc; break; // OK
        case BNE: if(rs1 != rs2) goto __store_pc; break; // OK
        case BLTU: case BLT: if(rs1 < rs2) goto __store_pc; break; // OK
        case BGEU: case BGE: if(rs1 >= rs2) goto __store_pc; break; // OK
        // For LOAD: dest = base(rs1) + offset (imm)
        case LBU: case LB: case LHU: case LW: case LWU: case LD: c = rs1 + instr->imm;
        // load
        case LBU: case LB: if(sys_load_byte(sys, c, (byte*) &rd)) return __fetch_failure(sys, proc, c); goto __store_rd;
        case LHU: case LH: if(sys_load_word(sys, c, (word*) &rd)) return __fetch_failure(sys, proc, c); goto __store_rd;
        case LW: case LWU: if(sys_load_tetra(sys, c, (tetra*) &rd)) return __fetch_failure(sys, proc, c); goto __store_rd;
        case LD: if(sys_load_octa(sys, c, &rd)) return __fetch_failure(sys, proc, c); goto __store_rd;
        // For STORE: dest = base(rs1) + offset (imm)
        case SB: case SH: case SW: case SD: c = rs1 + instr->imm;
        // store
        case SB: if(!sys_store_byte(sys, c, rs2)) return __store_failure(sys, proc, c); break;
        case SH: if(!sys_store_word(sys, c, rs2)) return __store_failure(sys, proc, c); break;
        case SW: if(!sys_store_tetra(sys, c, rs2)) return __store_failure(sys, proc, c); break;
        case SD: if(!sys_store_octa(sys, c, rs2)) return __store_failure(sys, proc, c); break;
        // add
        case ADD:
            a = rs2;
        case ADDI:
            a = instr->imm;
        case ADD:  case ADDI: case ADDIW: rd = rs1 + a; goto __store_rd; 
        case ADDW: rd = (rs1 + a) & 0xFFFFFFFF; goto __store_rd;
        // sub
        case SUB: rd = rs1 - a; goto __store_rd;
        case SUBW: rd = (rs1 - a) & 0xFFFFFFFF; goto __store_rd;
        // compare lt
        case SLTU: case SLTIU: case SLTI: rd = rs1 < a; goto __store_rd;         
        // xor
        case XOR: case XORI: rd =  rs1 ^ a; goto __store_rd;
        // or
        case OR: case ORI: rd = rs1 | a; goto __store_rd;
        // and
        case AND: case ANDI: rd = rs1 & a; goto __store_rd;
        // shift left
        case SLL: case SLLI: case SLLIW: rd = rs1 << b; goto __store_rd;
        case SLLW: rd = (rs1 << b) & 0xFFFFFFFF; goto __store_rd;
        // shift right
        case SRL: case SRA: case SRLI: case SRAI: case SRLIW: case SRAIW: rd = rs1 >> b; goto __store_rd;
        case SRLW: rd = (rs1 >> b) & 0xFFFFFFFF; goto __store_rd;
        case SRLIW: rd = ((rs1 & 0xFFFFFFFF) >> b); goto __store_rd;
        case SRAW: rd = ((rs1 & 0xFFFFFFFF) >> (shamt & 0xFFFFFFFF)); goto __store_rd;

        __store_rd_pc: proc->pc = pc; proc->regs[instr->rd] = rd; break;
        __store_pc: proc->pc = pc; break;
        __store_rd: proc->regs[instr->rd] = rd; break;
    }
}

void riscv_step(system_t* sys)
{   
    riscv_processor_t* proc = __get_riscv_proc(sys);
    tetra raw_instr;
    riscv_instr_t instr;

    if(proc->pc >= proc->regs[2]) 
        return sys_panic(sys);

    if(!__fetch(sys, proc, &raw_instr)) 
        return __fetch_failure(sys, proc, (void*)(proc->pc));

    proc->pc += 4;

    __decode(sys, proc, &raw_instr, &instr);
    __execute(sys, proc, &instr);
}


#endif