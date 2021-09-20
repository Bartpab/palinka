#ifndef __STRING_CORE_H__
#define __STRING_CORE_H__

#include "../allocator.h"
#include "../buffer.h"
#include "../types/desc.h"

#include <string.h>

typedef struct string_t {
    bool is_const;
    union {
        char* base;
        const char* cbase;
    };
    size_t length;

    allocator_t char_allocator;  // Raw char array allocator
} string_t;

const string_t string_init = {0, .base = 0, 0, NO_ALLOCATOR};

string_t* string_new(allocator_t* allocator);
void string_delete(string_t* str, allocator_t* allocator);

string_t string();
/**
 * \brief Return an access to the raw pointer of char
 */
const char* string_raw(const string_t* pstr);
int string_compare(const string_t* s1, const string_t* s2);
bool string_eq(const string_t* s1, const string_t* s2);
bool string_concat(string_t* dest, const string_t* s1, const string_t* s2, allocator_t* allocator);

void string_move(string_t* dest, string_t* src);
bool string_copy(string_t* dest, const string_t* src);
const size_t string_length(const string_t* pstr);
void string_clear(string_t* str); 
void string_destruct(string_t * str);

bool buffer_write_string(buffer_t* buffer, string_t* str);
bool buffer_move_to_string(string_t *pstr, buffer_t* buffer);
bool buffer_copy_to_string(string_t *pstr, buffer_t* buffer);

/**
 * \brief Move the content to the string_t
 */
bool string_move_from_const_char(string_t* pstr, const char* cstr, allocator_t* allocator);

/**
 * \brief Copy the content to the string_t
 */
bool string_copy_from_const_char(string_t* pstr, const char* cstr, allocator_t* allocator);

typedef struct {
    DECL_TYPE_DESC(string_t)
} string_desc_t;

const string_desc_t string_desc = {
    DEF_TYPE_DESC(
        string_copy,
        string_move, 
        string_eq,
        string_destruct, 
        sizeof(string_t)
    )
};
// IMPL //
string_t string()
{
    return string_init;
}
string_t* string_new(allocator_t* allocator)
{
    string_t* tmp = (string_t*) pmalloc(allocator, sizeof(string_t));
    *tmp = string_init;
    return tmp;
}

bool string_move_from_const_char(string_t* pstr, const char* cstr, allocator_t* char_allocator)
{
    string_clear(pstr);

    allocator_t callocator = char_allocator == NULL ? NO_ALLOCATOR : allocator_copy(char_allocator);
    
    pstr->is_const = 1;
    pstr->cbase = cstr;
    pstr->char_allocator = callocator;
    pstr->length = strlen(cstr);
    
    return true;
}

/**
 * \brief Get the ownership of the content behind the raw const char pointer
 */
bool string_move_from_char(string_t* pstr, char* cstr, allocator_t* allocator)
{
    string_clear(pstr);

    allocator_t callocator = allocator == NULL ? NO_ALLOCATOR : allocator_copy(allocator);
    pstr->is_const = 0;
    pstr->base = cstr;
    pstr->char_allocator = callocator;
    pstr->length = strlen(cstr);
    
    return true;
}

/**
 * \brief Copy the content of the const char pointer and store it in the string.
 */
bool string_copy_from_const_char(string_t* pstr, const char* cstr, allocator_t* allocator) 
{
    string_clear(pstr);

    size_t length = strlen(cstr);
    pstr->base = (char*) pmalloc(allocator, length + 1);
    
    if(pstr->base == NULL)
        return false;

    memcpy(pstr->base, cstr, length + 1);
    *(pstr->base + length) = '\0';

    pstr->length = length;
    pstr->char_allocator = allocator_copy(allocator);

    return true;
}

void string_destruct(string_t* str)
{
    string_clear(str);
}

void string_delete(string_t* str, allocator_t* allocator)
{
    string_destruct(str);
    pfree(allocator, str);
}

void string_clear(string_t * str) 
{   
    if(!str->is_const)  
    {
        if(str->base != NULL) 
        {
            pfree(&str->char_allocator, str->base);
            str->base = NULL;
            allocator_delete(&str->char_allocator);      
        }
    }

    str->char_allocator = NO_ALLOCATOR;
    str->length = 0;
}

bool string_concat(string_t* dest, const string_t* s1, const string_t* s2, allocator_t* allocator)
{
    string_clear(dest);

    size_t length = string_length(s1) + string_length(s2);
    char* base = (char*) pmalloc(allocator, length + 1);
    
    if(!base)
        return false;

    memcpy(base, string_raw(s1), string_length(s1));
    memcpy(base + string_length(s1), string_raw(s2), string_length(s2));
    
    *(base + length) = '\0';

    dest->base = base;
    dest->length = length;
    dest->char_allocator = allocator_copy(allocator);

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

const char* string_raw(const string_t* pstr) 
{
    return pstr->is_const ? pstr->cbase : pstr->base;
}

void string_move(string_t* dest, string_t* src) 
{
    string_clear(dest);

    if(dest->is_const)
        dest->base = src->base;
    else
        dest->cbase = src->cbase;
    
    dest->char_allocator = src->char_allocator;
    dest->length = src->length;
    dest->is_const = src->is_const;

    *src = string_init;
}

bool string_copy(string_t* dest, const string_t* src)
{
    string_clear(dest);

    dest->char_allocator = allocator_copy(&src->char_allocator);
    
    // Special case with global const char wrapper
    if(dest->char_allocator.type == NO_ALLOCATOR.type && src->is_const) {
        dest->cbase = src->cbase;
        dest->is_const = true;
        dest->length = src->length;
        dest->char_allocator = allocator_copy(&src->char_allocator);
        
        return true;
    } else if(dest->char_allocator.type == NO_ALLOCATOR.type) {
        return false;
    } else {
        char* base = (char*) pmalloc(&dest->char_allocator, string_length(src) + 1);
        
        if(base == NULL)
            return false;
        
        memcpy(base, string_raw(src), string_length(src) + 1);

        dest->base = base;
        dest->is_const = false;
        dest->length = string_length(src);
        
        return true;
    }
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

bool buffer_copy_to_string(string_t* str, buffer_t* buff)
{
    if(buff->base == NULL)
        return false;
    
    buffer_t tmp = buffer(buff->capacity, &buff->allocator);

    if(!buffer_copy(&tmp, buff))
        return false;

    return buffer_move_to_string(str, &tmp);
}

bool buffer_write_string(buffer_t* buffer, string_t* str)
{
    return buffer_write(buffer, string_raw(str), string_length(str));
}


#endif