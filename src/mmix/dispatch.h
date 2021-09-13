#ifndef __MMIX_DISPATCH_H__
#define __MMIX_DISPATCH_H__

#include <math.h>

#include "instr.h"
#include "mem.h"
#include "processor.h"

#include "../arith.h"
#include "../sys.h"
#include "../types.h"

typedef void (*mmix_exec_t)(system_t* sys, mmix_processor_t* proc, instr_t* instr);
#define MEX_ARGS sys, proc, instr
#define MEX_DEF_ARGS system_t * sys, mmix_processor_t* proc,instr_t * instr
#define MEXF(op_name) void mmix_ ##op_name (MEX_DEF_ARGS)
#define MEXN(op_name) mmix_##op_name

#define MMEMR(addr, type, out) MMIX_MEM_READ(sys, addr, type, out)
#define MMEMW(addr, type, val) MMIX_MEM_WRITE(sys, addr, type, val)
#define MMIX_SREG(REG) proc->g[REG]

#define EXCEPTION_MMIX_SREG proc->g[rA]
#define REMAINDER_MMIX_SREG proc->g[rR]

bool __mmix_lring_is_full(mmix_processor_t* proc);
unsigned int __mmix_lring_push_local(system_t* sys, mmix_processor_t* proc);

void mmix_read_regv(mmix_processor_t* proc, octa* x, unsigned char xx);
octa mmix_get_regv(mmix_processor_t* proc, unsigned char xx);
void mmix_set_regv(mmix_processor_t* proc, octa x, unsigned int xx);
void mmix_set_sregv(mmix_processor_t* proc, octa x, unsigned char xx);
octa mmix_get_sregv(mmix_processor_t* proc, unsigned char xx);

void __mmix_stack_store(system_t* sys);
void __mmix_stack_load(system_t* sys);

void __mmix_sclock_incr(mmix_processor_t* proc, unsigned int s);

//////////
// IMPL //
//////////

void __mmix_sclock_incr(mmix_processor_t* proc, unsigned int s) {
  proc->sclock = octa_incr(proc->sclock, s);
  if(octa_unsigned_cmp(MMIX_SREG(rI), uint_to_octa(s)) > 0) {
    MMIX_SREG(rI) = octa_incr(MMIX_SREG(rI), s);
  } else {
    MMIX_SREG(rI) = 0;
  }
}

static int __reg_truth(octa o, unsigned char op) {
  register int b;
  switch((op >> 1) & 0x3) {
    case 0: 
      b = octa_signed_cmp(o, octa_zero) == -1; 
    break; // Negative ?
    default: case 1: 
      b = octa_eq(o, octa_zero);
    break; // Zero ?
    case 2: 
      b = octa_signed_cmp(o, octa_zero) == 1; 
    break; // Positive ?
  }

  if(op & 0x8) return b ^ 1;
  
  return b;
}

//////////
// IMPL //
//////////

void mmix_read_regv(mmix_processor_t* proc, octa* x, unsigned char xx)
{
  if(xx > proc->G) 
    *x = proc->g[xx];
  else if(xx < proc->L) 
    *x = proc->l[(proc->O + xx) & proc->lmask];
}

octa mmix_get_regv(mmix_processor_t* proc, unsigned char xx) 
{
  octa e = 0;
  mmix_read_regv(proc, &e, xx);
  return e;
}

octa mmix_get_sregv(mmix_processor_t* proc, unsigned char xx)
{
  return proc->g[xx];
}

void mmix_set_regv(mmix_processor_t* proc, octa x, unsigned int xx) 
{
  if(xx > proc->G) 
    proc->g[xx] = x;
  else if(xx < proc->L) 
    proc->l[(proc->O + xx) & proc->lmask] = x;
}

void mmix_set_sregv(mmix_processor_t* proc, octa x, unsigned char xx)
{
  proc->g[xx] = x;
}

bool __mmix_lring_is_full(mmix_processor_t* proc)
{
  if((proc->S - proc->O - proc->L) & proc->lmask)
    return true;
  
  return false;
}

unsigned int __mmix_lring_push_local(system_t* sys, mmix_processor_t* proc)
{
  proc->l[(proc->O + proc->L) & proc->lmask] = octa_zero;
  proc->L = proc->g[rL] = proc->L + 1;
        
  if(__mmix_lring_is_full(proc)) 
    __mmix_stack_store(sys); 

  return proc->L;
}

static inline void __store_x(MEX_DEF_ARGS) 
{
  *instr->x_ptr = instr->x;
}

void __mmix_stack_store(system_t* sys)
{
  mmix_processor_t* proc = __get_mmix_proc(sys);
  unsigned int k = proc->S & proc->lmask;

  MMEMW(
    octa_to_voidp(proc->g[rS]),
    octa, 
    proc->l[k]
  );

  proc->g[rS] = octa_incr(proc->g[rS], 8); proc->S++;
}

void __mmix_stack_load(system_t* sys) 
{
  mmix_processor_t* proc = __get_mmix_proc(sys);
  unsigned int k = proc->S & proc->lmask;

  proc->S--, proc->g[rS] = octa_incr(proc->g[rS], -8);
  
  MMEMW(
    octa_to_voidp(proc->g[rS]),
    octa, 
    proc->l[k]
  );
}

MEXF(TRAP) 
{
  MMIX_SREG(rWW) = (octa) proc->instr_ptr;
  MMIX_SREG(rXX) = tetra_to_octa(0x80000000, mmix_write_instr(*instr));
  MMIX_SREG(rYY) = instr->y, MMIX_SREG(rZZ) = instr->z;

  instr->z = tetra_to_octa(0, instr->zz);
  instr->a = octa_incr(instr->b, 8);

  if(proc->ivt[instr->xx].hdlr != NULL)
    proc->ivt[instr->xx].hdlr(sys, instr);

  instr->x = proc->g[0xFF] = proc->g[rBB];
}

