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

#define XX instr->xx
#define YY instr->yy
#define ZZ instr->zz

#define ZI char_to_octa(instr->zz)
#define YI char_to_octa(instr->yy)
#define YZI byte_to_octa(0, 0, 0, 0, 0, 0, YY, ZZ)
#define XYZI byte_to_octa(0, 0, 0, 0, 0, XX, YY, ZZ)

#define MMEMR(addr, type, out) MMIX_MEM_READ(sys, addr, type, out)
#define MREG(REG) proc->reg[REG]

#define EXCEPTION_MREG proc->reg[rA]
#define REMAINDER_MREG proc->reg[rR]

#define XMREG proc->reg[instr->xx]
#define YMREG proc->reg[instr->yy]

#define YLMREG ol(YMREG)
#define ZMREG proc->reg[instr->zz]

#define Z ((instr->info->flags & Z_is_immed_bit) ? ZI : ZMREG)
#define Y ((instr->info->flags & Y_is_immed_bit) ? YI : YMREG)

MEXF(TRAP) 
{
  MREG(rWW) = (octa) proc->instr_ptr;
  MREG(rXX) = sign_bit | binary_tl_op(*instr);
  MREG(rYY) = YMREG, MREG(rZZ) = ZMREG;

  if(proc->ivt[XX].hdlr != NULL)
    proc->ivt[XX].hdlr(sys, instr);
}

MEXF(FCMP) 
{
  XMREG = octa_fcmp(Y, Z, &EXCEPTION_MREG);
}

MEXF(FUN) 
{
  XMREG = octa_fun(Y, Z);
}

MEXF(FEQL) 
{
  XMREG = octa_feql(YMREG, ZMREG, &EXCEPTION_MREG); 
}

MEXF(FADD) 
{
  XMREG = octa_fadd(YMREG, ZMREG, &EXCEPTION_MREG);
}

MEXF(FIX) 
{
  XMREG = octa_fix(ZMREG, &EXCEPTION_MREG);
}

MEXF(FSUB) 
{
  XMREG = octa_fsub(YMREG, ZMREG, &EXCEPTION_MREG);
}

MEXF(FIXU) 
{
  XMREG = octa_fixu(YMREG, &EXCEPTION_MREG);
}

static inline void __flot(MEX_DEF_ARGS) {
  XMREG = octa_flot(Z, proc->rounding_mode, &EXCEPTION_MREG);
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
  XMREG = octa_fmult(YMREG, ZMREG, &EXCEPTION_MREG);
}

MEXF(FCMPE) {}
MEXF(FUNE) {}
MEXF(FEQLE) {}

MEXF(FDIV) 
{
  XMREG = octa_fdiv(YMREG, ZMREG, &EXCEPTION_MREG);
}

MEXF(FSQRT) 
{
  XMREG = octa_fsqrt(ZMREG, YLMREG, &EXCEPTION_MREG);
}

MEXF(FREM) {
  XMREG = octa_frem(YMREG, ZMREG, &EXCEPTION_MREG);
}

MEXF(FINT) {
  XMREG = octa_fint(ZMREG, proc->rounding_mode, &EXCEPTION_MREG);
}

static inline void __signed_mul(MEX_DEF_ARGS)
{
  bool overflow; 
  octa aux;

  XMREG = octa_signed_mult(Y, Z, &aux, &overflow);

  if(overflow)
    EXCEPTION_MREG |= V_BIT;
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

  XMREG = octa_mult(Y, Z, &aux, &overflow);

  if(overflow)
    EXCEPTION_MREG |= V_BIT;

  MREG(rH) = aux;
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
  hexadeca yy = {Y & sign_bit, Y};
  
  XMREG = octa_signed_div(yy, Z, &aux);
  REMAINDER_MREG = aux;
}

MEXF(DIV) {
  __signed_div(MEX_ARGS);
}

MEXF(DIVI) {
  __signed_div(MEX_ARGS);
}

