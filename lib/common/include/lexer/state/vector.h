#ifndef __COMMON_LEXER_STATE_VECTOR_H__
#define __COMMON_LEXER_STATE_VECTOR_H__

#include "../container/vector.h"
#include "../container/macro_vector.h"
#include "./core.h"

typedef struct lexer_state_vector_t lexer_state_vector_t;

VECTOR_DECL(lexer_state_t, lexer_state)

#endif