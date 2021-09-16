#ifndef __COMMON_LEXER_TOKEN_VECTOR_H__
#define __COMMON_LEXER_TOKEN_VECTOR_H__

#include "../../container/vector.h"
#include "./core.h"

typedef struct {
    vector_t impl;
} token_vector_t;

const token_vector_t token_vector_init = {vector_init};

typedef struct token_vector_iterator_t {
    DECL_ITERATOR(struct token_vector_iterator_t, token_t);
    vector_iterator_t impl;
} token_vector_iterator_t;

bool token_vector_create(token_vector_t* vec, size_t capacity, allocator_t* allocator);
void token_vector_delete(token_vector_t* vec);
bool token_vector_get(token_vector_t* vec, token_t** out, unsigned int index);
bool token_vector_eq(const token_vector_t* v1, const token_vector_t* v2);
bool token_vector_move_add(token_vector_t* vec, token_t* element);
bool token_vector_copy_add(token_vector_t* vec, const token_t* element, allocator_t* allocator);
void token_vector_iter(token_vector_t* vec, token_vector_iterator_t* it);

bool token_vector_iterator_next(token_vector_iterator_t* it);
token_t* token_vector_iterator_get(token_vector_iterator_t* it);

const token_vector_iterator_t token_vector_iterator_init = {token_vector_iterator_next, token_vector_iterator_get, vector_it_init};

bool token_vector_create(token_vector_t* vec, size_t capacity, allocator_t* allocator)
{
    return vector_create(
        &vec->impl, 
        (type_desc_t*)(&token_desc), 
        capacity, 
        allocator
    );
}

void token_vector_delete(token_vector_t* vec)
{
    vector_delete(&vec->impl);
}

bool token_vector_get(token_vector_t* vec, token_t** out, unsigned int index)
{
    return vector_get(&vec->impl, (void**) out, index);
}

bool token_vector_eq(const token_vector_t* v1, const token_vector_t* v2)
{
    return vector_eq(&v1->impl, &v2->impl);
}

bool token_vector_move_add(token_vector_t* vec, token_t* element)
{
    return vector_move_add(&vec->impl, element);
}

bool token_vector_copy_add(token_vector_t* vec, const token_t* element, allocator_t* allocator)
{
    return vector_copy_add(&vec->impl, element, allocator);
}

void token_vector_iter(token_vector_t* vec, token_vector_iterator_t* it)
{
    vector_iter(&vec->impl, &it->impl);
    it->next = token_vector_iterator_next;
    it->get = token_vector_iterator_get;
}

bool  token_vector_iterator_next(token_vector_iterator_t* it)
{
    return vector_iterator_next(&it->impl);
}

token_t* token_vector_iterator_get(token_vector_iterator_t* it)
{
    return (token_t*) vector_iterator_get(&it->impl);
}
#endif