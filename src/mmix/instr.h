#ifndef __INSTR_H__
#define __INSTR_H__

#include <stdint.h>
#include "../types.h"
#include "op.h"

typedef struct  
{
  // Basics
  byte op; 
  byte xx, yy, zz;
  word yz;

  // Helpers
  octa loc;
  octa w, x, y, z, a, b, ma, mb;
  octa* x_ptr;
  octa exc;

  // Op flags
  tetra f;
  
  tetra bin;
  mmix_op_info* info;
} instr_t;

instr_t mmix_read_instr(tetra bin)
{
  instr_t instr;

  instr.op = (bin >> 24) & 0xFF;
  instr.xx = (bin >> 16) & 0xFF;
  instr.yy = (bin >> 8) & 0xFF;
  instr.zz = bin & 0xFF;
  instr.yz = bin & 0xFFFF;
  
  instr.w = instr.x = instr.y = instr.z = instr.a = instr.b = 0;
  instr.exc = instr.ma = instr.mb = 0;
  instr.x_ptr = NULL;

  instr.bin = bin;
  instr.info = MMIX_OP_INFOS + instr.op;
  instr.f = instr.info->flags;

  return instr;
}

tetra __mmix_instr(byte op, byte x, byte y, byte z)
{
  return (op << 24) | (x << 16) | (y << 8) | z;  
}

tetra mmix_write_instr(instr_t instr)
{
  return instr.op << 24 | instr.xx << 16 | instr.yy << 8 | instr.zz;
}

#endif