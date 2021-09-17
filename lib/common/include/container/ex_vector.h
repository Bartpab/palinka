#ifndef __COMMON_LEXER_STATE_VECTOR_H__
#define __COMMON_LEXER_STATE_VECTOR_H__

typedef struct typealias_vector_t typealias_vector_t;
typedef struct typealias_vector_iterator_t typealias_vector_iterator_t;

struct typealias_vector_t {
    vector_t impl;
};

const typealias_vector_t typealias_vector_init = {vector_init};

struct typealias_vector_iterator_t {
    DECL_ITERATOR(struct typealias_vector_iterator_t, type_t);
    vector_iterator_t impl;
};

bool typealias_vector_create(typealias_vector_t* vec, size_t capacity, allocator_t* allocator);
void typealias_vector_delete(typealias_vector_t* vec);
bool typealias_vector_get(typealias_vector_t* vec, type_t** out, unsigned int index);
void typealias_vector_move(typealias_vector_t* dest, typealias_vector_t* src);
bool typealias_vector_copy(typealias_vector_t* dest, const typealias_vector_t* src, allocator_t* allocator);
bool typealias_vector_eq(const typealias_vector_t* v1, const typealias_vector_t* v2);
bool typealias_vector_move_add(typealias_vector_t* vec, type_t* element);
bool typealias_vector_copy_add(typealias_vector_t* vec, const type_t* element, allocator_t* allocator);
void typealias_vector_iter(typealias_vector_t* vec, typealias_vector_iterator_t* it);

bool typealias_vector_iterator_next(typealias_vector_iterator_t* it);
type_t* typealias_vector_iterator_get(typealias_vector_iterator_t* it);

const typealias_vector_iterator_t typealias_vector_iterator_init = {typealias_vector_iterator_next, typealias_vector_iterator_get, vector_it_init};

bool typealias_vector_create(typealias_vector_t* vec, size_t capacity, allocator_t* allocator)
{
    return vector_create(
        &vec->impl, 
        (type_desc_t*)(&typealias_desc), 
        capacity, 
        allocator
    );
}

void typealias_vector_delete(typealias_vector_t* vec)
{
    vector_delete(&vec->impl);
}

void typealias_vector_move(typealias_vector_t* dest, typealias_vector_t* src)
{
    vector_move(&dest->impl, &src->impl);
}

bool typealias_vector_copy(typealias_vector_t* dest, const typealias_vector_t* src, allocator_t* allocator)
{
    vector_copy(&dest->impl, &src->impl);
}

bool typealias_vector_get(typealias_vector_t* vec, type_t** out, unsigned int index)
{
    return vector_get(&vec->impl, (void**) out, index);
}

bool typealias_vector_eq(const typealias_vector_t* v1, const typealias_vector_t* v2)
{
    return vector_eq(&v1->impl, &v2->impl);
}

bool typealias_vector_move_add(typealias_vector_t* vec, type_t* element)
{
    return vector_move_add(&vec->impl, element);
}

bool typealias_vector_copy_add(typealias_vector_t* vec, const type_t* element, allocator_t* allocator)
{
    return vector_copy_add(&vec->impl, element, allocator);
}

void typealias_vector_iter(typealias_vector_t* vec, typealias_vector_iterator_t* it)
{
    vector_iter(&vec->impl, &it->impl);
    it->next = typealias_vector_iterator_next;
    it->get = typealias_vector_iterator_get;
}

bool typealias_vector_iterator_next(typealias_vector_iterator_t* it)
{
    return vector_iterator_next(&it->impl);
}

type_t* typealias_vector_iterator_get(typealias_vector_iterator_t* it)
{
    return (type_t*) vector_iterator_get(&it->impl);
}

#endif