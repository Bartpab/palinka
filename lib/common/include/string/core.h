#ifndef __STRING_CORE_H__
#define __STRING_CORE_H__

#include "../allocator.h"
#include "../buffer.h"

#include <string.h>

typedef struct {
    bool is_const;
    union {
        char* base;
        const char* cbase;
    };
    size_t length;
    allocator_t allocator;
} string_t;

const string_t string_init = {0, .base = 0, 0, NO_ALLOCATOR};

/**
 * \brief Return an access to the raw pointer of char
 */
const char* string_raw(const string_t* pstr);
int string_compare(const string_t* s1, const string_t* s2);
bool string_eq(const string_t* s1, const string_t* s2);
bool string_concat(string_t* dest, const string_t* s1, const string_t* s2, allocator_t* allocator);


void string_move(string_t* dest, string_t* src);
bool string_copy(string_t* dest, const string_t* src, allocator_t* allocator);
const size_t string_length(const string_t* pstr); 
void string_delete(string_t * str);


bool buffer_write_string(buffer_t* buffer, string_t* str);
bool buffer_move_to_string(string_t *pstr, buffer_t* buffer);
bool buffer_copy_to_string(string_t *pstr, buffer_t* buffer, allocator_t* allocator);

/**
 * \brief Move the content to the string_t
 */
bool string_move_from_const_char(string_t* pstr, const char* cstr, allocator_t* allocator);

/**
 * \brief Copy the content to the string_t
 */
bool string_copy_from_const_char(string_t* pstr, const char* cstr, allocator_t* allocator);

/**
 * \brief String iterator 
 **/
typedef struct string_it_t {
    bool (*next)(struct string_it_t* it);
    string_t* (*get)(struct string_it_t* it);
} string_iterator_t;

/**
 * \brief Consume the iterator and concat its elements
 */
bool string_concat_it(string_t* dest, string_iterator_t* it, allocator_t* allocator);

// String vector //

typedef struct {
    string_t* base;
    size_t capacity;
    size_t size;

    allocator_t allocator;
} string_vector_t;

const string_vector_t string_vector_init = {0, 0, 0, NO_ALLOCATOR};

typedef struct string_vector_it_t
{
    bool (*next)(struct string_vector_it_t* it);
    string_t* (*get)(struct string_vector_it_t* it);

    string_t* current;
    string_t* base;
    string_t* limit;

} string_vector_it_t;

bool string_vector_iterator_next(string_vector_it_t* it);
string_t* string_vector_iterator_get(string_vector_it_t* it);

const string_vector_it_t string_vector_it_init = {string_vector_iterator_next, string_vector_iterator_get, 0, 0, 0};

/**
 * \brief Create a vector of string
 */
bool string_vector_create(string_vector_t* vec, size_t capacity, allocator_t* allocator);

/**
 * \brief Delete the vector of string
 */
void string_vector_delete(string_vector_t* vec);

bool string_vector_get(string_vector_t* vec, string_t** out, unsigned int index);

/**
 * \brief Init an iterator over a vector of string.
 */
void string_vector_iter(string_vector_t* vec, string_vector_it_t* it);

/**
 * \brief Add the element to the vector, move the string.
 */
bool string_vector_move_add(string_vector_t* vec, string_t* str);

/**
 * \brief Add the element to the vector, copy the string.
 */
bool string_vector_copy_add(string_vector_t* vec, const string_t* str, allocator_t* allocator);


// IMPL //

bool string_vector_iterator_next(string_vector_it_t* it) 
{
    if(it->base == 0)
        return false;
        
    if(it->current == it->limit)
        return false;
    
    if(it->current == NULL)
        it->current = it->base;
    else
        it->current++;
    
    return true;
}

string_t* string_vector_iterator_get(string_vector_it_t* it) 
{
    return it->current;
}

bool string_vector_create(string_vector_t* vec, size_t capacity, allocator_t* allocator)
{
    *vec = string_vector_init;

    string_t* base = (string_t*) pmalloc(allocator, sizeof(string_t) * capacity);
    
    if(base == NULL)
        return false;

    vec->base = base;
    vec->capacity = capacity;
    vec->size = 0;
    vec->allocator = allocator_copy(allocator);

    return true;
}

void string_vector_delete(string_vector_t* vec)
{
    if(vec->base == NULL)
        return;
    
    string_vector_it_t it;
    string_vector_iter(vec, &it);

    while(it.next(&it)) string_delete(it.get(&it));

    pfree(&vec->allocator, vec->base);
    allocator_delete(&vec->allocator);
    vec->base = 0;
    vec->size = 0;
}


bool string_vector_get(string_vector_t* vec, string_t** out, unsigned int index)
{
    if(vec->base == NULL)
        return false;
    
    if(index >= vec->size)
        return false;
    
    *out = vec->base + index;
    return true;
}

void string_vector_iter(string_vector_t* vec, string_vector_it_t* it)
{
    *it = string_vector_it_init;

    it->current = 0;
    it->base = vec->base;
    it->limit = vec->base + vec->size - 1;
}

static bool __string_vector_check_capacity(string_vector_t* vec, size_t size) 
{
    if(size >= vec->capacity)
    {
        size_t new_capacity = vec->capacity << 1;
        
        string_t* base = (string_t*) prealloc(
            &vec->allocator, 
            vec->base, 
            sizeof(string_t) * new_capacity
        );
        
        if(base == NULL)
            return false;

        vec->base = base;
        vec->capacity = new_capacity;
    }

    return true;
}

