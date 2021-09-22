#ifndef __RISCV_CONTROL_H__
#define __RISCV_CONTROL_H__

#include "../../lib/common/include/types.h"

typedef struct {
    tetra raw;
    
    struct {
        byte opcode;
        byte rd;
        byte funct3;
        byte funct7;
        byte rs1;
        byte rs2;
        octa shamt; 
        octa imm;
    } decoded;

    unsigned int op;
    struct riscv_instr_info_t* infos;
    int stage;
    
    struct {
        octa flag;   //
        octa* ptr;   // if !is_immediate
        octa value;  // value
    } args[4];

    // Read/write registers
    struct {
        octa* ptr;
        octa value;
        octa flag;
    } out[3];

} riscv_control_t;

#endif