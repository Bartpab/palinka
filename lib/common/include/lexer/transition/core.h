#ifndef __COMMON_LEXER_TRANSITION_CORE_H__
#define __COMMON_LEXER_TRANSITION_CORE_H__

#include "../../types/desc.h"
#include "../../allocator.h"

typedef struct lexer_state_t lexer_state_t;
typedef struct lexer_transition_t lexer_transition_t;

struct lexer_transition_t {
    const char* chars;
    lexer_state_t* next;
    allocator_t __allocator;
};

const lexer_transition_t lexer_transition_init = {0, 0, NO_ALLOCATOR};

lexer_transition_t* lexer_transition_new(const char* chars, lexer_state_t* next, allocator_t* allocator);
lexer_transition_t lexer_transition_const_chars(const char* chars, lexer_state_t* next);
void lexer_transition_create_const_chars(lexer_transition_t* transition, const char* chars, struct lexer_state_t* next);
bool lexer_is_valid_transition(const char c, lexer_transition_t* transition);

void lexer_transition_move(lexer_transition_t* dest, lexer_transition_t* src);
bool lexer_transition_copy(lexer_transition_t* dest, const lexer_transition_t* src, allocator_t* allocator);
bool lexer_transition_eq(const lexer_transition_t* t1, const lexer_transition_t* t2);
void lexer_transition_delete(lexer_transition_t* token);

typedef struct {
    DECL_TYPE_DESC(lexer_transition_t)
} lexer_transition_desc_t;

const lexer_transition_desc_t lexer_transition_desc = {
    DEF_TYPE_DESC(
        lexer_transition_copy,
        lexer_transition_move, 
        lexer_transition_eq,
        lexer_transition_delete, 
        sizeof(lexer_transition_t)
    )
};

lexer_transition_t* lexer_transition_new(const char* chars, lexer_state_t* next, allocator_t* allocator)
{
    lexer_transition_t* ts = (lexer_transition_t*) pmalloc(allocator, sizeof(lexer_transition_t));
    if(ts == NULL)
        return NULL;
    
    lexer_transition_create_const_chars(ts, chars, next);
    ts->__allocator = allocator_copy(allocator);
    return ts;
}

lexer_transition_t lexer_transition_const_chars(const char* chars, lexer_state_t* next)
{
    lexer_transition_t t = lexer_transition_init;
    lexer_transition_create_const_chars(&t, chars, next);
    return t;   
}

void lexer_transition_create_const_chars(lexer_transition_t* transition, const char* chars, struct lexer_state_t* next)
{
    transition->chars = chars;
    transition->next = next;
}

bool lexer_is_valid_transition(const char c, lexer_transition_t* transition) 
{
    for(const char* it = transition->chars; *it != '\0'; it++) 
    {
        if(*it == c)
            return true;
    }

    return false;
}

void lexer_transition_move(lexer_transition_t* dest, lexer_transition_t* src)
{
    dest->chars = src->chars;
    dest->next = src->next;
}

bool lexer_transition_copy(lexer_transition_t* dest, const lexer_transition_t* src, allocator_t* allocator)
{
    dest->chars = src->chars;
    dest->next = src->next;
    return true;
}

bool lexer_transition_eq(const lexer_transition_t* t1, const lexer_transition_t* t2)
{
    return strcmp(t1->chars, t2->chars) && t1->next == t2->next;
}

void lexer_transition_delete(lexer_transition_t* token)
{
    allocator_t allocator = allocator_copy(&token->__allocator);
    pfree(&allocator, token);
    allocator_delete(&token->__allocator);
}

#endif