bool string_vector_move_add(string_vector_t* vec, string_t* str)
{
    if(vec->base == NULL)
        return false;

    // Check the capacity of the array, and try to readjust if too small (can fail...)
    if(!__string_vector_check_capacity(vec, vec->size + 1))
        return false;

    string_t* curr = vec->base + vec->size;
    string_move(curr, str);
    vec->size++;

    return true;
}

bool string_vector_copy_add(string_vector_t* vec, const string_t* str, allocator_t* allocator)
{
    if(vec->base == NULL)
        return false;

    // Check the capacity of the array, and try to readjust if too small (can fail...)
    if(!__string_vector_check_capacity(vec, vec->size + 1))
        return false;

    string_t tmp = string_init;
    
    if(!string_copy(&tmp, str, allocator))
        return false;

    *(vec->base + vec->size) = tmp;
    vec->size++;

    return true;
}

bool string_move_from_const_char(string_t* pstr, const char* cstr, allocator_t* allocator)
{
    string_delete(pstr);

    allocator_t callocator = allocator == NULL ? NO_ALLOCATOR : allocator_copy(allocator);
    
    pstr->is_const = 1;
    pstr->cbase = cstr;
    pstr->allocator = callocator;
    pstr->length = strlen(cstr);
    
    return true;
}

/**
 * \brief Get the ownership of the content behind the raw const char pointer
 */
bool string_move_from_char(string_t* pstr, char* cstr, allocator_t* allocator)
{
    string_delete(pstr);

    allocator_t callocator = allocator == NULL ? NO_ALLOCATOR : allocator_copy(allocator);
    pstr->is_const = 0;
    pstr->base = cstr;
    pstr->allocator = callocator;
    pstr->length = strlen(cstr);
    
    return true;
}

/**
 * \brief Copy the content of the const char pointer and store it in the string.
 */
bool string_copy_from_const_char(string_t* pstr, const char* cstr, allocator_t* allocator) 
{
    string_delete(pstr);

    size_t length = strlen(cstr);
    pstr->base = (char*) pmalloc(allocator, length + 1);
    
    if(pstr->base == NULL)
        return false;

    memcpy(pstr->base, cstr, length + 1);
    *(pstr->base + length) = '\0';

    pstr->length = length;
    pstr->allocator = allocator_copy(allocator);

    return true;
}

void string_delete(string_t * str) 
{   
    if(!str->is_const)  {
        if(str->base != NULL) {
            pfree(&str->allocator, str->base);
            str->base = NULL;
            allocator_delete(&str->allocator);      
        }
    }

    str->allocator = NO_ALLOCATOR;
    str->length = 0;
}

bool string_concat_it(string_t* dest, string_iterator_t* it, allocator_t* allocator)
{
    string_delete(dest);

    buffer_t buffer;
    
    if(!buffer_create(&buffer, 2, allocator))
        return false;


    while(it->next(it)) 
    {
        if(false == buffer_write_string(&buffer, it->get(it)))
        {
            buffer_delete(&buffer);
            return false;
        }
    }

    buffer_move_to_string(dest, &buffer);
    buffer_delete(&buffer);
    
    return true;
}

bool string_concat(string_t* dest, const string_t* s1, const string_t* s2, allocator_t* allocator)
{
    string_delete(dest);

    size_t length = string_length(s1) + string_length(s2);
    char* base = (char*) pmalloc(allocator, length + 1);
    
    if(!base)
        return false;

    memcpy(base, string_raw(s1), string_length(s1));
    memcpy(base + string_length(s1), string_raw(s2), string_length(s2));
    
    *(base + length) = '\0';

    dest->base = base;
    dest->length = length;
    dest->allocator = allocator_copy(allocator);

    return true;
}

int string_compare(const string_t* s1, const string_t* s2) 
{
    return strcmp(string_raw(s1), string_raw(s2));
}

bool string_eq(const string_t* s1, const string_t* s2)
{
    return string_compare(s1, s2) == 0;
}

const size_t string_length(const string_t* pstr) {
    return pstr->length;
}

const char* string_raw(const string_t* pstr) {
    return pstr->is_const ? pstr->cbase : pstr->base;
}

void string_move(string_t* dest, string_t* src) {
    string_delete(dest);

    if(dest->is_const)
        dest->base = src->base;
    else
        dest->cbase = src->cbase;
    
    dest->allocator = src->allocator;
    dest->length = src->length;
    dest->is_const = src->is_const;

    *src = string_init;
}

bool string_copy(string_t* dest, const string_t* src, allocator_t* allocator)
{
    string_delete(dest);
    char* base = (char*) pmalloc(allocator, string_length(src) + 1);
    
    if(base == NULL)
        return false;
    
    memcpy(base, string_raw(src), string_length(src) + 1);

    dest->base = base;
    dest->allocator = allocator_copy(allocator);
    dest->is_const = 0;
    dest->length = string_length(src);
    return true;
}

/**
 * \brief Move the buffer into a string_t
 * 
 * The buffer must not be used afterwards !
 */
bool buffer_move_to_string(string_t *pstr, buffer_t* buffer) 
{
    if(buffer->base == NULL)
        return false;

    buffer_write_char(buffer, '\0');
    string_move_from_char(pstr, (char*) buffer->base, &buffer->allocator);
    
    buffer->base = NULL;
    allocator_delete(&buffer->allocator);
    
    return true;
}

bool buffer_copy_to_string(string_t* str, buffer_t* buffer, allocator_t* allocator)
{
    if(buffer->base == NULL)
        return false;
    
    buffer_t tmp;

    if(!buffer_copy(&tmp, buffer, allocator))
        return false;

    return buffer_move_to_string(str, &tmp);
}

bool buffer_write_string(buffer_t* buffer, string_t* str)
{
    return buffer_write(buffer, string_raw(str), string_length(str));
}


#endif