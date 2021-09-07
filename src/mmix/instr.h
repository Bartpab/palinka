#ifndef __INSTR_H__
#define __INSTR_H__

#include <stdint.h>
#include "../types.h"
#include "op.h"

typedef struct  
{
  byte op; 
  byte xx, yy, zz;
  word yz;
  
  tetra bin;
  mmix_op_info* info;
} instr_t;

instr_t op_tl_binary(octa bin)
{
  instr_t instr;

  instr.op = (bin >> 24) & 0xFF;
  instr.xx = (bin >> 16) & 0xFF;
  instr.yy = (bin >> 8) & 0xFF;
  instr.zz = bin & 0xFF;
  instr.yz = bin & 0xFFFF;
  instr.bin = bin;
  instr.info = MMIX_OP_INFOS + instr.op;

  return instr;
}

octa __mmix_instr(byte op, byte x, byte y, byte z)
{
  return (op << 24) | (x << 16) | (y << 8) | z;  
}

octa binary_tl_op(instr_t instr)
{
  return instr.op << 24 | instr.xx << 16 | instr.yy << 8 | instr.zz;
}

#endif