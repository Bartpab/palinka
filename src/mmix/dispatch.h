#ifndef __MMIX_DISPATCH_H__
#define __MMIX_DISPATCH_H__

#include <math.h>

#include "instr.h"
#include "processor.h"

#include "../arith.h"
#include "../sys.h"
#include "../types.h"

typedef void (*mmix_exec_t)(system_t* sys, mmix_processor_t* proc, instr_t* instr);
#define MEXS(op_name) void mmix_ ##op_name (system_t * sys, mmix_processor_t* proc,instr_t * instr)
#define MEXN(op_name) mmix_##op_name

#define XX instr->xx
#define YY instr->yy
#define ZZ instr->zz

#define ZI char_to_octa(instr->zz)
#define YI char_to_octa(instr->yy)
#define YZI byte_to_octa(0, 0, 0, 0, 0, 0, YY, ZZ)
#define XYZI byte_to_octa(0, 0, 0, 0, 0, XX, YY, ZZ)

#define MREG(REG) proc->reg[REG]

#define EXCEPTION_MREG proc->reg[rA]
#define REMAINDER_MREG proc->reg[rR]

#define XMREG proc->reg[instr->xx]
#define YMREG proc->reg[instr->yy]

#define YLMREG ol(YMREG)
#define ZMREG proc->reg[instr->zz]

#define Z ((instr->info->flags & Z_is_immed_bit) ? ZI : ZMREG)
#define Y ((instr->info->flags & Y_is_immed_bit) ? YI : YMREG)

MEXS(TRAP) 
{
  MREG(rWW) = (octa) proc->instr_ptr;
  MREG(rXX) = sign_bit | binary_tl_op(*instr);
  MREG(rYY) = YMREG, MREG(rZZ) = ZMREG;

  if(proc->ivt[XX].hdlr != NULL)
    proc->ivt[XX].hdlr(sys, instr);
}

MEXS(FCMP) 
{
  XMREG = octa_fcmp(Y, Z, &EXCEPTION_MREG);
}

MEXS(FUN) 
{
  XMREG = octa_fun(Y, Z);
}

MEXS(FEQL) 
{
  XMREG = octa_feql(YMREG, ZMREG, &EXCEPTION_MREG); 
}

MEXS(FADD) 
{
  XMREG = octa_fadd(YMREG, ZMREG, &EXCEPTION_MREG);
}

MEXS(FIX) 
{
  XMREG = octa_fix(ZMREG, &EXCEPTION_MREG);
}

MEXS(FSUB) 
{
  XMREG = octa_fsub(YMREG, ZMREG, &EXCEPTION_MREG);
}

MEXS(FIXU) 
{
  XMREG = octa_fixu(YMREG, &EXCEPTION_MREG);
}

MEXS(FLOT) 
{
  XMREG = octa_flot(ZMREG, proc->rounding_mode, &EXCEPTION_MREG);
}

MEXS(FLOTI) 
{
  
}

MEXS(FLOTU) {}
MEXS(FLOTUI) {}
MEXS(SFLOT) {}
MEXS(SFLOTI) {}
MEXS(SFLOTU) {}
MEXS(SFLOTUI) {}

MEXS(FMUL) 
{
  XMREG = octa_fmult(YMREG, ZMREG, &EXCEPTION_MREG);
}

MEXS(FCMPE) {}
MEXS(FUNE) {}
MEXS(FEQLE) {}

MEXS(FDIV) 
{
  XMREG = octa_fdiv(YMREG, ZMREG, &EXCEPTION_MREG);
}

MEXS(FSQRT) 
{
  XMREG = octa_fsqrt(ZMREG, YLMREG, &EXCEPTION_MREG);
}

MEXS(FREM) {
  XMREG = octa_frem(YMREG, ZMREG, &EXCEPTION_MREG);
}

MEXS(FINT) {
  XMREG = octa_fint(ZMREG, proc->rounding_mode, &EXCEPTION_MREG);
}

