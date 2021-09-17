#ifndef __COMMON_LEXER_STATE_IT_H__
#define __COMMON_LEXER_STATE_IT_H__

#include "../../container/iterator.h"
#include "../../buffer.h"
#include "./core.h"

typedef struct lexer_state_iterator_t {
    DECL_ITERATOR(struct lexer_state_iterator_t, lexer_state_t)
} lexer_state_iterator_t;

#endif