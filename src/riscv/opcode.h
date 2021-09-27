#ifndef __RISCV_OPCODE_H__
#define __RISCV_OPCODE_H__

typedef enum {
    UNKNOWN,
    // RV32I Instruction Set
    RISCV_LUI, RISCV_AUIPC, RISCV_JAL, RISCV_JALR, RISCV_BEQ, RISCV_BNE, RISCV_BLT, RISCV_BGE, RISCV_BLTU, RISCV_BGEU, RISCV_LB, RISCV_LH, 
    RISCV_LW, RISCV_LBU, RISCV_LHU, RISCV_SB, RISCV_SH, RISCV_SW,
    RISCV_ADDI, RISCV_SLTI, RISCV_SLTIU, RISCV_XORI, RISCV_ORI, RISCV_ANDI, RISCV_SLLI, RISCV_SRLI, RISCV_SRAI,
    RISCV_ADD, RISCV_SUB, RISCV_SLL, RISCV_SLT, RISCV_SLTU, RISCV_XOR, RISCV_SRL, 
    RISCV_SRA, RISCV_OR, RISCV_AND, RISCV_FENCE, RISCV_ECALL, RISCV_EBREAK,
    // RV64I Instruction Set
    RISCV_LWU, RISCV_LD, RISCV_SD,  
    RISCV_ADDIW, RISCV_SLLIW, RISCV_SRLIW, RISCV_SRAIW, 
    RISCV_ADDW, RISCV_SUBW, RISCV_SLLW, RISCV_SRLW, RISCV_SRAW,
    // RV32/RV64 Zifencei Standard Extension
    RISCV_FENCE_I,
    // RV32/RV64 Zicsr Standard Extension
    RISCV_CSRRW, RISCV_CSRRS, RISCV_CSRRC, RISCV_CSRRWI, RISCV_CSRRSI, RISCV_CSRRCI,
    // RV32M Standard Extension
    RISCV_MUL, RISCV_MULH, RISCV_MULHSU, RISCV_MULHU, RISCV_DIV, RISCV_DIVU, RISCV_REM, RISCV_REMU,
    // RV64M Standard Extension
    RISCV_MULW, RISCV_DIVW, RISCV_DIVUW, RISCV_REMW, RISCV_REMUW 
} riscv_opcode_t;

#define ARG0_IS_IMMEDIATE   0b001
#define ARG0_IS_RS1         0b010 
#define ARG0_IS_RS2         0b100 
#define ARG0_IS_PC          0b110 

#define ARG1_IS_IMMEDIATE   0b001 << 3
#define ARG1_IS_RS1         0b100 << 3
#define ARG1_IS_RS2         0b010 << 3
#define ARG1_IS_PC          0b110 << 3

#define ARG2_IS_IMMEDIATE   0b001 << 6
#define ARG2_IS_RS1         0b100 << 6
#define ARG2_IS_RS2         0b010 << 6
#define ARG2_IS_PC          0b110 << 6
#define ARG2_COMPUTE_PLUS   0b111 << 6

#define ARG3_IS_IMMEDIATE   0b001 << 9
#define ARG3_IS_RS1         0b100 << 9
#define ARG3_IS_RS2         0b010 << 9
#define ARG3_IS_PC          0b110 << 9
      
#define OUT0                0b111 << 12
#define OUT0_IS_TETRA       0b001 << 12
#define WRITE_REG          0b010 << 12
#define OUT0_IS_PC          0b010 << 12

#define OUT1                0b111 << 15
#define OUT1_IS_RD          0b010 << 15
#define OUT1_IS_PC          0b100 << 15

#define WRITE_PC            0b1 << 16

typedef struct riscv_instr_info_t {
    const char* name;
    int flags;
} riscv_instr_info_t;

struct riscv_instr_info_t riscv_instr_infos[] = {
    {"NOP",     0},
    {"LUI",     ARG1_IS_IMMEDIATE | WRITE_REG | WRITE_PC}, 
    {"AUIPC",   ARG1_IS_IMMEDIATE | WRITE_REG | WRITE_PC}, 
    {"JAL",     ARG1_IS_IMMEDIATE | WRITE_REG | WRITE_PC}, 
    {"JALR",    ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG | WRITE_PC},
    {"BEQ",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_PC}, 
    {"BNE",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_PC}, 
    {"BLT",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_PC}, 
    {"BGE",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_PC}, 
    {"BLTU",    ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_PC}, 
    {"BGEU",    ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_PC},
    {"LB",      ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG}, 
    {"LH",      ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG}, 
    {"LW",      ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG}, 
    {"LBU",     ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG}, 
    {"LHU",     ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG},
    {"SB",      ARG0_IS_RS1 | ARG1_IS_RS2}, 
    {"SH",      ARG0_IS_RS1 | ARG1_IS_RS2}, 
    {"SW",      ARG0_IS_RS1 | ARG1_IS_RS2},
    {"ADDI",    ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG}, 
    {"SLTI",    ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG}, 
    {"SLTIU",   ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG}, 
    {"XORI",    ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG},
    {"ORI",     ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG}, 
    {"ANDI",    ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG}, 
    {"SLLI",    ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG}, 
    {"SRLI",    ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG},
    {"SRAI",    ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG},
    {"ADD",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_REG}, 
    {"SUB",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_REG}, 
    {"SLL",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_REG}, 
    {"SLT",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_REG}, 
    {"SLTU",    ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_REG}, 
    {"XOR",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_REG}, 
    {"SRL",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_REG}, 
    {"SRA",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_REG}, 
    {"OR",      ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_REG}, 
    {"AND",     ARG0_IS_RS1 | ARG1_IS_RS2 | WRITE_REG}, 
    {"FENCE", 0}, 
    {"ECALL", 0}, 
    {"EBREAK", 0},
    {"LWU", ARG0_IS_RS1 | ARG1_IS_RS2 | ARG2_COMPUTE_PLUS}, 
    {"LD", ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | WRITE_REG}, 
    {"SD", ARG0_IS_RS1 | ARG1_IS_RS2}, 
    {"ADDIW", ARG0_IS_IMMEDIATE}, 
    {"SLLIW", ARG0_IS_IMMEDIATE}, 
    {"SRLIW", ARG0_IS_IMMEDIATE | OUT0_IS_TETRA}, 
    {"SRAIW", ARG0_IS_RS1 | ARG1_IS_IMMEDIATE | OUT0_IS_TETRA}, 
    {"ADDW", ARG0_IS_RS1 | ARG1_IS_RS2 | OUT0_IS_TETRA}, 
    {"SUBW", ARG0_IS_RS1 | ARG1_IS_RS2 | OUT0_IS_TETRA}, 
    {"SLLW", ARG0_IS_RS1 | ARG1_IS_RS2 | OUT0_IS_TETRA}, 
    {"SRLW", ARG0_IS_RS1 | ARG1_IS_RS2 | OUT0_IS_TETRA}, 
    {"SRAW", ARG0_IS_RS1 | ARG1_IS_RS2 | OUT0_IS_TETRA},
    {"FENCE_I", 0},
    {"CSRRW", 0}, {"CSRRS", 0}, {"CSRRC", 0}, {"CSRRWI", 0}, {"CSRRSI", 0}, {"CSRRCI", 0},
    {"MUL", 0}, {"MULH", 0}, {"MULHSU", 0}, {"MULHU", 0}, {"DIV", 0}, {"DIVU", 0}, {"REM", 0}, {"REMU", 0},
    {"MULW", 0}, {"DIVW", 0}, {"DIVUW", 0}, {"REMW", 0}, {"REMUW", 0} 
};

#endif