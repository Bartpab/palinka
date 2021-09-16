#ifndef __COMMON_STRING_VECTOR_H__
#define __COMMON_STRING_VECTOR_H__

#include "../container/vector.h"
#include "./core.h"

typedef struct {
    vector_t impl;
} string_vector_t;

const string_vector_t string_vector_init = {vector_init};

typedef struct string_vector_iterator_t {
    DECL_ITERATOR(struct string_vector_iterator_t, string_t);
    vector_iterator_t impl;
} string_vector_iterator_t;

bool string_vector_create(string_vector_t* vec, size_t capacity, allocator_t* allocator);
void string_vector_delete(string_vector_t* vec);
bool string_vector_get(string_vector_t* vec, string_t** out, unsigned int index);
bool string_vector_eq(const string_vector_t* v1, const string_vector_t* v2);
bool string_vector_move_add(string_vector_t* vec, string_t* element);
bool string_vector_copy_add(string_vector_t* vec, const string_t* element, allocator_t* allocator);
void string_vector_iter(string_vector_t* vec, string_vector_iterator_t* it);

bool string_vector_iterator_next(string_vector_iterator_t* it);
string_t* string_vector_iterator_get(string_vector_iterator_t* it);

const string_vector_iterator_t string_vector_iterator_init = {string_vector_iterator_next, string_vector_iterator_get, vector_it_init};

bool string_vector_create(string_vector_t* vec, size_t capacity, allocator_t* allocator)
{
    return vector_create(
        &vec->impl, 
        (type_desc_t*)(&string_desc), 
        capacity, 
        allocator
    );
}

void string_vector_delete(string_vector_t* vec)
{
    vector_delete(&vec->impl);
}

bool string_vector_get(string_vector_t* vec, string_t** out, unsigned int index)
{
    return vector_get(&vec->impl, (void**) out, index);
}

bool string_vector_eq(const string_vector_t* v1, const string_vector_t* v2)
{
    return vector_eq(&v1->impl, &v2->impl);
}

bool string_vector_move_add(string_vector_t* vec, string_t* element)
{
    return vector_move_add(&vec->impl, element);
}

bool string_vector_copy_add(string_vector_t* vec, const string_t* element, allocator_t* allocator)
{
    return vector_copy_add(&vec->impl, element, allocator);
}

void string_vector_iter(string_vector_t* vec, string_vector_iterator_t* it)
{
    vector_iter(&vec->impl, &it->impl);
    it->next = string_vector_iterator_next;
    it->get = string_vector_iterator_get;
}

bool  string_vector_iterator_next(string_vector_iterator_t* it)
{
    return vector_iterator_next(&it->impl);
}

string_t* string_vector_iterator_get(string_vector_iterator_t* it)
{
    return (string_t*) vector_iterator_get(&it->impl);
}
#endif