static inline void __unsigned_div(MEX_DEF_ARGS)
{
  hexadeca yy = {MREG(rD), Y};
  XMREG = octa_div(yy, Z, &REMAINDER_MREG);
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

  XMREG = octa_plus(Y, Z, &overflow);
  
  if(overflow && test_overflow)
    EXCEPTION_MREG |= V_BIT;
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

  XMREG = octa_minus(Y, Z, &overflow);
  
  if(test_overflow && overflow) 
    EXCEPTION_MREG |= V_BIT; 
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
  XMREG = octa_plus(
    octa_plus(Y, Y, &overflow), 
    Z, &overflow
  );
}

static inline void __unsigned_4add(MEX_DEF_ARGS) {
  bool overflow;
  XMREG = octa_plus(
    octa_plus(
      octa_plus(Y, Y, &overflow), 
      octa_plus(Y, Y, &overflow),
      &overflow
    ), Z, &overflow
  );
}

static inline void __unsigned_8add(MEX_DEF_ARGS) {
  bool overflow;

  XMREG = octa_plus(
    octa_plus(
      octa_plus(
        octa_plus(Y, Y, &overflow), 
        octa_plus(Y, Y, &overflow),
        &overflow
      ), 
      octa_plus(
        octa_plus(Y, Y, &overflow), 
        octa_plus(Y, Y, &overflow),
        &overflow
      ), &overflow
    ), Z, &overflow
  );
}

