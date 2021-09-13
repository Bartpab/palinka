#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stddef.h>

typedef enum {
  false, true
} bool;

typedef unsigned char byte;
typedef uint16_t word;
typedef uint32_t tetra;
typedef uint64_t octa;
typedef struct {
  octa h, l;
} hexadeca;

octa octa_zero = 0;
octa octa_int_max = 0x7FFFFFFFFFFFFFFF;
octa octa_uint_max = 0xFFFFFFFFFFFFFFFF;
octa octa_sign_bit = 0x8000000000000000;

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
void* octa_to_voidp(octa x);

////////////
/// IMPL ///
////////////

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

void* octa_to_voidp(octa x) {
  return (void*)x;
}
#endif