MEXF(FCMP) 
{
  instr->x = octa_fcmp(instr->y, instr->z, &instr->exc);
  __store_x(MEX_ARGS);
}

MEXF(FUN) 
{
  instr->x = octa_fun(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(FEQL) 
{
  instr->x = octa_feql(instr->y, instr->x, &instr->exc);
  __store_x(MEX_ARGS);
}

MEXF(FADD) 
{
  instr->x = octa_fadd(instr->y, instr->z, &instr->exc);
  __store_x(MEX_ARGS);
}

MEXF(FIX) 
{
  instr->x = octa_fix(instr->z, &instr->exc);
  __store_x(MEX_ARGS);
}

MEXF(FSUB) 
{
  instr->x = octa_fsub(instr->y, instr->z, &instr->exc);
  __store_x(MEX_ARGS);
}

MEXF(FIXU) 
{
  instr->x = octa_fixu(instr->z, &instr->exc);
  __store_x(MEX_ARGS);
}

static inline void __flot(MEX_DEF_ARGS) {
  instr->x = octa_flot(instr->z, proc->rounding_mode, &instr->exc);
  __store_x(MEX_ARGS);
}

MEXF(FLOT) 
{
  __flot(MEX_ARGS);
}

MEXF(FLOTI) 
{
  __flot(MEX_ARGS);
}

MEXF(FLOTU) 
{
  __flot(MEX_ARGS);
}
MEXF(FLOTUI) 
{
  __flot(MEX_ARGS);
}

MEXF(SFLOT) {}
MEXF(SFLOTI) {}
MEXF(SFLOTU) {}
MEXF(SFLOTUI) {}

MEXF(FMUL) 
{
  instr->x = octa_fmult(instr->y, instr->z, &instr->exc);
  __store_x(MEX_ARGS);
}

MEXF(FCMPE) {}
MEXF(FUNE) {}
MEXF(FEQLE) {}

MEXF(FDIV) 
{
  instr->x = octa_fdiv(instr->y, instr->z, &instr->exc);
  __store_x(MEX_ARGS);
}

MEXF(FSQRT) 
{
  instr->x = octa_fsqrt(instr->z, ol(instr->x), &instr->exc);
  __store_x(MEX_ARGS);
}

MEXF(FREM) {
  instr->x = octa_frem(instr->y, instr->z, &instr->exc);
  __store_x(MEX_ARGS);
}

MEXF(FINT) {
  instr->x = octa_fint(instr->z, proc->rounding_mode, &instr->exc);
}

static inline void __signed_mul(MEX_DEF_ARGS)
{
  bool overflow; 
  octa aux;

  instr->x = octa_signed_mult(instr->y, instr->z, &aux, &overflow);

  if(overflow)
    instr->exc |= V_BIT;

  __store_x(MEX_ARGS);
}

MEXF(MUL) 
{
  __signed_mul(MEX_ARGS);
}

MEXF(MULI) 
{
  __signed_mul(MEX_ARGS);
}

static inline void __unsigned_mul(MEX_DEF_ARGS)
{
  bool overflow;
  octa aux;

  instr->x = octa_mult(instr->y, instr->z, &aux, &overflow);

  if(overflow)
    instr->exc |= V_BIT;

  MMIX_SREG(rH) = aux;
  __store_x(MEX_ARGS);
}

MEXF(MULU) {
  __unsigned_mul(MEX_ARGS);
}

MEXF(MULUI) {
  __unsigned_mul(MEX_ARGS);
}

static inline void __signed_div(MEX_DEF_ARGS)
{
  octa aux;
  hexadeca yy = {instr->y & sign_bit, instr->y};
  
  instr->x = octa_signed_div(yy, instr->z, &aux);
  
  REMAINDER_MMIX_SREG = aux;

  __store_x(MEX_ARGS);
}

MEXF(DIV) {
  __signed_div(MEX_ARGS);
}

MEXF(DIVI) {
  __signed_div(MEX_ARGS);
}

static inline void __unsigned_div(MEX_DEF_ARGS)
{
  hexadeca yy = {MMIX_SREG(rD), instr->y};
  instr->x = octa_div(yy, instr->z, &REMAINDER_MMIX_SREG);
  __store_x(MEX_ARGS);
}

MEXF(DIVU) 
{
  __unsigned_div(MEX_ARGS);
}

MEXF(DIVUI) 
{
  __unsigned_div(MEX_ARGS);
}

static inline void __add(MEX_DEF_ARGS, bool test_overflow)
{
  bool overflow = false;

  instr->x = octa_plus(instr->y, instr->z, &overflow);
  
  if(overflow && test_overflow)
    instr->exc |= V_BIT;

  __store_x(MEX_ARGS);
}

MEXF(ADD) {
  __add(MEX_ARGS, true);
}
MEXF(ADDI) {
  __add(MEX_ARGS, true);
}
MEXF(ADDU) {
  __add(MEX_ARGS, false);
}
MEXF(ADDUI) {
  __add(MEX_ARGS, false);
}

static inline void __sub(MEX_DEF_ARGS, bool test_overflow) {
  bool overflow = false;

  instr->x = octa_minus(instr->y, instr->z, &overflow);
  
  if(test_overflow && overflow) 
    instr->exc |= V_BIT;

  __store_x(MEX_ARGS);
}

MEXF(SUB) {
  __sub(MEX_ARGS, true);
}
MEXF(SUBI) {
  __sub(MEX_ARGS, true);
}
MEXF(SUBU) {
  __sub(MEX_ARGS, false);
}
MEXF(SUBUI) {
  __sub(MEX_ARGS, false);
}

static inline void __unsigned_2add(MEX_DEF_ARGS){
  bool overflow;
  instr->x = octa_plus(
    octa_plus(instr->y, instr->y, &overflow), 
    instr->z, &overflow
  );

  __store_x(MEX_ARGS);
}

static inline void __unsigned_4add(MEX_DEF_ARGS) {
  bool overflow;
  
  instr->x = octa_plus(
    octa_plus(
      octa_plus(instr->y, instr->y, &overflow), 
      octa_plus(instr->y, instr->y, &overflow),
      &overflow
    ), instr->z, &overflow
  );

  __store_x(MEX_ARGS);
}

static inline void __unsigned_8add(MEX_DEF_ARGS) {
  bool overflow;

  instr->x = octa_plus(
    octa_plus(
      octa_plus(
        octa_plus(instr->y, instr->y, &overflow), 
        octa_plus(instr->y, instr->y, &overflow),
        &overflow
      ), 
      octa_plus(
        octa_plus(instr->y, instr->y, &overflow), 
        octa_plus(instr->y, instr->y, &overflow),
        &overflow
      ), &overflow
    ), instr->z, &overflow
  );

  __store_x(MEX_ARGS);
}

static inline void __unsigned_16add(MEX_DEF_ARGS) {
  bool overflow;

  instr->x = octa_plus(
    octa_plus(
      octa_plus(
        octa_plus(
          octa_plus(instr->y, instr->y, &overflow), 
          octa_plus(instr->y, instr->y, &overflow), 
          &overflow), 
        octa_plus(
          octa_plus(instr->y, instr->y, &overflow), 
          octa_plus(instr->y, instr->y, &overflow), 
          &overflow),
        &overflow
      ), 
      octa_plus(
        octa_plus(
          octa_plus(instr->y, instr->y, &overflow), 
          octa_plus(instr->y, instr->y, &overflow), 
          &overflow), 
        octa_plus(
          octa_plus(instr->y, instr->y, &overflow), 
          octa_plus(instr->y, instr->y, &overflow), 
          &overflow),
        &overflow
      ), &overflow
    ), instr->z, &overflow
  );

  __store_x(MEX_ARGS);
}

MEXF(IIADDU) {
  __unsigned_2add(MEX_ARGS);
}
MEXF(IIADDUI) {
  __unsigned_2add(MEX_ARGS);
}
MEXF(IVADDU) {
  __unsigned_4add(MEX_ARGS);
}
MEXF(IVADDUI) {
  __unsigned_4add(MEX_ARGS);
}
MEXF(VIIIADDU) {
  __unsigned_8add(MEX_ARGS);
}
MEXF(VIIIADDUI) {
  __unsigned_8add(MEX_ARGS);
}
MEXF(XVIADDU) {
  __unsigned_16add(MEX_ARGS);
}
MEXF(XVIADDUI) {
  __unsigned_16add(MEX_ARGS);
}

static inline void __cmp(MEX_DEF_ARGS, bool is_signed) {
  instr->x = !is_signed ? octa_unsigned_cmp(instr->y, instr->z) : octa_signed_cmp(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(CMP) {
  __cmp(sys, proc, instr, true);
}
MEXF(CMPI) {
  __cmp(sys, proc, instr, true);
}
MEXF(CMPU) {
  __cmp(sys, proc, instr, false);
}
MEXF(CMPUI) {
  __cmp(sys, proc, instr, false);
}

static inline void __neg(MEX_DEF_ARGS, bool test_overflow) {
  bool overflow = false;

  instr->x = octa_minus(instr->y, instr->z, &overflow);

  if(test_overflow && overflow)
    instr->exc |= V_BIT;

  __store_x(MEX_ARGS);
}

MEXF(NEG) {
  __neg(MEX_ARGS, true);
}
MEXF(NEGI) {
  __neg(MEX_ARGS, true);
}
MEXF(NEGU) {
  __neg(MEX_ARGS, false);
}
MEXF(NEGUI) {
  __neg(MEX_ARGS, false);
}

static inline void __sl(MEX_DEF_ARGS) {
  instr->x = octa_left_shift(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(SL) {
  __sl(MEX_ARGS);
}
MEXF(SLI) {
  __sl(MEX_ARGS);
}
MEXF(SLU) {
  __sl(MEX_ARGS);
}
MEXF(SLUI) {
  __sl(MEX_ARGS);
}

static inline void __sr(MEX_DEF_ARGS) {
  instr->x = octa_right_shift(instr->y, instr->z, instr->op & 0x2);
  __store_x(MEX_ARGS);
}

MEXF(SR) {
  __sr(MEX_ARGS);
}
MEXF(SRI) {
  __sr(MEX_ARGS);
}
MEXF(SRU) {
  __sr(MEX_ARGS);
}
MEXF(SRUI) {
  __sr(MEX_ARGS);
}

static inline void __bn(MEX_DEF_ARGS)
{
  bool good_assumption;

  instr->x = int_to_octa(
    __reg_truth(
      instr->b, 
      instr->op
    )
  );

  if(octa_signed_cmp(instr->x, octa_zero)) 
  {
    proc->instr_ptr = (octa*) instr->z;
    good_assumption = (instr->op >= PBN);
  } else {
    good_assumption = instr->op < PBN;
  }

  if(good_assumption) {} 
  else {
    __mmix_sclock_incr(proc, 2); // Penalty aïe !
  }
}

MEXF(BN) {
  __bn(MEX_ARGS);
}
MEXF(BNB) {
  __bn(MEX_ARGS);
}
MEXF(BZ) {
  __bn(MEX_ARGS);
}
MEXF(BZB) {
  __bn(MEX_ARGS);
}
MEXF(BP) {
  __bn(MEX_ARGS);
}
MEXF(BPB) {
  __bn(MEX_ARGS);
}
MEXF(BOD) {
  __bn(MEX_ARGS);
}
MEXF(BODB) {
  __bn(MEX_ARGS);
}
MEXF(BNN) {
  __bn(MEX_ARGS);
}
MEXF(BNNB) {
  __bn(MEX_ARGS);
}
MEXF(BNZ) {
  __bn(MEX_ARGS);
}
MEXF(BNZB) {
  __bn(MEX_ARGS);
}
MEXF(BNP) {
  __bn(MEX_ARGS);
}
MEXF(BNPB) {
  __bn(MEX_ARGS);
}
MEXF(BEV) {
  __bn(MEX_ARGS);
}
MEXF(BEVB) {
  __bn(MEX_ARGS);
}
MEXF(PBN) {
  __bn(MEX_ARGS);
}
MEXF(PBNB) {
  __bn(MEX_ARGS);
}
MEXF(PBZ) {
  __bn(MEX_ARGS);
}
MEXF(PBZB) {
  __bn(MEX_ARGS);
}
MEXF(PBP) {
  __bn(MEX_ARGS);
}
MEXF(PBPB) {
  __bn(MEX_ARGS);
}
MEXF(PBOD) {
  __bn(MEX_ARGS);
}
MEXF(PBODB) {
  __bn(MEX_ARGS);
}
MEXF(PBNN) {
  __bn(MEX_ARGS);
}
MEXF(PBNNB) {
  __bn(MEX_ARGS);
}
MEXF(PBNZ) {
  __bn(MEX_ARGS);
}
MEXF(PBNZB) {
  __bn(MEX_ARGS);
}
MEXF(PBNP) {
  __bn(MEX_ARGS);
}
MEXF(PBNPB) {
  __bn(MEX_ARGS);
}
MEXF(PBEV) {
  __bn(MEX_ARGS);
}
MEXF(PBEVB) {
  __bn(MEX_ARGS);
}

static inline void __cs(MEX_DEF_ARGS) {
  instr->x = __reg_truth(instr->y, instr->op) ? instr->z : instr->b;
  __store_x(MEX_ARGS);
}

MEXF(CSN) {
  __cs(MEX_ARGS);
}
MEXF(CSNI) {
  __cs(MEX_ARGS);
}
MEXF(CSZ) {
  __cs(MEX_ARGS);
}
MEXF(CSZI) {
  __cs(MEX_ARGS);
}
MEXF(CSP) {
  __cs(MEX_ARGS);
}
MEXF(CSPI) {
  __cs(MEX_ARGS);
}
MEXF(CSOD) {
  __cs(MEX_ARGS);
}
MEXF(CSODI) {
  __cs(MEX_ARGS);
}
MEXF(CSNN) {
  __cs(MEX_ARGS);
}
MEXF(CSNNI) {
  __cs(MEX_ARGS);
}
MEXF(CSNZ) {
  __cs(MEX_ARGS);
}
MEXF(CSNZI) {
  __cs(MEX_ARGS);
}
MEXF(CSNP) {
  __cs(MEX_ARGS);
}
MEXF(CSNPI) {
  __cs(MEX_ARGS);
}
MEXF(CSEV) {
  __cs(MEX_ARGS);
}
MEXF(CSEVI) {
  __cs(MEX_ARGS);
}
MEXF(ZSN) {
  __cs(MEX_ARGS);
}
MEXF(ZSNI) {
  __cs(MEX_ARGS);
}
MEXF(ZSZ) {
  __cs(MEX_ARGS);
}
MEXF(ZSZI) {
  __cs(MEX_ARGS);
}
MEXF(ZSP) {
  __cs(MEX_ARGS);
}
MEXF(ZSPI) {
  __cs(MEX_ARGS);
}
MEXF(ZSOD) {
  __cs(MEX_ARGS);
}
MEXF(ZSODI) {
  __cs(MEX_ARGS);
}
MEXF(ZSNN) {
  __cs(MEX_ARGS);
}
MEXF(ZSNNI) {
  __cs(MEX_ARGS);
}
MEXF(ZSNZ) {
  __cs(MEX_ARGS);
}
MEXF(ZSNZI) {
  __cs(MEX_ARGS);
}
MEXF(ZSNP) {
  __cs(MEX_ARGS);
}
MEXF(ZSNPI) {
  __cs(MEX_ARGS);
}
MEXF(ZSEV) {
  __cs(MEX_ARGS);
}
MEXF(ZSEVI) {
  __cs(MEX_ARGS);
}

static inline void __ldb(MEX_DEF_ARGS) 
{
  byte out;  
  MMEMR((void*)(instr->y + instr->z), byte, out);
  instr->x = tetra_to_octa(0, out);
  __store_x(MEX_ARGS);
}

MEXF(LDB) {
  __ldb(MEX_ARGS);
}

MEXF(LDBI) {
  __ldb(MEX_ARGS);
}

MEXF(LDBU) {
  __ldb(MEX_ARGS);
}

MEXF(LDBUI) {
  __ldb(MEX_ARGS);
}

static inline void __ldw(MEX_DEF_ARGS) 
{
  word out;  
  MMEMR((void*)(instr->y + instr->z), word, out);
  instr->x = tetra_to_octa(0, out);
  __store_x(MEX_ARGS);
}

MEXF(LDW) {
  __ldw(MEX_ARGS);
}
MEXF(LDWI) {
  __ldw(MEX_ARGS);
}
MEXF(LDWU) {
  __ldw(MEX_ARGS);
}
MEXF(LDWUI) {
  __ldw(MEX_ARGS);
}

static inline void __ldt(MEX_DEF_ARGS) 
{
  tetra out;  
  MMEMR((void*)(instr->y + instr->z), tetra, out);
  instr->x = tetra_to_octa(0, out);
  __store_x(MEX_ARGS);
}

MEXF(LDT) {
  __ldt(MEX_ARGS);
}
MEXF(LDTI) {
  __ldt(MEX_ARGS);
}
MEXF(LDTU) {
  __ldt(MEX_ARGS);
}
MEXF(LDTUI) {
  __ldt(MEX_ARGS);
}

static inline void __ldo(MEX_DEF_ARGS) 
{
  bool overflow = false;
  octa a = octa_plus(instr->y, instr->z, &overflow);
  MMEMR(octa_to_voidp(a), octa, instr->x);
  __store_x(MEX_ARGS);
}

MEXF(LDO) {
  __ldo(MEX_ARGS);
}
MEXF(LDOI) {
  __ldo(MEX_ARGS);
}
MEXF(LDOU) {
  __ldo(MEX_ARGS);
}
MEXF(LDOUI) {
  __ldo(MEX_ARGS);
}

static inline void __ldsf(MEX_DEF_ARGS) {
  MMEMR(
    octa_to_voidp(instr->w),
    octa,
    instr->x
  );

  __store_x(MEX_ARGS);
}

MEXF(LDSF) {
  __ldsf(MEX_ARGS);
}

MEXF(LDSFI) {
  __ldsf(MEX_ARGS);
}

static inline void __ldht(MEX_DEF_ARGS) {
  MMEMR(
    octa_to_voidp(instr->w),
    octa,
    instr->x
  );

  instr->x = tetra_to_octa(oh(instr->x), 0);
  __store_x(MEX_ARGS);
}

MEXF(LDHT) {
  __ldht(MEX_ARGS);
}
MEXF(LDHTI) {
  __ldht(MEX_ARGS);
}

static inline void __cswap(MEX_DEF_ARGS)
{
  void* v;
  octa b;

  v = octa_to_voidp(instr->w);
  MMEMR(v, octa, b);

  if(octa_eq(b, proc->g[rP])) {
    MMEMW(v, octa, instr->b);
    instr->x = int_to_octa(1);
  } else {
    proc->g[rP] = b;
    instr->x = octa_zero;
  }

  __store_x(MEX_ARGS);
}

MEXF(CSWAP) {
  __cswap(MEX_ARGS);
}
MEXF(CSWAPI) {
  __cswap(MEX_ARGS);
}

MEXF(LDUNC) {}
MEXF(LDUNCI) {}
MEXF(LDVTS) {}
MEXF(LDVTSI) {}
MEXF(PRELD) {}
MEXF(PRELDI) {}
MEXF(PREGO) {}
MEXF(PREGOI) {}

static inline void __go(MEX_DEF_ARGS) {
  instr->x = (octa) proc->instr_ptr;
  proc->instr_ptr = (octa*) instr->w;
  __store_x(MEX_ARGS);
}

MEXF(GO) {
  __go(MEX_ARGS);
}
MEXF(GOI) {
  __go(MEX_ARGS);
}

static void __stb(MEX_DEF_ARGS, bool test_overflow)
{
  byte b = instr->b;

  MMEMW(
    octa_to_voidp(instr->w), 
    byte, 
    b
  );

  if(b != instr->x && test_overflow) {
    instr->exc |= I_BIT;
  }
}

MEXF(STB) {
  __stb(MEX_ARGS, true);
}
MEXF(STBI) {
  __stb(MEX_ARGS, true);
}
MEXF(STBU) {
  __stb(MEX_ARGS, false);
}
MEXF(STBUI) {
  __stb(MEX_ARGS, false);
}

static inline void __stw(MEX_DEF_ARGS, bool test_overflow)
{
  word w = instr->b;

  MMEMW((void*)(instr->y + instr->z), word, w);

  if(w != instr->x && test_overflow) {
    instr->exc |= I_BIT;
  }
}

MEXF(STW) {
  __stw(MEX_ARGS, true);
}
MEXF(STWI) {
  __stw(MEX_ARGS, true);
}
MEXF(STWU) {
  __stw(MEX_ARGS, false);
}
MEXF(STWUI) {
  __stw(MEX_ARGS, false);
}

static inline void __stt(MEX_DEF_ARGS, bool test_overflow)
{
  tetra t = instr->b;

  MMEMW(
    octa_to_voidp(instr->w), 
    tetra, 
    t
  );

  if(t != instr->x && test_overflow) {
    instr->exc |= I_BIT;
  }
}

MEXF(STT) {
  __stt(MEX_ARGS, true);
}
MEXF(STTI) {
  __stt(MEX_ARGS, true);
}
MEXF(STTU) {
  __stt(MEX_ARGS, false);
}
MEXF(STTUI) {
  __stt(MEX_ARGS, false);
}

static inline void __sto(MEX_DEF_ARGS)
{
  octa o = instr->b;

  MMEMW(
    octa_to_voidp(instr->w), 
    octa, 
    instr->b
  );
}

MEXF(STO) {
  __sto(MEX_ARGS);
}
MEXF(STOI) {
  __sto(MEX_ARGS);
}
MEXF(STOU) {
  __sto(MEX_ARGS);
}
MEXF(STOUI) {
  __sto(MEX_ARGS);
}

static inline void __stsf(MEX_DEF_ARGS)
{
  MMEMW(
    octa_to_voidp(instr->w), 
    octa, 
    instr->b
  );
}

MEXF(STSF) {
  __stsf(MEX_ARGS);
}

MEXF(STSFI) {
  __stsf(MEX_ARGS);
}

static inline void __stht(MEX_DEF_ARGS)
{
  MMEMW(
    octa_to_voidp(instr->w), 
    tetra, 
    oh(instr->b)
  );
}

MEXF(STHT) {
  __stht(MEX_ARGS);
}

MEXF(STHTI) {
  __stht(MEX_ARGS);
}

static inline void __stco(MEX_DEF_ARGS)
{
  MMEMW(
    octa_to_voidp(instr->w), 
    octa, 
    instr->b
  );
}

MEXF(STCO) {
  __stco(MEX_ARGS);
}

MEXF(STCOI) {
  __stco(MEX_ARGS);
}

MEXF(STUNC) {
  __sto(MEX_ARGS);
}

MEXF(STUNCI) {
  __sto(MEX_ARGS);
}

MEXF(SYNCD) {}
MEXF(SYNCDI) {}
MEXF(PREST) {}
MEXF(PRESTI) {}
MEXF(SYNCID) {}
MEXF(SYNCIDI) {}
MEXF(PUSHGO) {}
MEXF(PUSHGOI) {}

static inline void __or(MEX_DEF_ARGS) {
  instr->x = octa_or(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(OR) {
  __or(MEX_ARGS);
}
MEXF(ORI) {
  __or(MEX_ARGS);
}

static inline void __orn(MEX_DEF_ARGS) {
  instr->x = octa_orn(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(ORN) {
  __orn(MEX_ARGS);
}

MEXF(ORNI) {
  __orn(MEX_ARGS);
}

static inline void __nor(MEX_DEF_ARGS) {
  instr->x = octa_orn(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(NOR) {
  __nor(MEX_ARGS);
}

MEXF(NORI) {
  __nor(MEX_ARGS);
}

static inline void __xor(MEX_DEF_ARGS) {
  instr->x = octa_xor(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(XOR) {
  __xor(MEX_ARGS);
}
MEXF(XORI) {
  __xor(MEX_ARGS);
}

static inline void __and(MEX_DEF_ARGS) {
  instr->x = octa_and(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(AND) {
  __and(MEX_ARGS);
}
MEXF(ANDI) {
  __and(MEX_ARGS);
}

static inline void __andn(MEX_DEF_ARGS) {
  instr->x = octa_andn(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(ANDN) {
  __andn(MEX_ARGS);
}
MEXF(ANDNI) {
  __andn(MEX_ARGS);
}

static inline void __nand(MEX_DEF_ARGS) {
  instr->x = octa_nand(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(NAND) {
  __nand(MEX_ARGS);
}
MEXF(NANDI) {
  __nand(MEX_ARGS);
}

static inline void __nxor(MEX_DEF_ARGS) {
  instr->x = octa_nxor(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(NXOR) {
  __nxor(MEX_ARGS);
}
MEXF(NXORI) {
  __nxor(MEX_ARGS);
}

static inline void __bdif(MEX_DEF_ARGS) {
  instr->x = octa_bdif(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(BDIF) {
  __bdif(MEX_ARGS);
}
MEXF(BDIFI) {
  __bdif(MEX_ARGS);
}

static inline void __wdif(MEX_DEF_ARGS) {
  instr->x = octa_wdif(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(WDIF) {
  __wdif(MEX_ARGS);  
}
MEXF(WDIFI) {
  __wdif(MEX_ARGS);
}

static inline void __tdif(MEX_DEF_ARGS) {
  instr->x = octa_tdif(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(TDIF) {
  __tdif(MEX_ARGS);
}
MEXF(TDIFI) {
  __tdif(MEX_ARGS);
}

static inline void __odif(MEX_DEF_ARGS) {
  instr->x = octa_odif(instr->y, instr->z);
  __store_x(MEX_ARGS);
}

MEXF(ODIF) {
  __odif(MEX_ARGS);
}
MEXF(ODIFI) {
  __odif(MEX_ARGS);
}

static inline void __mux(MEX_DEF_ARGS) {
  instr->x = octa_mux(
    instr->y, 
    instr->z, 
    proc->g[rM]
  );
  
  __store_x(MEX_ARGS);
}

MEXF(MUX) {
  __mux(MEX_ARGS);
}

MEXF(MUXI) {
  __mux(MEX_ARGS);
}

static inline void __sadd(MEX_DEF_ARGS) {
  instr->x = unsigned_char_to_octa(
    octa_count_bits(
      octa_and(
        instr->x, 
        octa_compl(instr->z)
      )
    )
  );

  __store_x(MEX_ARGS);
}

MEXF(SADD) {
  __sadd(MEX_ARGS);
}
MEXF(SADDI) {
  __sadd(MEX_ARGS);
}

MEXF(MOR) {}
MEXF(MORI) {}
MEXF(MXOR) {}
MEXF(MXORI) {}

MEXF(SETH) {
    instr->x = instr->z;
  __store_x(MEX_ARGS);
}
MEXF(SETMH) {
  instr->x = instr->z;
  __store_x(MEX_ARGS);
}
MEXF(SETML) {
  instr->x = instr->z;
  __store_x(MEX_ARGS);
}
MEXF(SETL) {
  instr->x = instr->z;
  __store_x(MEX_ARGS);
}

MEXF(INCH) {
  instr->x = instr->w;
  __store_x(MEX_ARGS);
}

MEXF(INCMH) {
  instr->x = instr->z;
  __store_x(MEX_ARGS);
}

MEXF(INCML) {
  instr->x = instr->z;
  __store_x(MEX_ARGS);
}

MEXF(INCL) {
  instr->x = instr->z;
  __store_x(MEX_ARGS);
}

MEXF(ORH) {
  __or(MEX_ARGS);
}
MEXF(ORMH) {
  __or(MEX_ARGS);
}
MEXF(ORML) {
  __or(MEX_ARGS);
}
MEXF(ORL) {
  __or(MEX_ARGS);
}
MEXF(ANDNH) {
  __andn(MEX_ARGS);
}
MEXF(ANDNMH) {
  __andn(MEX_ARGS);
}
MEXF(ANDNML) {
  __andn(MEX_ARGS);
}
MEXF(ANDNL) {
  __andn(MEX_ARGS);
}

static inline void __jmp(MEX_DEF_ARGS) {
  proc->instr_ptr = (octa*)(instr->z);
}

MEXF(JMP) {
  __jmp(MEX_ARGS);
}
MEXF(JMPB) {
  __jmp(MEX_ARGS);
}

static inline void __push(MEX_DEF_ARGS)
{
  unsigned int raddr;
  octa x, b;
  
  while(instr->xx >= proc->G) 
    instr->xx = __mmix_lring_push_local(sys, proc);

  x = tetra_to_octa(0, instr->xx);
  mmix_set_regv(proc, x, instr->xx);

  // Save the address of return
  x = proc->g[rJ] = octa_incr(instr->loc, 4);
  
  // Decrease L pointer
  proc->L -= instr->xx + 1; 
  proc->g[rL] = tetra_to_octa(0, proc->L);

  // Increase O pointer
  proc->O += instr->xx + 1;
  proc->g[rO] = octa_incr(
    proc->g[rO], 
    (instr->xx + 1) << 3
  );
}

MEXF(PUSHJ) {
  proc->instr_ptr = (octa*) instr->z;
  __push(MEX_ARGS);
}
MEXF(PUSHJB) {
  proc->instr_ptr = (octa*) instr->z;
  __push(MEX_ARGS);
}

MEXF(GETA) {
  instr->x = instr->z;
  __store_x(MEX_ARGS);
}
MEXF(GETAB) {
  instr->x = instr->z;
  __store_x(MEX_ARGS);
}
MEXF(PUT) {}
MEXF(PUTI) {}
MEXF(POP) {}

MEXF(RESUME) 
{
  octa b, z;
  z = proc->g[instr->zz];
  proc->instr_ptr = (octa*) proc->g[rW];
  z = proc->g[rW];
  b = proc->g[rX];
}

MEXF(SAVE) 
{
  unsigned int k;

  if(instr->xx < proc->G 
    || instr->yy != 0 
    || instr->zz != 0)
    return; // Illegal
  
  proc->l[(proc->O + proc->L) & proc->lmask] = tetra_to_octa(0, proc->L);
  proc->L++;

  if(((proc->S-proc->L-proc->O) & proc->lmask) == 0) 
    __mmix_stack_store(sys);

  proc->O += proc->L;
  proc->g[rO] = octa_incr(proc->g[rO], proc->L << 3);
  proc->L = 0; proc->g[rL] = octa_zero;

  while(proc->g[rO] != proc->g[rS]) 
    __mmix_stack_store(sys);
  
  for(k = proc->G;;) {
    // Store g[k] in the register stack.
    if(k == rZ + 1) {
      instr->x = tetra_to_octa(
        proc->G,
        ol(proc->g[rA])
      );
    } else {
      instr->x = proc->g[k];
    }

    MMEMW(
      octa_to_voidp(proc->g[rG]), 
      octa, 
      instr->x
    );

    proc->S++, proc->g[rS] = octa_incr(proc->g[rS], 8);

    if(k == 0xFF) k = rB;
    else if(k == rR) k = rP;
    else if(k == rZ + 1) break;
    else k++;
  }

  proc->O = proc->S, proc->g[rO] = proc->g[rS];
  instr->x = octa_incr(proc->g[rO], -8);
  __store_x(MEX_ARGS);
}

MEXF(UNSAVE) {}
MEXF(SYNC) {}
MEXF(SWYM) {}
MEXF(GET) {}

MEXF(TRIP) 
{
  proc->g[rX] = sign_bit | instr->bin;
  proc->g[rY] = instr->yy;
  proc->g[rZ] = instr->zz;
  proc->g[rB] = proc->g[0xFF];
  proc->g[0xFF] = proc->g[rJ];
}

mmix_exec_t MMIX_DISPATCH_ROUTER[256] = {
  MEXN(TRAP), 
  MEXN(FCMP), 
  MEXN(FUN), 
  MEXN(FEQL), 
  MEXN(FADD), 
  MEXN(FIX), 
  MEXN(FSUB), 
  MEXN(FIXU), 
  MEXN(FLOT), 
  MEXN(FLOTI), 
  MEXN(FLOTU), 
  MEXN(FLOTUI), 
  MEXN(SFLOT), 
  MEXN(SFLOTI), 
  MEXN(SFLOTU), 
  MEXN(SFLOTUI), 
  MEXN(FMUL), 
  MEXN(FCMPE), 
  MEXN(FUNE), 
  MEXN(FEQLE), 
  MEXN(FDIV), 
  MEXN(FSQRT), 
  MEXN(FREM), 
  MEXN(FINT), 
  MEXN(MUL), 
  MEXN(MULI), 
  MEXN(MULU), 
  MEXN(MULUI), 
  MEXN(DIV), 
  MEXN(DIVI), 
  MEXN(DIVU), 
  MEXN(DIVUI), 
  MEXN(ADD), 
  MEXN(ADDI), 
  MEXN(ADDU), 
  MEXN(ADDUI), 
  MEXN(SUB), 
  MEXN(SUBI), 
  MEXN(SUBU), 
  MEXN(SUBUI), 
  MEXN(IIADDU), 
  MEXN(IIADDUI), 
  MEXN(IVADDU), 
  MEXN(IVADDUI), 
  MEXN(VIIIADDU), 
  MEXN(VIIIADDUI), 
  MEXN(XVIADDU), 
  MEXN(XVIADDUI), 
  MEXN(CMP), 
  MEXN(CMPI), 
  MEXN(CMPU), 
  MEXN(CMPUI), 
  MEXN(NEG), 
  MEXN(NEGI), 
  MEXN(NEGU), 
  MEXN(NEGUI), 
  MEXN(SL), 
  MEXN(SLI), 
  MEXN(SLU), 
  MEXN(SLUI), 
  MEXN(SR), 
  MEXN(SRI), 
  MEXN(SRU), 
  MEXN(SRUI), 
  MEXN(BN), 
  MEXN(BNB), 
  MEXN(BZ), 
  MEXN(BZB), 
  MEXN(BP), 
  MEXN(BPB), 
  MEXN(BOD), 
  MEXN(BODB), 
  MEXN(BNN), 
  MEXN(BNNB), 
  MEXN(BNZ), 
  MEXN(BNZB), 
  MEXN(BNP), 
  MEXN(BNPB), 
  MEXN(BEV), 
  MEXN(BEVB), 
  MEXN(PBN), 
  MEXN(PBNB), 
  MEXN(PBZ), 
  MEXN(PBZB), 
  MEXN(PBP), 
  MEXN(PBPB), 
  MEXN(PBOD), 
  MEXN(PBODB), 
  MEXN(PBNN), 
  MEXN(PBNNB), 
  MEXN(PBNZ), 
  MEXN(PBNZB), 
  MEXN(PBNP), 
  MEXN(PBNPB), 
  MEXN(PBEV), 
  MEXN(PBEVB), 
  MEXN(CSN), 
  MEXN(CSNI), 
  MEXN(CSZ), 
  MEXN(CSZI), 
  MEXN(CSP), 
  MEXN(CSPI), 
  MEXN(CSOD), 
  MEXN(CSODI), 
  MEXN(CSNN), 
  MEXN(CSNNI), 
  MEXN(CSNZ), 
  MEXN(CSNZI), 
  MEXN(CSNP), 
  MEXN(CSNPI), 
  MEXN(CSEV), 
  MEXN(CSEVI), 
  MEXN(ZSN), 
  MEXN(ZSNI), 
  MEXN(ZSZ), 
  MEXN(ZSZI), 
  MEXN(ZSP), 
  MEXN(ZSPI), 
  MEXN(ZSOD), 
  MEXN(ZSODI), 
  MEXN(ZSNN), 
  MEXN(ZSNNI), 
  MEXN(ZSNZ), 
  MEXN(ZSNZI), 
  MEXN(ZSNP), 
  MEXN(ZSNPI), 
  MEXN(ZSEV), 
  MEXN(ZSEVI), 
  MEXN(LDB), 
  MEXN(LDBI), 
  MEXN(LDBU), 
  MEXN(LDBUI), 
  MEXN(LDW), 
  MEXN(LDWI), 
  MEXN(LDWU), 
  MEXN(LDWUI), 
  MEXN(LDT), 
  MEXN(LDTI), 
  MEXN(LDTU), 
  MEXN(LDTUI), 
  MEXN(LDO), 
  MEXN(LDOI), 
  MEXN(LDOU), 
  MEXN(LDOUI), 
  MEXN(LDSF), 
  MEXN(LDSFI), 
  MEXN(LDHT), 
  MEXN(LDHTI), 
  MEXN(CSWAP), 
  MEXN(CSWAPI), 
  MEXN(LDUNC), 
  MEXN(LDUNCI), 
  MEXN(LDVTS), 
  MEXN(LDVTSI), 
  MEXN(PRELD), 
  MEXN(PRELDI), 
  MEXN(PREGO), 
  MEXN(PREGOI), 
  MEXN(GO), 
  MEXN(GOI), 
  MEXN(STB), 
  MEXN(STBI), 
  MEXN(STBU), 
  MEXN(STBUI), 
  MEXN(STW), 
  MEXN(STWI), 
  MEXN(STWU), 
  MEXN(STWUI), 
  MEXN(STT), 
  MEXN(STTI), 
  MEXN(STTU), 
  MEXN(STTUI), 
  MEXN(STO), 
  MEXN(STOI), 
  MEXN(STOU), 
  MEXN(STOUI), 
  MEXN(STSF), 
  MEXN(STSFI), 
  MEXN(STHT), 
  MEXN(STHTI), 
  MEXN(STCO), 
  MEXN(STCOI), 
  MEXN(STUNC), 
  MEXN(STUNCI), 
  MEXN(SYNCD), 
  MEXN(SYNCDI), 
  MEXN(PREST), 
  MEXN(PRESTI), 
  MEXN(SYNCID), 
  MEXN(SYNCIDI), 
  MEXN(PUSHGO), 
  MEXN(PUSHGOI), 
  MEXN(OR), 
  MEXN(ORI), 
  MEXN(ORN), 
  MEXN(ORNI), 
  MEXN(NOR), 
  MEXN(NORI), 
  MEXN(XOR), 
  MEXN(XORI), 
  MEXN(AND), 
  MEXN(ANDI), 
  MEXN(ANDN), 
  MEXN(ANDNI), 
  MEXN(NAND), 
  MEXN(NANDI), 
  MEXN(NXOR), 
  MEXN(NXORI), 
  MEXN(BDIF), 
  MEXN(BDIFI), 
  MEXN(WDIF), 
  MEXN(WDIFI), 
  MEXN(TDIF), 
  MEXN(TDIFI), 
  MEXN(ODIF), 
  MEXN(ODIFI), 
  MEXN(MUX), 
  MEXN(MUXI), 
  MEXN(SADD), 
  MEXN(SADDI), 
  MEXN(MOR), 
  MEXN(MORI), 
  MEXN(MXOR), 
  MEXN(MXORI), 
  MEXN(SETH), 
  MEXN(SETMH), 
  MEXN(SETML), 
  MEXN(SETL), 
  MEXN(INCH), 
  MEXN(INCMH), 
  MEXN(INCML), 
  MEXN(INCL), 
  MEXN(ORH), 
  MEXN(ORMH), 
  MEXN(ORML), 
  MEXN(ORL), 
  MEXN(ANDNH), 
  MEXN(ANDNMH), 
  MEXN(ANDNML), 
  MEXN(ANDNL), 
  MEXN(JMP), 
  MEXN(JMPB), 
  MEXN(PUSHJ), 
  MEXN(PUSHJB), 
  MEXN(GETA), 
  MEXN(GETAB), 
  MEXN(PUT), 
  MEXN(PUTI), 
  MEXN(POP), 
  MEXN(RESUME), 
  MEXN(SAVE), 
  MEXN(UNSAVE), 
  MEXN(SYNC), 
  MEXN(SWYM), 
  MEXN(GET), 
  MEXN(TRIP)
};

void mmix_dispatch(system_t* sys, mmix_processor_t* proc, instr_t* instr)
{
  mmix_exec_t ex = *(MMIX_DISPATCH_ROUTER + instr->op);
  ex(sys, proc, instr);
}

#endif