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

#endif