static inline void __signed_mul(system_t* sys, mmix_processor_t* proc, instr_t* instr)
{
  bool overflow; 
  octa aux;

  XMREG = octa_signed_mult(Y, Z, &aux, &overflow);

  if(overflow)
    EXCEPTION_MREG |= V_BIT;
}

MEXS(MUL) 
{
  __signed_mul(sys, proc, instr);
}

MEXS(MULI) 
{
  __signed_mul(sys, proc, instr);
}

static inline void __unsigned_mul(system_t* sys, mmix_processor_t* proc, instr_t* instr)
{
  bool overflow;
  octa aux;

  XMREG = octa_mult(Y, Z, &aux, &overflow);

  if(overflow)
    EXCEPTION_MREG |= V_BIT;

  MREG(rH) = aux;
}

MEXS(MULU) {
  __unsigned_mul(sys, proc, instr);
}

MEXS(MULUI) {
  __unsigned_mul(sys, proc, instr);
}

static inline void __signed_div(system_t* sys, mmix_processor_t* proc, instr_t* instr)
{
  octa aux;
  hexadeca yy = {Y & sign_bit, Y};
  
  XMREG = octa_signed_div(yy, Z, &aux);
  REMAINDER_MREG = aux;
}

MEXS(DIV) {
  __signed_div(sys, proc, instr);
}

MEXS(DIVI) {
  __signed_div(sys, proc, instr);
}

static inline void __unsigned_div(system_t* sys, mmix_processor_t* proc, instr_t* instr)
{
  octa aux = 0;
  hexadeca yy = {MREG(rD), Y};

  XMREG = octa_div(yy, Z, &aux);
  REMAINDER_MREG = aux;
}

MEXS(DIVU) 
{
  __unsigned_div(sys, proc, instr);
}

MEXS(DIVUI) 
{
  __unsigned_div(sys, proc, instr);
}

static inline void __add(system_t* sys, mmix_processor_t* proc, instr_t* instr, bool test_overflow) 
{
  bool overflow = false;

  XMREG = octa_plus(Y, Z, &overflow);
  
  if(overflow && test_overflow)
    EXCEPTION_MREG |= V_BIT;
}

MEXS(ADD) {
  __add(sys, proc, instr, true);
}
MEXS(ADDI) {
  __add(sys, proc, instr, true);
}
MEXS(ADDU) {
  __add(sys, proc, instr, true);
}
MEXS(ADDUI) {
  __add(sys, proc, instr, true);
}

static inline void __sub(system_t* sys, mmix_processor_t* proc, instr_t* instr, bool test_overflow) {
  bool overflow = false;

  XMREG = octa_minus(Y, Z, &overflow);
  
  if(test_overflow && overflow) 
    EXCEPTION_MREG |= V_BIT; 
}

MEXS(SUB) {
  __sub(sys, proc, instr, true);
}
MEXS(SUBI) {
  __sub(sys, proc, instr, true);
}
MEXS(SUBU) {
  __sub(sys, proc, instr, false);
}
MEXS(SUBUI) {
  __sub(sys, proc, instr, false);
}

static inline void __unsigned_2add(system_t* sys, mmix_processor_t* proc, instr_t* instr){
  bool overflow;
  XMREG = octa_plus(
    octa_plus(Y, Y, &overflow), 
    Z, &overflow
  );
}

static inline void __unsigned_4add(system_t* sys, mmix_processor_t* proc, instr_t* instr) {
  bool overflow;
  XMREG = octa_plus(
    octa_plus(
      octa_plus(Y, Y, &overflow), 
      octa_plus(Y, Y, &overflow),
      &overflow
    ), Z, &overflow
  );
}

