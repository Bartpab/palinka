#ifndef __MMIX_ASM_CORE_H__
#define __MMIX_ASM_CORE_H__

#include "../../../lib/common/include/allocator.h"
#include "../../../lib/common/include/string/core.h"

typedef struct {
    int   type;
    string_t value;
    unsigned int line, col;
} token_t;

void token_delete(token_t* token) 
{
    string_delete(&token->value);
}

#endif