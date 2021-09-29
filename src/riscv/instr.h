#ifndef __RISCV_CONTROL_H__
#define __RISCV_CONTROL_H__

#include "../../lib/common/include/types.h"
#include "./opcode.h"

typedef struct {
    tetra raw;

    byte opcode;
    byte rd;
    byte funct3;
    byte funct7;
    byte rs1;
    byte rs2;
    octa imm;
    
    unsigned int op;
    struct {
        int addr;
        char type;
    } sregs[2];
    
    struct {
        int addr;
        char type;
    } dregs[2];

    bool write_pc;
    bool arg1_is_imm;
} riscv_decoded_instr_t;

tetra encode_j_type(tetra imm)
{
    tetra imm_10_1 = imm & 0x1ff;
    tetra imm_11 = (imm >> 11) & 1;
    tetra imm_19_12 = (imm >> 12) & 0xff;
    tetra imm_20 = (imm >> 20) & 1;

    return imm_19_12 | (imm_11 << 20) | (imm_10_1 << 21) | (imm_20 << 31);
}
tetra decode_j_type(tetra raw)
{
    tetra imm_19_12 = (raw >> 12) & 0xff;
    tetra imm_11 = (raw >> 20) & 1;
    tetra imm_10_1 = (raw >> 21) & 0x1ff;
    tetra imm_20 = (raw >> 31) & 1;

    return imm_10_1 | (imm_11 << 11) | (imm_19_12 << 12) | (imm_20 << 20);
}
tetra decode_b_type(tetra raw)
{
    tetra imm_11 = (raw >> 7) & 1;
    tetra imm_4_1 = (raw >> 8) & 0xf;
    tetra imm_10_5 = (raw >> 25) & 0x3f;
    tetra imm_12 = (raw >> 31) & 1;
    
    tetra tmp = imm_4_1 | (imm_10_5 << 5) | (imm_11 << 11) | (imm_12 << 12);
    return tmp;
}
tetra encode_b_type(tetra imm)
{
    tetra imm_11 = (imm >> 11) & 1;
    tetra imm_4_1 = imm & 0xf;
    tetra imm_10_5 = (imm >> 5) & 0x3f;
    tetra imm_12 = (imm >> 12) & 1;   

    tetra tmp = (imm_11 << 7) | (imm_4_1 << 8) | (imm_10_5 << 25) | (imm_12 << 31);
    return tmp;
}
tetra encode_i_type(tetra imm)
{
    return ((imm & 0xfff) << 20);
}
tetra decode_i_type(tetra raw)
{
    return (raw >> 20) & 0xfff;
}
tetra encode_s_type(tetra imm)
{
    tetra imm_4_0 = imm & 0x1f;
    tetra imm_11_5 = (imm >> 5) & 0x7f;
    return (imm_4_0 << 7) | (imm_11_5 << 25);
}
tetra decode_s_type(tetra raw)
{
    tetra imm_4_0 = (raw >> 7) & 0x1f;
    tetra imm_11_5 = (raw >> 25) & 0x7f;
    return imm_4_0 | (imm_11_5 << 5);
}
tetra encode_rd(byte rd)
{
    return (((tetra)(rd) & 0x1f) << 7);
}
byte decode_rd(tetra raw)
{
    return (raw >> 7) & 0x1f;
}
tetra encode_rs1(byte rs1)
{
    return ((tetra)(rs1 & 0x1f) << 15);
}
byte decode_rs1(tetra raw)
{
    return (raw >> 15) & 0x1f;
}
byte decode_rs2(tetra raw)
{
    return (raw >> 20) & 0x1f;
}
tetra encode_rs2(byte rs1)
{
    return ((tetra)(rs1 & 0x1f) << 20);
}
tetra encode_opcode(byte opcode)
{
    return opcode & 0x7f;
}
byte decode_opcode(tetra raw)
{
    return raw & 0x7f;
}
tetra encode_funct3(byte funct3)
{
    return ((tetra)(funct3) & 0b111) << 12;
}
tetra decode_funct3(tetra raw)
{
    return (raw >> 12) & 0b111;
}
tetra encode_funct7(byte funct7) {
    return (funct7 << 25);
}
riscv_decoded_instr_t decode(tetra raw)
{
    riscv_decoded_instr_t decoded;
    
    byte opcode = raw & 0x7f;
    byte rd = (raw >> 7) & 0x1F;
    byte funct3 = (raw >> 12) & 0x7;
    byte rs1 = (raw >> 15) & 0x1F;
    byte rs2 = (raw >> 20) & 0x1F;
    byte funct7 = (raw >> 25) & 0x7F;

    decoded.raw = raw;
    decoded.opcode = opcode;
    decoded.rd = rd;
    decoded.funct3 = funct3;
    decoded.funct7 = funct7;
    decoded.rs1 = rs1;
    decoded.rs2 = rs2;

    decoded.sregs[0].type = 0;
    decoded.sregs[0].addr = 0;
    decoded.sregs[1].type = 0;
    decoded.sregs[1].addr = 0;
    decoded.dregs[0].type = 0;
    decoded.dregs[0].addr = 0;
    decoded.dregs[1].type = 0;
    decoded.dregs[1].addr = 0;

    decoded.imm = 0;
    decoded.write_pc  = 0;
    decoded.op = 0;
    decoded.arg1_is_imm = 0;

    switch(decoded.opcode) {
        case 0b0110111: decoded.op = RISCV_LUI; goto u_type;
        case 0b0010111: decoded.op = RISCV_AUIPC; goto u_type;
        case 0b1101111: decoded.op = RISCV_JAL; goto j_type;
        case 0b1100111: decoded.op = RISCV_JALR; goto i_type;
        case 0b1100011:
            switch(decoded.funct3) {
                case 0b000: decoded.op = RISCV_BEQ; goto b_type;
                case 0b001: decoded.op = RISCV_BNE; goto b_type;
                case 0b100: decoded.op = RISCV_BLT; goto b_type;
                case 0b101: decoded.op = RISCV_BGE; goto b_type;
                case 0b110: decoded.op = RISCV_BLTU; goto b_type;
                case 0b111: decoded.op = RISCV_BGEU; goto b_type;
                default: goto __end;
            }
            break;
        case 0b0000011:
            switch(decoded.funct3) {
                case 0b000: decoded.op = RISCV_LB; goto i_type;
                case 0b001: decoded.op = RISCV_LH; goto i_type;
                case 0b010: decoded.op = RISCV_LW; goto i_type;
                case 0b100: decoded.op = RISCV_LBU; goto i_type;
                case 0b101: decoded.op = RISCV_LHU; goto i_type;
                default: goto __end;
            }
        case 0b0100011:
            switch(decoded.funct3) {
                case 0b000: decoded.op = RISCV_SB; goto s_type;
                case 0b001: decoded.op = RISCV_SH; goto s_type;
                case 0b010: decoded.op = RISCV_SW; goto s_type;
                case 0b011: decoded.op = RISCV_SD; goto s_type;
                default: goto __end;
            } 
            break;
        case 0b0010011:
            switch(decoded.funct3) {
                case 0b000: decoded.op = RISCV_ADDI; goto i_type;
                case 0b010: decoded.op = RISCV_SLTI; goto i_type;
                case 0b011: decoded.op = RISCV_SLTIU; goto i_type;
                case 0b100: decoded.op = RISCV_XORI; goto i_type;
                case 0b110: decoded.op = RISCV_ORI; goto i_type;
                case 0b111: decoded.op = RISCV_ANDI; goto i_type;
                case 0b001:
                    switch(decoded.funct7) {
                        case 0b0000000: decoded.op = RISCV_SLLI; goto i_type;
                        default: goto __end;
                    }
                case 0b101:
                    switch(decoded.funct7) {
                        case 0b0000000: decoded.op = RISCV_SRLI; goto i_type;
                        case 0b0100000: decoded.op = RISCV_SRAI; goto i_type;
                        default: goto __end;
                    }
                default: goto __end;
            }
        case 0b0110011:
            switch(decoded.funct3) {
                case 0b000:
                    switch(decoded.funct7) {
                        case 0b0000000: decoded.op = RISCV_ADD; goto r_type;
                        case 0b0100000: decoded.op = RISCV_SUB; goto r_type;
                        default: goto __end;
                    }
                case 0b001:
                    switch(decoded.funct7) {
                        case 0b0000000: decoded.op = RISCV_SLL; goto __end;
                        default: goto __end;
                    }
                case 0b010:
                    switch(decoded.funct7) {
                        case 0b0000000: decoded.op = RISCV_SLT; goto __end;
                        default: goto __end;
                    }
                case 0b011:
                    switch(decoded.funct7) {
                        case 0b0000000: decoded.op = RISCV_SLTU; goto __end;
                        default: goto __end;
                    }
                case 0b100:
                    switch(decoded.funct7) {
                        case 0b0000000: decoded.op = RISCV_XOR; goto __end;
                        default: goto __end;
                    }
                case 0b101:
                    switch(decoded.funct7) {
                        case 0b0000000: decoded.op = RISCV_SRL; goto __end;
                        case 0b0100000: decoded.op = RISCV_SRA; goto __end;
                        default: goto __end;
                    }
                case 0b110:
                    switch(decoded.funct7) {
                        case 0b0000000: decoded.op = RISCV_OR; goto __end;
                        default: goto __end;
                    }
                case 0b111:
                    switch(decoded.funct7) {
                        case 0b0000000: decoded.op = RISCV_AND; goto __end;
                        default: goto __end;
                    } 
                default: goto __end;
            }
            break;
        case 0b0001111: decoded.op = RISCV_FENCE; goto __end;
        case 0b1110011:
            if(decoded.rd == 0 && decoded.funct3 == 0 && decoded.rs1 == 0) {
                switch(decoded.rs2) {
                    case 0x0: decoded.op = RISCV_ECALL; goto __end;
                    case 0x1: decoded.op = RISCV_EBREAK; goto __end;
                    default: goto __end;
                }
            }
            switch(decoded.funct3) {
                case 0b001: decoded.op = RISCV_CSRRW; goto i_type;
                case 0b010: decoded.op = RISCV_CSRRS; goto i_type;
                case 0b011: decoded.op = RISCV_CSRRC; goto i_type;
                case 0b101: decoded.op = RISCV_CSRRWI; goto i_type;
                case 0b110: decoded.op = RISCV_CSRRSI; goto i_type;
                case 0b111: decoded.op = RISCV_CSRRCI; goto i_type;
            }
            goto __end;
        case 0b0011011:
            switch(decoded.funct3) {
                case 0b000: decoded.op = RISCV_ADDIW; goto __end;
                case 0b001: decoded.op = RISCV_SLLIW; goto __end;
                case 0b101: 
                    switch(decoded.funct7) {
                        case 0x00: decoded.op = RISCV_SRLIW; goto __end;
                        case 0x20: decoded.op = RISCV_SRAIW; goto __end;
                        default: goto __end;
                    }
                default: goto __end;
            }
            break;
        i_type: 
            decoded.imm = decode_i_type(raw);
            break;
        s_type:
            decoded.imm = ((raw >> 7) & 1) | (((raw >> 8) & 0xf) << 1) | (((raw >> 25) & 0x40) << 5);
            break;
        b_type:
            decoded.imm = decode_b_type(raw);
            break;
        u_type:
            decoded.imm = (raw >> 12) & 0x7FFFF;
            //decoded.imm = (((raw >> 12) & 0xFF) << 12) | (((raw >> 20) & 0x7FF) << 20) | (((raw >> 31) & 1) << 31);
            break;
        j_type:
            decoded.imm = decode_j_type(raw);
            break;
        r_type:
        __end:
            break;
    }

    riscv_instr_info_t* info = &riscv_instr_infos[decoded.op];
    
    int flag = info->flags;

    if((flag & ARG0_IS_RS1) == ARG0_IS_RS1) decoded.sregs[0].addr = decoded.rs1, decoded.sregs[0].type = 0;
    else if((flag & ARG0_IS_RS2) == ARG0_IS_RS2) decoded.sregs[0].addr = decoded.rs2, decoded.sregs[0].type = 0;
    
    if((flag & ARG1_IS_RS1) == ARG1_IS_RS1) decoded.sregs[1].addr = decoded.rs1, decoded.sregs[1].type = 0;
    else if((flag & ARG1_IS_RS2) == ARG1_IS_RS2) decoded.sregs[1].addr = decoded.rs2, decoded.sregs[1].type = 0;
    else if((flag & ARG1_IS_CSR) == ARG1_IS_CSR) decoded.sregs[1].addr = decoded.imm, decoded.sregs[1].type = 1;
    else if((flag & ARG1_IS_IMMEDIATE) == ARG1_IS_IMMEDIATE) decoded.arg1_is_imm = true;

    if((flag & ARG1_READ_IF_RD_NOT_X0) == ARG1_READ_IF_RD_NOT_X0 && decoded.rd == 0) decoded.sregs[1].addr = decoded.sregs[1].type = 0; // Cancel reading

    if((flag & OUT0_WRITE_REG)) decoded.dregs[0].addr = decoded.rd, decoded.dregs[0].type = 0;
    if((flag & OUT1_WRITE_CSR)) decoded.dregs[1].addr = decoded.imm, decoded.dregs[1].type = 1;
    if((flag & WRITE_PC) == WRITE_PC) decoded.write_pc = true;

    return decoded;
}

#endif