static inline void __unsigned_8add(system_t* sys, mmix_processor_t* proc, instr_t* instr) {
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

static inline void __unsigned_16add(system_t* sys, mmix_processor_t* proc, instr_t* instr) {
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

MEXS(IIADDU) {
  __unsigned_2add(sys, proc, instr);
}
MEXS(IIADDUI) {
  __unsigned_2add(sys, proc, instr);
}
MEXS(IVADDU) {
  __unsigned_4add(sys, proc, instr);
}
MEXS(IVADDUI) {
  __unsigned_4add(sys, proc, instr);
}
MEXS(VIIIADDU) {
  __unsigned_8add(sys, proc, instr);
}
MEXS(VIIIADDUI) {
  __unsigned_8add(sys, proc, instr);
}
MEXS(XVIADDU) {
  __unsigned_16add(sys, proc, instr);
}
MEXS(XVIADDUI) {
  __unsigned_16add(sys, proc, instr);
}
MEXS(CMP) {}
MEXS(CMPI) {}
MEXS(CMPU) {}
MEXS(CMPUI) {}
MEXS(NEG) {
}
MEXS(NEGI) {
}
MEXS(NEGU) {}
MEXS(NEGUI) {}
MEXS(SL) {}
MEXS(SLI) {}
MEXS(SLU) {}
MEXS(SLUI) {}
MEXS(SR) {}
MEXS(SRI) {}
MEXS(SRU) {}
MEXS(SRUI) {}
MEXS(BN) {}
MEXS(BNB) {}
MEXS(BZ) {}
MEXS(BZB) {}
MEXS(BP) {}
MEXS(BPB) {}
MEXS(BOD) {}
MEXS(BODB) {}
MEXS(BNN) {}
MEXS(BNNB) {}
MEXS(BNZ) {}
MEXS(BNZB) {}
MEXS(BNP) {}
MEXS(BNPB) {}
MEXS(BEV) {}
MEXS(BEVB) {}
MEXS(PBN) {}
MEXS(PBNB) {}
MEXS(PBZ) {}
MEXS(PBZB) {}
MEXS(PBP) {}
MEXS(PBPB) {}
MEXS(PBOD) {}
MEXS(PBODB) {}
MEXS(PBNN) {}
MEXS(PBNNB) {}
MEXS(PBNZ) {}
MEXS(PBNZB) {}
MEXS(PBNP) {}
MEXS(PBNPB) {}
MEXS(PBEV) {}
MEXS(PBEVB) {}
MEXS(CSN) {}
MEXS(CSNI) {}
MEXS(CSZ) {}
MEXS(CSZI) {}
MEXS(CSP) {}
MEXS(CSPI) {}
MEXS(CSOD) {}
MEXS(CSODI) {}
MEXS(CSNN) {}
MEXS(CSNNI) {}
MEXS(CSNZ) {}
MEXS(CSNZI) {}
MEXS(CSNP) {}
MEXS(CSNPI) {}
MEXS(CSEV) {}
MEXS(CSEVI) {}
MEXS(ZSN) {}
MEXS(ZSNI) {}
MEXS(ZSZ) {}
MEXS(ZSZI) {}
MEXS(ZSP) {}
MEXS(ZSPI) {}
MEXS(ZSOD) {}
MEXS(ZSODI) {}
MEXS(ZSNN) {}
MEXS(ZSNNI) {}
MEXS(ZSNZ) {}
MEXS(ZSNZI) {}
MEXS(ZSNP) {}
MEXS(ZSNPI) {}
MEXS(ZSEV) {}
MEXS(ZSEVI) {}
MEXS(LDB) {}
MEXS(LDBI) {}
MEXS(LDBU) {}
MEXS(LDBUI) {}
MEXS(LDW) {}
MEXS(LDWI) {}
MEXS(LDWU) {}
MEXS(LDWUI) {}
MEXS(LDT) {}
MEXS(LDTI) {}
MEXS(LDTU) {}
MEXS(LDTUI) {}
MEXS(LDO) {}
MEXS(LDOI) {}
MEXS(LDOU) {}
MEXS(LDOUI) {}
MEXS(LDSF) {}
MEXS(LDSFI) {}
MEXS(LDHT) {}
MEXS(LDHTI) {}
MEXS(CSWAP) {}
MEXS(CSWAPI) {}
MEXS(LDUNC) {}
MEXS(LDUNCI) {}
MEXS(LDVTS) {}
MEXS(LDVTSI) {}
MEXS(PRELD) {}
MEXS(PRELDI) {}
MEXS(PREGO) {}
MEXS(PREGOI) {}
MEXS(GO) {}
MEXS(GOI) {}
MEXS(STB) {}
MEXS(STBI) {}
MEXS(STBU) {}
MEXS(STBUI) {}
MEXS(STW) {}
MEXS(STWI) {}
MEXS(STWU) {}
MEXS(STWUI) {}
MEXS(STT) {}
MEXS(STTI) {}
MEXS(STTU) {}
MEXS(STTUI) {}
MEXS(STO) {}
MEXS(STOI) {}
MEXS(STOU) {}
MEXS(STOUI) {}
MEXS(STSF) {}
MEXS(STSFI) {}
MEXS(STHT) {}
MEXS(STHTI) {}
MEXS(STCO) {}
MEXS(STCOI) {}
MEXS(STUNC) {}
MEXS(STUNCI) {}
MEXS(SYNCD) {}
MEXS(SYNCDI) {}
MEXS(PREST) {}
MEXS(PRESTI) {}
MEXS(SYNCID) {}
MEXS(SYNCIDI) {}
MEXS(PUSHGO) {}
MEXS(PUSHGOI) {}
MEXS(OR) {}
MEXS(ORI) {}
MEXS(ORN) {}
MEXS(ORNI) {}
MEXS(NOR) {}
MEXS(NORI) {}
MEXS(XOR) {}
MEXS(XORI) {}
MEXS(AND) {}
MEXS(ANDI) {}
MEXS(ANDN) {}
MEXS(ANDNI) {}
MEXS(NAND) {}
MEXS(NANDI) {}
MEXS(NXOR) {}
MEXS(NXORI) {}
MEXS(BDIF) {}
MEXS(BDIFI) {}
MEXS(WDIF) {}
MEXS(WDIFI) {}
MEXS(TDIF) {}
MEXS(TDIFI) {}
MEXS(ODIF) {}
MEXS(ODIFI) {}
MEXS(MUX) {}
MEXS(MUXI) {}
MEXS(SADD) {}
MEXS(SADDI) {}
MEXS(MOR) {}
MEXS(MORI) {}
MEXS(MXOR) {}
MEXS(MXORI) {}
MEXS(SETH) {}
MEXS(SETMH) {}
MEXS(SETML) {}
MEXS(SETL) {}
MEXS(INCH) {}
MEXS(INCMH) {}
MEXS(INCML) {}
MEXS(INCL) {}
MEXS(ORH) {}
MEXS(ORMH) {}
MEXS(ORML) {}
MEXS(ORL) {}
MEXS(ANDNH) {}
MEXS(ANDNMH) {}
MEXS(ANDNML) {}
MEXS(ANDNL) {}
MEXS(JMP) {}
MEXS(JMPB) {}
MEXS(PUSHJ) {}
MEXS(PUSHJB) {}
MEXS(GETA) {}
MEXS(GETAB) {}
MEXS(PUT) {}
MEXS(PUTI) {}
MEXS(POP) {}

MEXS(RESUME) 
{
  octa b, z;
  z = proc->reg[instr->zz];
  proc->instr_ptr = (octa*) proc->reg[rW];
  z = proc->reg[rW];
  b = proc->reg[rX];
}

MEXS(SAVE) {}

MEXS(UNSAVE) 
{

}

MEXS(SYNC) {}
MEXS(SWYM) {}
MEXS(GET) {}
MEXS(TRIP) 
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