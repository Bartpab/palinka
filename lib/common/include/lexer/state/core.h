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
};

const lexer_state_t lexer_state_init = {lexer_transition_vector_init, 0};

lexer_state_t lexer_state(size_t capacity, int type, allocator_t* allocator);
bool lexer_state_create(lexer_state_t* state, size_t capacity, int type, allocator_t* transition_vector_allocator);
void lexer_state_create_move(lexer_state_t* state, lexer_transition_vector_t* vec, int type);
bool lexer_state_move_add_transition(lexer_state_t* state, lexer_transition_t* transition);
bool lexer_state_copy_add_transition(lexer_state_t* state, const lexer_transition_t* transition);
void lexer_state_move(lexer_state_t* dest, lexer_state_t* src);
bool lexer_state_copy(lexer_state_t* dest, const lexer_state_t* src);
bool lexer_state_eq(const lexer_state_t* t1, const lexer_state_t* t2);
void lexer_state_destruct(lexer_state_t* token);

void lexer_run(token_vector_t* toks, lexer_state_t* init, const char* stream, allocator_t* allocator);

typedef struct {
    DECL_TYPE_DESC(lexer_state_t)
} lexer_state_desc_t;

const lexer_state_desc_t lexer_state_desc = {
    DEF_TYPE_DESC(
        lexer_state_copy,
        lexer_state_move, 
        lexer_state_eq,
        lexer_state_destruct, 
        sizeof(lexer_state_t)
    )
};

bool lexer_next_transition(lexer_state_t* current, const char c, lexer_state_t** out) 
{
    lexer_transition_vector_iterator_t it;
    lexer_transition_vector_iter(&current->vec, &it);

    *out = NULL;

    while(it.next(&it)) 
    {
        lexer_transition_t* t = it.get(&it);

        if(lexer_is_valid_transition(c, t)) 
        {
            *out = t->next;
            return true;
        }
    }

    return false;
}

lexer_state_t* lexer_step(lexer_state_t* init, buffer_t* buffer, const char** c) 
{
    const char* it = *c;
    lexer_state_t* state = init;
    lexer_state_t* next;

    while(*it != '\0') 
    {
        if(false == lexer_next_transition(state, *it, &next)) 
        {
            *c = it;
            return state;
        }
        
        buffer_write_char(buffer, *it);

        it++;
        state = next;
    }
    
    *c = it;
    return state;
}

lexer_state_t lexer_state(size_t capacity, int type, allocator_t* allocator)
{
    lexer_state_t tmp = lexer_state_init;
    lexer_state_create(&tmp, capacity, type, allocator);
    return tmp;
}

bool lexer_state_create(lexer_state_t* state, size_t capacity, int type, allocator_t* transitions_allocator)
{   
    if(!lexer_transition_vector_create(&state->vec, capacity, transitions_allocator))
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

bool lexer_state_copy_add_transition(lexer_state_t* state, const lexer_transition_t* transition)
{
    return lexer_transition_vector_copy_add(&state->vec, transition);
}

void lexer_state_move(lexer_state_t* dest, lexer_state_t* src)
{
    lexer_transition_vector_move(&dest->vec, &src->vec);
    dest->type = src->type;
}

bool lexer_state_copy(lexer_state_t* dest, const lexer_state_t* src)
{
    if(!lexer_transition_vector_copy(&dest->vec, &src->vec))
        return false;

    dest->type = src->type;  
    return true; 
}

bool lexer_state_eq(const lexer_state_t* t1, const lexer_state_t* t2)
{
    return lexer_transition_vector_eq(&t1->vec, &t2->vec) && t1->type == t2->type;
}

void lexer_state_destruct(lexer_state_t* state)
{
    lexer_transition_vector_destruct(&state->vec);
}

void lexer_run(token_vector_t* toks, lexer_state_t* init, const char* stream, allocator_t* allocator) 
{
    const char* it = stream;   
    unsigned int col = 0, row = 0;

    buffer_t buff       = buffer(32, allocator);
    string_t str        = string();
    
    lexer_state_t* st_end;

    while(*it != '\0') 
    {
        while(*it != '\0' && (*it == '\n' || *it == ' ')) it++;

        if(*it == '\0')
            break;

        st_end = lexer_step(init, &buff, &it);
        buffer_copy_to_string(&str, &buff);

        token_t tok = token_move_value(st_end->type, &str, row, col);
        token_vector_move_add(toks, &tok);
        
        buffer_reset(&buff);
    }

    buffer_destruct(&buff);
    string_destruct(&str);
}


#endif