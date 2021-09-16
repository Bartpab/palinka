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

void token_move(token_t* dest, token_t* src);
void token_copy(token_t* dest, const token_t* src, allocator_t* allocator);
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

void token_move(token_t* dest, token_t* src)
{
    dest->type = src->type;
    string_move(&dest->value, &src->value);
    dest->col = src->col;
    dest->line = src->line;
}
void token_copy(token_t* dest, token_t* src, allocator_t* allocator)
{
    dest->type = src->type;
    string_copy(&dest->value, &src->value, allocator);
    dest->col = src->col;
    dest->line = src->line;   
}

bool token_eq(token_t* t1, token_t* t2)
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