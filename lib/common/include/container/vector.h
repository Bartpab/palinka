#ifndef __CONTAINER_VECTOR_H__
#define __CONTAINER_VECTOR_H__

#include <string.h>
#include <assert.h>

#include "../allocator.h"
#include "../types/desc.h"

#include "./iterator.h"

typedef struct vector_t {
    void* base;
    type_desc_t* type_desc;

    size_t capacity;
    size_t size;

    allocator_t __elements_allocator;
} vector_t;

const vector_t vector_init = {0, 0, 0, 0, NO_ALLOCATOR};

typedef struct vector_iterator_t
{
    DECL_ITERATOR(struct vector_iterator_t, void)

    void* current;
    void* base;
    void* limit;

    type_desc_t* type_desc;

} vector_iterator_t;


/**
 * \brief Fetch the next element of the iterator
 * 
 * \return true if the next element exists, false else;
 */
bool  vector_iterator_next(vector_iterator_t* it);
void* vector_iterator_get(vector_iterator_t* it);

const vector_iterator_t vector_it_init = {vector_iterator_next, vector_iterator_get, 0, 0, 0, 0};

/**
 * \brief Create a vector of string
 */
bool vector_create(vector_t* vec, type_desc_t* type_desc, size_t capacity, const allocator_t* elements_allocator);
void vector_create_array(vector_t* vec, type_desc_t* type_desc, void* base, size_t length);

/**
 * \brief Delete the vector of string
 */
void vector_clear(vector_t* vec);

/**
 * \brief Move the vector
 */
void vector_move(vector_t* dest, vector_t* src);

/**
 * \brief Copy the vector
 */
bool vector_copy(vector_t* dest, const vector_t* src);

/**
 * \brief Vector eq
 */
bool vector_eq(const vector_t* v1, const vector_t* v2);

/**
 * \brief Get the element at index
 */
bool vector_get(vector_t* vec, void** out, unsigned int index);

/**
 * \brief Init an iterator over a vector of string.
 */
void vector_iter(const vector_t* vec, vector_iterator_t* it);

/**
 * \brief Add the element to the vector
 */
bool vector_move_add(vector_t* vec, void* element);

/**
 * \brief Add the element to the vector
 */
bool vector_copy_add(vector_t* vec, const void* element);

// IMPL //

bool vector_create(vector_t* vec, type_desc_t* type_desc, size_t capacity, const allocator_t* elements_allocator)
{
    allocator_t __el_allocator = allocator_copy(elements_allocator);

    void* base = pmalloc(&__el_allocator, type_desc->size * capacity);
    
    if(!base)
        return false;
    
    vec->base = base;
    vec->type_desc = type_desc;
    vec->capacity = capacity;
    vec->__elements_allocator = __el_allocator;
    vec->size = 0;

    return true;
}

void vector_create_array(vector_t* vec, type_desc_t* type_desc, void* base, size_t length)
{
    vec->base = base;
    vec->capacity = length;
    vec->size = length;
    vec->__elements_allocator = NO_ALLOCATOR;
}

static bool __vector_check_capacity(vector_t* vec, size_t size) 
{
    if(size > vec->capacity)
    {
        size_t new_capacity = vec->capacity << 1;
        
        void* base = prealloc(
            &vec->__elements_allocator, 
            vec->base, 
            vec->type_desc->size * new_capacity
        );
        
        if(base == NULL)
            return false;

        vec->base = base;
        vec->capacity = new_capacity;
    }

    return true;
}

void vector_destruct(vector_t* vec) 
{
    if(vec->base == NULL)
        return;

    if(vec->size > 0) 
    {
        void* it;
        
        for(size_t i; i < vec->size; i++) 
        {
            it = vec->base + i * vec->type_desc->size;
            type_destruct(vec->type_desc, it);
        }
    }

    pfree(&vec->__elements_allocator, vec->base);
    vec->base = NULL;
    allocator_delete(&vec->__elements_allocator);
}

void vector_move(vector_t* dest, vector_t* src)
{
    dest->base  = src->base;
    dest->capacity = src->capacity;
    dest->size = src->size;
    dest->type_desc = src->type_desc;
    dest->__elements_allocator = src->__elements_allocator;

    src->base = 0;
    src->capacity = 0;
    src->size = 0;
    src->type_desc = 0;
    src->__elements_allocator = NO_ALLOCATOR;
}

bool vector_copy(vector_t* dest, const vector_t* src)
{
    if(src->base == NULL)
        return false;
    
    if(type_is_copiable(src->type_desc))
        return false;

    if(!vector_create(dest, src->type_desc, src->capacity, &src->__elements_allocator))
        return false;

    void* it_src, *it_dest;
    
    for(size_t i; i < src->size; i++) 
    {
        it_src = src->base + i * src->type_desc->size;
        it_dest = dest->base + i * src->type_desc->size;
        type_copy(src->type_desc, it_dest, it_src);
    }

    return true;
}

bool vector_get(vector_t* vec, void** out, unsigned int index)
{
    if(index >= vec->size)
        return false;
    
    *out = vec->base + (index * vec->type_desc->size);
    
    return true;
}

void vector_iter(const vector_t* vec, vector_iterator_t* it)
{
    *it = vector_it_init;
    
    it->base        = vec->base;
    it->current     = NULL;
    it->type_desc   = vec->type_desc;
    it->limit       = it->base + (vec->type_desc->size * (vec->size - 1));
}

bool vector_eq(const vector_t* v1, const vector_t* v2)
{
    if(v1->type_desc != v2->type_desc)
        return false;

    if(v1->type_desc->eq == NULL)
        return false;

    if(v2->size != v1->size)
        return false;

    vector_iterator_t it1, it2;
    
    vector_iter(v1, &it1);
    vector_iter(v2, &it2);

    while(it1.next(&it1) && it2.next(&it2)) 
    {
        if(false == v1->type_desc->eq(it1.get(&it1), it2.get(&it2)))
            return false;
    }

    return true;
}

bool vector_move_add(vector_t* vec, void* element)
{
    if(vec->base == NULL)
        return false;
        
    if(!type_is_movable(vec->type_desc))
        return false;

    if(!__vector_check_capacity(vec, vec->size + 1))
        return false;

    void* curr = vec->base + (vec->size * vec->type_desc->size);
    type_move(vec->type_desc, curr, element);
    vec->size++;
    return true;
}

bool vector_copy_add(vector_t* vec, const void* element)
{
    if(vec->base == NULL)
        return false;

    if(!type_is_copiable(vec->type_desc))
        return false;

    if(!__vector_check_capacity(vec, vec->size + 1))
        return false;

    void* curr = vec->base + (vec->size * vec->type_desc->size);
    
    if(!type_copy(vec->type_desc, curr, element))
        return false;
    
    vec->size++;
    
    return true;
}

bool vector_iterator_next(vector_iterator_t* it)
{
    if(it->base == NULL)
        return false;
    
    if(it->current >= it->limit)
        return false;

    if(it->current == NULL) 
        it->current = it->base;
    else 
        it->current = it->current + it->type_desc->size;
    
    return true;
}

void* vector_iterator_get(vector_iterator_t* it)
{
    return it->current;
}

#endif