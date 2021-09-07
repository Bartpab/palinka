#ifndef __ARITH_H__
#define __ARITH_H__

#include "types.h"
#include <math.h>
#include <assert.h>

#define sign_bit 0x8000000000000000

#define X_BIT (1 << 8)  // Floating inexact
#define Z_BIT (1 << 9)  // Floating division by zero
#define U_BIT (1 << 10) // Floating underflow
#define O_BIT (1 << 11) // Floating overflow
#define I_BIT (1 << 12) // Floating invalid operation
#define W_BIT (1 << 13) // Floating-to-fix overflow
#define V_BIT (1 << 14) // Integer overflow
#define D_BIT (1 << 15) // Integer divide check
#define H_BIT (< << 16) // Trip
#define E_BIT (1 << 18) // External dynamic trap bit

#define UINT64_MAX_VALUE 18446744073709551615U
#define INT64_MAX_VALUE 9223372036854775807

#define ROUND_OFF 1
#define ROUND_UP 2
#define ROUND_DOWN 3
#define ROUND_NEAR 4

#define oh(o) (o >> 32)
#define ol(o) (o & 0xFFFFFFFF)

void hexadeca_str(hexadeca x, char* buf, size_t len);
void octa_str(octa x, char* buf, size_t len);

word byte_to_word(byte b2, byte b1);
octa byte_to_octa(byte b7, byte b6, byte b5, byte b4, byte b3, byte b2, byte b1, byte b0);
octa word_to_octa(word hh, word hl, word lh, word ll);
octa tetra_to_octa(tetra h, tetra l);

octa char_to_octa(char x);
octa uint_to_octa(unsigned int x);
octa int_to_octa(int x);
octa double_to_octa(double x);
double octa_to_double(octa x);

octa octa_and(octa x, octa y);
octa octa_or(octa x, octa y);
octa octa_xor(octa x, octa y);
octa octa_andn(octa x, octa y);
octa octa_orn(octa x, octa y);
octa octa_nand(octa x, octa y);
octa octa_nor(octa x, octa y);
octa octa_nxor(octa x, octa y);

octa octa_bdif(octa x, octa y);
octa octa_wdif(octa x, octa y);
octa octa_tdif(octa x, octa y);
octa octa_odif(octa x, octa y);

octa octa_plus(octa y, octa z, bool* overflow);
octa octa_minus(octa y, octa z, bool *overflow);

bool octa_is_neg(octa x);
octa octa_compl(octa x);
octa octa_incr(octa y, int d);

octa octa_right_shift(octa y, int s, int u);
octa octa_left_shift(octa y, int s);

/* IMPL */
void hexadeca_str(hexadeca x, char* buf, size_t len)
{
  snprintf(buf, len, "%#lx%064lx", x.h, x.l);
}

void octa_str(octa x, char* buf, size_t len)
{
  snprintf(buf, len, "%#lx", x);
}

word byte_to_word(byte b2, byte b1)
{
  return ((word)(b2) << 8) | b1;
}

octa byte_to_octa(byte b7, byte b6, byte b5, byte b4, byte b3, byte b2, byte b1, byte b0) {
  return word_to_octa(
    byte_to_word(b7, b6), 
    byte_to_word(b5, b4), 
    byte_to_word(b3, b2), 
    byte_to_word(b1, b0)
  );
}

octa word_to_octa(word hh, word hl, word lh, word ll) 
{
  return ((octa)(hh) << 48) + ((octa)(hl) << 32) + ((octa)(lh) << 16) + ll;
}

octa tetra_to_octa(tetra h, tetra l) {
  return ((octa)(h) << 32) + l;
}

octa char_to_octa(char x) {
  return x;
}

octa uint_to_octa(unsigned x) {
  return x;
}
octa int_to_octa(int x) {
  return x;
}

octa double_to_octa(double x) {
  octa d;
  d = (*(octa*)&x);
  return d;
}

double octa_to_double(octa x) {
  double d;
  d = (*(double*)&x);
  return d;
}

