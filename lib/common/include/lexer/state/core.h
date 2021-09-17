#ifndef __COMMON_LEXER_STATE_CORE_H__
#define __COMMON_LEXER_STATE_CORE_H__

#include "../../allocator.h"
#include "../../buffer.h"
#include "../token/vector.h"

#include "../transition/core.h"
#include "../transition/vector.h"

typedef struct lexer_state_t lexer_state_t;

struct lexer_state_t 
{ 
    lexer_transition_vector_t vec;
    int type;

    allocator_t __allocator;
};

const lexer_state_t lexer_state_init = {lexer_transition_vector_init, 0, NO_ALLOCATOR};

bool lexer_state_create(lexer_state_t* state, size_t capacity, int type, allocator_t* allocator);
void lexer_state_create_move(lexer_state_t* state, lexer_transition_vector_t* vec, int type);
bool lexer_state_move_add_transition(lexer_state_t* state, lexer_transition_t* transition);
bool lexer_state_copy_add_transition(lexer_state_t* state, const lexer_transition_t* transition, allocator_t* allocator);
void lexer_state_move(lexer_state_t* dest, lexer_state_t* src);
bool lexer_state_copy(lexer_state_t* dest, const lexer_state_t* src, allocator_t* allocator);
bool lexer_state_eq(const lexer_state_t* t1, const lexer_state_t* t2);
void lexer_state_delete(lexer_state_t* token);

token_vector_t lexer_run(lexer_state_t* init, const char* stream, allocator_t* allocator);

typedef struct {
    DECL_TYPE_DESC(lexer_state_t)
} lexer_state_desc_t;

const lexer_state_desc_t lexer_state_desc = {
    DEF_TYPE_DESC(
        lexer_state_copy,
        lexer_state_move, 
        lexer_state_eq,
        lexer_state_delete, 
        sizeof(lexer_state_t)
    )
};


static bool __next_transition(lexer_state_t* current, const char c, lexer_state_t** out) 
{
    lexer_transition_vector_iterator_t it;
    lexer_transition_vector_iter(&current->vec, &it);

    *out = NULL;

    while(it.next(&it)) 
    {
        lexer_transition_t* t = it.get(&it);
        if(lexer_is_valid_transition(c, t)) {
            *out = t->next;
            return true;
        }
    }

    return false;
}

static lexer_state_t* __lexer_step(lexer_state_t* init, buffer_t* buffer, const char** c) 
{
    const char* it = *c;
    lexer_state_t* state = init;

    while(*it != '\0') 
    {
        while(*it == '\0' && (*it == '\n' || *it == ' ')) it++;

        if(*it == '\0')
            break;

        lexer_state_t* next;

        if(!__next_transition(state, *it, &next)) 
        {
            *c = it;
            return state;
        }
        
        buffer_write_char(buffer, *it);

        it++;
        next = state;
    }
    
    *c = it;
    return state;
}

bool lexer_state_create(lexer_state_t* state, size_t capacity, int type, allocator_t* allocator)
{   
    if(!lexer_transition_vector_create(&state->vec, capacity, allocator))
        return false;
    
    state->type = type;
    return true;
}

void lexer_state_create_move(lexer_state_t* state, lexer_transition_vector_t* vec, int type)
{
    lexer_transition_vector_move(&state->vec, vec);
    state->type = type;
}

bool lexer_state_move_add_transition(lexer_state_t* state, lexer_transition_t* transition)
{
    return lexer_transition_vector_move_add(&state->vec, transition);
}

bool lexer_state_copy_add_transition(lexer_state_t* state, const lexer_transition_t* transition, allocator_t* allocator)
{
    return lexer_transition_vector_copy_add(&state->vec, transition, allocator);
}


void lexer_state_move(lexer_state_t* dest, lexer_state_t* src)
{
    lexer_transition_vector_move(&dest->vec, &src->vec);
    dest->type = src->type;
}

bool lexer_state_copy(lexer_state_t* dest, const lexer_state_t* src, allocator_t* allocator)
{
    if(!lexer_transition_vector_copy(&dest->vec, &src->vec, allocator))
        return false;

    dest->type = src->type;  
    return true; 
}

bool lexer_state_eq(const lexer_state_t* t1, const lexer_state_t* t2)
{
    return lexer_transition_vector_eq(&t1->vec, &t2->vec) && t1->type == t2->type;
}

void lexer_state_delete(lexer_state_t* state)
{
    lexer_transition_vector_delete(&state->vec);
    allocator_t allocator = allocator_copy(&state->__allocator);
    pfree(&allocator, state);
    allocator_delete(&allocator);
}

token_vector_t lexer_run(lexer_state_t* init, const char* stream, allocator_t* allocator) 
{
    const char* it;   
    unsigned int col, row;

    token_vector_t toks = token_vector_init;
    buffer_t buffer = buffer_init;
    string_t str = string_init;

    it = stream;
    col = row = 0;

    token_vector_create(&toks, 32, allocator);
    buffer_create(&buffer, 32, allocator);

    while(*it != '\0') 
    {
        while(*it == '\0' && (*it == '\n' || *it == ' ')) it++;

        lexer_state_t* st = __lexer_step(init, &buffer, &it);
        buffer_copy_to_string(&str, &buffer, allocator);

        token_t tok = {st->type, str, row, col};
        token_vector_move_add(&toks, &tok);
        
        buffer_reset(&buffer);
        
        str = string_init;
    }

    buffer_delete(&buffer);
    string_delete(&str);

    return toks;
}


#endif