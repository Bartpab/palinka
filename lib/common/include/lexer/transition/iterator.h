#ifndef __COMMON_LEXER_TRANSITION_IT_H__
#define __COMMON_LEXER_TRANSITION_IT_H__

#include "../../container/iterator.h"
#include "../../buffer.h"
#include "./core.h"

typedef struct lexer_transition_iterator_t lexer_transition_iterator_t;

struct lexer_transition_iterator_t {
    DECL_ITERATOR(struct lexer_transition_iterator_t, lexer_transition_t)
} lexer_transition_iterator_t;

#endif