octa octa_and(octa x, octa y) {
  return x & y;
}

octa octa_or(octa x, octa y) {
  return x | y;
}

octa octa_xor(octa x, octa y) {
  return x^y;
}

octa octa_andn(octa x, octa y) {
  return x & (~y);
}

octa octa_orn(octa x, octa y) {
  return x & (~y);
}

octa octa_nand(octa x, octa y) {
  return ~(x & y);
}

octa octa_nor(octa x, octa y) {
  return ~(x | y);
}

octa octa_nxor(octa x, octa y) {
  return ~(x ^ y);
}

octa octa_bdif(octa x, octa y) {
  octa tmp = 0;
  
  for (char i = 0; i < 8; i++) 
  {
    byte bx = x >> (i * 8);
    byte by = y >> (i * 8);

    byte bd = bx - by;
    
    bd = bd & 0x80 ? 0 : bd;

    tmp |= bd << (i * 8);
  }

  return tmp;
}

octa octa_wdif(octa x, octa y) {
  octa tmp = 0;
  
  for (char i = 0; i < 4; i++) 
  {
    word wx = (x >> (i * 16)) & 0xFFFF;
    word wy = (y >> (i * 16)) & 0xFFFF;

    word wd = wx - wy;
    
    wd = wd & 0x8000 ? 0 : wd;

    tmp |= (wd << (i * 16));
  }

  return tmp;
}

octa octa_tdif(octa x, octa y) {
  octa tmp = 0;
  
  for (char i = 0; i < 2; i++) 
  {
    tetra tx = (x >> (i * 32)) & 0xFFFFFFFF;
    tetra ty = (y >> (i * 32)) & 0xFFFFFFFF;

    tetra td = tx - ty;
    
    td = td & 0x80000000 ? 0 : td;

    tmp |= ((octa)(td) << (i * 32));
  }

  return tmp;
}

octa octa_odif(octa x, octa y) {
  octa tmp;
  bool overflow;
  tmp = octa_minus(x, y, &overflow);
  tmp = tmp & sign_bit ? 0 : tmp;
  return tmp;
}

octa octa_plus(octa y, octa z, bool* overflow)
{
  octa x = y + z;
  
  if(((y^z) & sign_bit) == 0 && ((y^x) & sign_bit) != 0)
    *overflow = true;

  return x;
}

octa octa_minus(octa y, octa z, bool *overflow)
{
  octa x = y - z;

  if(((x^z) & sign_bit) == 0 && ((y^x) & sign_bit) != 0)
    *overflow = true;

  return y - z;
}

bool octa_is_neg(octa x) {
  return x & sign_bit;
}

octa octa_compl(octa x) {
  return ~x + 1;
}

octa octa_incr(octa y, int d) 
{
  return y + d;
}

octa octa_right_shift(octa y, int s, int u)
{
  return y >> s;
}

octa octa_left_shift(octa y, int s)
{
  return y << s;
}

octa octa_mult(octa y, octa z, octa* aux, bool* overflow)
{
  octa yl = y & 0xFFFFFFFF;
  octa yh = y >> 32;

  octa zl = z & 0xFFFFFFFF;
  octa zh = z >> 32;

  /*                
              yh yl
      X       zh zl
      -------------
                  00  LOW PART
      -------------
              00
            10 10     MIDDLE PART
      +       01
      -------------
            01 
      + 11 11        HIGH PART
      -------------
  */

  octa phh = yh * zh, plh = yl * zh;
  octa phl = yh * zl, pll = yl * zl;

  /*
      Proof that 64-bit products can accumulate two more 32-bit values
      without overflowing:

      Max 32-bit value is 2^32 - 1.
      PSum = (2^32-1) * (2^32-1) + (2^32-1) + (2^32-1)
            = 2^64 - 2^32 - 2^32 + 1 + 2^32 - 1 + 2^32 - 1
            = 2^64 - 1
      Therefore it cannot overflow regardless of input.
  */
  octa pm = phl + (pll >> 32) + (plh & 0xFFFFFFFF);
  octa ph = phh + (pm >> 32) + (plh >> 32);
  
  *aux = ph;
  
  if(ph > 0)
    *overflow = true;

  return (pm << 32) | (pll & 0xFFFFFFFF); 
}

