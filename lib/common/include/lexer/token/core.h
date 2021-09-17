#ifndef __LEXER_TOKEN_H__
#define __LEXER_TOKEN_H__

#include "../../allocator.h"
#include "../../string/core.h"
#include "../../types/desc.h"

typedef struct token_t {
    int   type;
    string_t value;
    unsigned int line, col;
} token_t;

const token_t token_init = {0, string_init, 0, 0};

token_t token_const_chars(int type, const char* value, unsigned int line, unsigned int col);
void token_create_const_chars(token_t* tok, int type, const char* value, unsigned int line, unsigned int col);
void token_move(token_t* dest, token_t* src);
bool token_copy(token_t* dest, const token_t* src, allocator_t* allocator);
bool token_eq(const token_t* t1, const token_t* t2);
void token_delete(token_t* token);

typedef struct {
    DECL_TYPE_DESC(token_t)
} token_desc_t;

const token_desc_t token_desc = {
    DEF_TYPE_DESC(
        token_copy,
        token_move, 
        token_eq,
        token_delete, 
        sizeof(token_t)
    )
};


token_t token_const_chars(int type, const char* value, unsigned int line, unsigned int col)
{
    token_t tmp = token_init;
    token_create_const_chars(&tmp, type, value, line, col);
    return tmp;
}

void token_create_const_chars(token_t* tok, int type, const char* value, unsigned int line, unsigned int col)
{
    string_t tmp = string_init;
    string_move_from_const_char(&tmp, value, 0);
    tok->type = type;
    tok->line = line;
    tok->col = col;
}

void token_move(token_t* dest, token_t* src)
{
    dest->type = src->type;
    string_move(&dest->value, &src->value);
    dest->col = src->col;
    dest->line = src->line;
}

bool token_copy(token_t* dest, const token_t* src, allocator_t* allocator)
{
    if(!string_copy(&dest->value, &src->value, allocator))
        return false;

    dest->type = src->type;
    dest->col = src->col;
    dest->line = src->line;  

    return true; 
}

bool token_eq(const token_t* t1, const token_t* t2)
{
    bool r;

    r = t1->type == t2->type;
    r = r && string_eq(&t1->value, &t2->value);
    
    return r;
}

void token_delete(token_t* token) 
{
    string_delete(&token->value);
}

#endif