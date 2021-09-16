#ifndef __COMMON_STRING_IT_H__
#define __COMMON_STRING_IT_H__

#include "../../container/iterator.h"
#include "../../buffer.h"
#include "./core.h"

typedef struct token_iterator_t {
    DECL_ITERATOR(struct token_iterator_t, token_t)
} token_iterator_t;

#endif