octa octa_signed_mult(octa y, octa z, octa* aux, bool* overflow)
{
  octa acc = octa_mult(y, z, aux, overflow);

  if(y & sign_bit) *aux = octa_minus(*aux, z, overflow);
  if(z & sign_bit) *aux = octa_minus(*aux, y, overflow);

  *overflow = (oh(*aux) != ol(*aux)) || (oh(*aux) ^ (oh(*aux) >> 1) ^ (acc & sign_bit));
  
  return acc;
}

/*
* \brief Perform 128-bit long division
* (x*2^64 + y) = q*z + r
*/
octa octa_div(hexadeca x, octa z, octa* aux)
{
  octa q = 0;
  octa rem = 0;
  
  char s = 127;
  char rs = -1;
  while(s >= 0) 
  {
    rs = -1;
    // Shift divisor until rem is bigger
    if(rem < z) 
    {
      rem <<= 1;
      rs++;

      if(s > 31)
        rem |= (x.h >> (s - 64)) & 0b1;
      else
        rem |= (x.l >> s) & 0b1;
    } 
    
    if(rem >= z) 
    {
      rem -= z;
      q |= (0b1 << s);
    }
    s--;
  }

  *aux = rem;
  return q;
}

octa octa_signed_div(hexadeca x, octa z, octa* aux)
{
  char sign = 1;
  
  if(x.h & sign_bit)
  {
    x.h &= (~sign_bit);
    sign *= -1;
  }
  
  if(z & sign_bit)
  {
    z &= (~sign_bit);
    sign *= -1;
  }

  octa q = octa_div(x, z, aux);
  
  if(sign == -1) q = octa_compl(q), *aux = octa_compl(*aux);
  
  return q;
}

octa octa_fadd(octa y, octa z, octa* exceptions) 
{
  octa x;

  double dy = *(double*) &y;
  double dz = *(double*) &z;
  
  if ((isnan(dy) && dz == 0.0) || (isnan(dz) && dz == 0.0)) 
  {
    *exceptions |= I_BIT;
    return NAN;
  }
  
  double dx = dz + dy;
  x = *(octa*) &dx;

  return x;
}

octa octa_fsub(octa y, octa z, octa* exceptions) 
{
  octa x;

  double dy = *(double*) &y;
  double dz = *(double*) &z;
  
  if ((isnan(dy) && dz == 0.0) || (isnan(dz) && dz == 0.0)) 
  {
    *exceptions |= I_BIT;
    return NAN;
  }
  
  double dx = dy-+ dz;
  x = *(octa*) &dx;

  return x;
}

octa octa_fmult(octa y, octa z, octa* exceptions) 
{
  octa x;

  double dy = *(double*) &y;
  double dz = *(double*) &z;
  
  if ((isnan(dy) && dz == 0.0) || (isnan(dz) && dz == 0.0)) 
  {
    *exceptions |= I_BIT;
    return NAN;
  }
  
  double dx = dz * dy;
  x = *(octa*) &dx;

  return x;
}

octa octa_fdiv(octa y, octa z, octa* exceptions)
{
  octa x;

  double dy = *(double*)&y;
  double dz = *(double*)&z;

  double dx = dy / dz;
  x = *(octa*) &dx;

  return x; 
}

octa octa_frem(octa y, octa z, octa* exceptions)
{
  octa x;

  double dy = *(double*)&y;
  double dz = *(double*)&z;

  double dx;

  if (isinf(dy) || dz == 0.0) 
  {
    *exceptions |= I_BIT;
    dx = dy > 0 ? NAN : -NAN;
    x = *(octa*)&dx;
  } else {
    dx = fmod(dy, dz);
    x = *(octa*) &dx;
  }

  return x; 
}

