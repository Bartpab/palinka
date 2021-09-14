#ifndef __MMIX_ASM_CORE_H__
#define __MMIX_ASM_CORE_H__

#include "../../allocator.h"

typedef struct {
    int   type,
    string_t value,
    unsigned int line, col;
} token_t;

#endif