static inline void __unsigned_16add(MEX_DEF_ARGS) {
  bool overflow;

  XMREG = octa_plus(
    octa_plus(
      octa_plus(
        octa_plus(
          octa_plus(Y, Y, &overflow), 
          octa_plus(Y, Y, &overflow), 
          &overflow), 
        octa_plus(
          octa_plus(Y, Y, &overflow), 
          octa_plus(Y, Y, &overflow), 
          &overflow),
        &overflow
      ), 
      octa_plus(
        octa_plus(
          octa_plus(Y, Y, &overflow), 
          octa_plus(Y, Y, &overflow), 
          &overflow), 
        octa_plus(
          octa_plus(Y, Y, &overflow), 
          octa_plus(Y, Y, &overflow), 
          &overflow),
        &overflow
      ), &overflow
    ), Z, &overflow
  );
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

static inline void __cmp(MEX_DEF_ARGS) {
  XMREG = octa_cmp(Y, Z);
}

MEXF(CMP) {
  __cmp(sys, proc, instr);
}
MEXF(CMPI) {
  __cmp(sys, proc, instr);
}
MEXF(CMPU) {
  __cmp(sys, proc, instr);
}
MEXF(CMPUI) {
  __cmp(sys, proc, instr);
}

static inline void __neg(MEX_DEF_ARGS, bool test_overflow) {
  bool overflow = false;
  XMREG = octa_minus(Y, Z, &overflow);

  if(test_overflow && overflow)
    EXCEPTION_MREG |= V_BIT;
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

MEXF(SL) {}
MEXF(SLI) {}
MEXF(SLU) {}
MEXF(SLUI) {}
MEXF(SR) {}
MEXF(SRI) {}
MEXF(SRU) {}
MEXF(SRUI) {}
MEXF(BN) {}
MEXF(BNB) {}
MEXF(BZ) {}
MEXF(BZB) {}
MEXF(BP) {}
MEXF(BPB) {}
MEXF(BOD) {}
MEXF(BODB) {}
MEXF(BNN) {}
MEXF(BNNB) {}
MEXF(BNZ) {}
MEXF(BNZB) {}
MEXF(BNP) {}
MEXF(BNPB) {}
MEXF(BEV) {}
MEXF(BEVB) {}
MEXF(PBN) {}
MEXF(PBNB) {}
MEXF(PBZ) {}
MEXF(PBZB) {}
MEXF(PBP) {}
MEXF(PBPB) {}
MEXF(PBOD) {}
MEXF(PBODB) {}
MEXF(PBNN) {}
MEXF(PBNNB) {}
MEXF(PBNZ) {}
MEXF(PBNZB) {}
MEXF(PBNP) {}
MEXF(PBNPB) {}
MEXF(PBEV) {}
MEXF(PBEVB) {}
MEXF(CSN) {}
MEXF(CSNI) {}
MEXF(CSZ) {}
MEXF(CSZI) {}
MEXF(CSP) {}
MEXF(CSPI) {}
MEXF(CSOD) {}
MEXF(CSODI) {}
MEXF(CSNN) {}
MEXF(CSNNI) {}
MEXF(CSNZ) {}
MEXF(CSNZI) {}
MEXF(CSNP) {}
MEXF(CSNPI) {}
MEXF(CSEV) {}
MEXF(CSEVI) {}
MEXF(ZSN) {}
MEXF(ZSNI) {}
MEXF(ZSZ) {}
MEXF(ZSZI) {}
MEXF(ZSP) {}
MEXF(ZSPI) {}
MEXF(ZSOD) {}
MEXF(ZSODI) {}
MEXF(ZSNN) {}
MEXF(ZSNNI) {}
MEXF(ZSNZ) {}
MEXF(ZSNZI) {}
MEXF(ZSNP) {}
MEXF(ZSNPI) {}
MEXF(ZSEV) {}
MEXF(ZSEVI) {}

static inline void __ldb(MEX_DEF_ARGS) 
{
  byte out;  
  MMEMR((void*)(Y + Z), byte, out);
  XMREG = tetra_to_octa(0, out);
}

MEXF(LDB) 
{
  __ldb(MEX_ARGS);
}
MEXF(LDBI) 
{
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
  MMEMR((void*)(Y + Z), word, out);
  XMREG = tetra_to_octa(0, out);
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
  MMEMR((void*)(Y + Z), tetra, out);
  XMREG = tetra_to_octa(0, out);
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
  octa out;  
  MMEMR((void*)(Y + Z), octa, out);
  XMREG = out;
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

MEXF(LDSF) {}
MEXF(LDSFI) {}
MEXF(LDHT) {}
MEXF(LDHTI) {}
MEXF(CSWAP) {}
MEXF(CSWAPI) {}
MEXF(LDUNC) {}
MEXF(LDUNCI) {}
MEXF(LDVTS) {}
MEXF(LDVTSI) {}
MEXF(PRELD) {}
MEXF(PRELDI) {}
MEXF(PREGO) {}
MEXF(PREGOI) {}
MEXF(GO) {}
MEXF(GOI) {}
MEXF(STB) {}
MEXF(STBI) {}
MEXF(STBU) {}
MEXF(STBUI) {}
MEXF(STW) {}
MEXF(STWI) {}
MEXF(STWU) {}
MEXF(STWUI) {}
MEXF(STT) {}
MEXF(STTI) {}
MEXF(STTU) {}
MEXF(STTUI) {}
MEXF(STO) {}
MEXF(STOI) {}
MEXF(STOU) {}
MEXF(STOUI) {}
MEXF(STSF) {}
MEXF(STSFI) {}
MEXF(STHT) {}
MEXF(STHTI) {}
MEXF(STCO) {}
MEXF(STCOI) {}
MEXF(STUNC) {}
MEXF(STUNCI) {}
MEXF(SYNCD) {}
MEXF(SYNCDI) {}
MEXF(PREST) {}
MEXF(PRESTI) {}
MEXF(SYNCID) {}
MEXF(SYNCIDI) {}
MEXF(PUSHGO) {}
MEXF(PUSHGOI) {}

static inline void __or(MEX_DEF_ARGS) {
  XMREG = octa_or(Y, Z);
}

MEXF(OR) {
  __or(MEX_ARGS);
}
MEXF(ORI) {
  __or(MEX_ARGS);
}

static inline void __orn(MEX_DEF_ARGS) {
  XMREG = octa_orn(Y, Z);
}

MEXF(ORN) {
  __orn(MEX_ARGS);
}
MEXF(ORNI) {
  __orn(MEX_ARGS);
}

static inline void __nor(MEX_DEF_ARGS) {
  XMREG = octa_orn(Y, Z);
}

MEXF(NOR) {
  __nor(MEX_ARGS);
}

MEXF(NORI) {
  __nor(MEX_ARGS);
}

static inline void __xor(MEX_DEF_ARGS) {
  XMREG = octa_xor(Y, Z);
}

MEXF(XOR) {
  __xor(MEX_ARGS);
}
MEXF(XORI) {
  __xor(MEX_ARGS);
}

static inline void __and(MEX_DEF_ARGS) {
  XMREG = octa_and(Y, Z);
}

MEXF(AND) {
  __and(MEX_ARGS);
}
MEXF(ANDI) {
  __and(MEX_ARGS);
}

static inline void __andn(MEX_DEF_ARGS) {
  XMREG = octa_andn(Y, Z);
}

MEXF(ANDN) {
  __andn(MEX_ARGS);
}
MEXF(ANDNI) {
  __andn(MEX_ARGS);
}

static inline void __nand(MEX_DEF_ARGS) {
  XMREG = octa_nand(Y, Z);
}

MEXF(NAND) {
  __nand(MEX_ARGS);
}
MEXF(NANDI) {
  __nand(MEX_ARGS);
}

static inline void __nxor(MEX_DEF_ARGS) {
  XMREG = octa_nxor(Y, Z);
}

MEXF(NXOR) {
  __nxor(MEX_ARGS);
}
MEXF(NXORI) {
  __nxor(MEX_ARGS);
}

static inline void __bdif(MEX_DEF_ARGS) {
  XMREG = octa_bdif(Y, Z);
}

MEXF(BDIF) {
  __bdif(MEX_ARGS);
}
MEXF(BDIFI) {
  __bdif(MEX_ARGS);
}

static inline void __wdif(MEX_DEF_ARGS) {
  XMREG = octa_wdif(Y, Z);
}

MEXF(WDIF) {
  __wdif(MEX_ARGS);  
}
MEXF(WDIFI) {
  __wdif(MEX_ARGS);
}

static inline void __tdif(MEX_DEF_ARGS) {
  XMREG = octa_tdif(Y, Z);
}

MEXF(TDIF) {
  __tdif(MEX_ARGS);
}
MEXF(TDIFI) {
  __tdif(MEX_ARGS);
}

static inline void __odif(MEX_DEF_ARGS) {
  XMREG = octa_odif(Y, Z);
}

MEXF(ODIF) {
  __odif(MEX_ARGS);
}
MEXF(ODIFI) {
  __odif(MEX_ARGS);
}

MEXF(MUX) {}
MEXF(MUXI) {}
MEXF(SADD) {}
MEXF(SADDI) {}
MEXF(MOR) {}
MEXF(MORI) {}
MEXF(MXOR) {}
MEXF(MXORI) {}
MEXF(SETH) {}
MEXF(SETMH) {}
MEXF(SETML) {}
MEXF(SETL) {}
MEXF(INCH) {}
MEXF(INCMH) {}
MEXF(INCML) {}
MEXF(INCL) {}
MEXF(ORH) {}
MEXF(ORMH) {}
MEXF(ORML) {}
MEXF(ORL) {}
MEXF(ANDNH) {}
MEXF(ANDNMH) {}
MEXF(ANDNML) {}
MEXF(ANDNL) {}
MEXF(JMP) {}
MEXF(JMPB) {}
MEXF(PUSHJ) {}
MEXF(PUSHJB) {}
MEXF(GETA) {}
MEXF(GETAB) {}
MEXF(PUT) {}
MEXF(PUTI) {}
MEXF(POP) {}

MEXF(RESUME) 
{
  octa b, z;
  z = proc->reg[instr->zz];
  proc->instr_ptr = (octa*) proc->reg[rW];
  z = proc->reg[rW];
  b = proc->reg[rX];
}

MEXF(SAVE) {}

MEXF(UNSAVE) 
{

}

MEXF(SYNC) {}
MEXF(SWYM) {}
MEXF(GET) {}
MEXF(TRIP) 
{
  proc->reg[rX] = sign_bit | instr->bin;
  proc->reg[rY] = instr->yy;
  proc->reg[rZ] = instr->zz;
  proc->reg[rB] = proc->reg[0xFF];
  proc->reg[0xFF] = proc->reg[rJ];
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