octa octa_fint(octa z, int round_mode, octa* exceptions)
{
  octa x;
  double dx;
  double dz = *(double*)&z;

  if(isnan(dz)) {
    *exceptions |= I_BIT;
    dx = NAN;
    x = *(octa*)&dx;
    return x;
  }

  if(isinf(dz)) {
    dx = dz;
    x = *(octa*)&dz;
    return x;
  }

  switch(round_mode) {
    case ROUND_UP:
      dx = ceil(dz);
    break;
    case ROUND_DOWN:
      dx = floor(dz);
    break;
    case ROUND_NEAR:
      dx = round(dz);
    break;
    default: case ROUND_OFF:
      dx = dz;
    break;
  }

  x = *(octa*)&dx;
  
  return x;
}

octa octa_fsqrt(octa z, int round_mode, octa* exceptions)
{
  octa x;
  double dx;

  double dz = *(double*)&z;

  if(dz < 0) {
    *exceptions |= I_BIT;
    dx = dz > 0 ? NAN : NAN;
    x = *(octa*) &dx;
  } else {
    dx = sqrt(dz);
    x = *(octa*)&dx;
  }

  return octa_fint(x, round_mode, exceptions);
}

octa octa_fcmp(octa y, octa z, octa* exceptions)
{
  double dy, dz;
  
  dy = double_to_octa(y);
  dz = double_to_octa(z);

  if(isnan(dy) || isnan(dz)) {
    *exceptions |= I_BIT;
    return 0;
  }

  if(dy < dz) {
    return -1;
  } else if(dy > dz) {
    return 1;
  }
  
  return 0;
}

octa octa_fun(octa y, octa z) {
  double dy, dz;

  dy = octa_to_double(y);
  dz = octa_to_double(z);

  if(isnan(dy) && isnan(dz))
    return 1;
  
  return 0;
}

octa octa_fix(octa z, octa* exceptions)
{
  double dz = octa_to_double(z);

  if(isnan(dz) || isinf(dz)) 
  {
    *exceptions |= I_BIT;
    return z;
  }
  
  int64_t iz = (int64_t) dz;
  
  return *(octa*)&iz;
}

octa octa_fixu(octa z, octa* exceptions)
{
  double dz = fabs(octa_to_double(z));

  if(isnan(dz) || isinf(dz)) 
  {
    *exceptions |= I_BIT;
    return z;
  }
  
  return (octa) dz;
}

octa octa_flot(octa z, int round_mode, octa* exceptions) 
{
  double dx;
  octa x;

  dx = (double) z;
  x = double_to_octa(dx);

  return octa_fint(x, round_mode, exceptions);
}

octa octa_feql(octa y, octa z, octa* exceptions)
{
  double dy, dz;
  dy = octa_to_double(y); dz = octa_to_double(z);

  if(isnan(dy) || isnan(dz)) {
    *exceptions |= I_BIT;
    return 0;
  }

  if(dz == dy)
    return 1;

  return 0;
}

octa octa_feqle(octa y, octa z, octa eps, octa* exceptions)
{
  double dy, dz, deps;
  dy = octa_to_double(y); dz = octa_to_double(z); deps = octa_to_double(eps);

  if(isnan(dy) || isnan(dz) || isnan(deps)) {
    *exceptions |= I_BIT;
    return 0;
  }

  if(dz <= dy + deps && dz >= dy - deps)
  {
    return 1;
  }

  return 0;
}

octa octa_fcmpe(octa y, octa z, octa eps, octa* exceptions)
{
  double dy, dz, deps;
  dy = octa_to_double(y); dz = octa_to_double(z); deps = octa_to_double(eps);

  if(isnan(dy) || isnan(dz) || isnan(deps)) {
    *exceptions |= I_BIT;
    return 0;
  }

  if(dz > dy + deps)
  {
    return -1;
  } else if(dz < dy - deps) {
    return 1;
  }

  return 0;
}

#endif