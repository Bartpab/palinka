#ifndef __MMIX_ASM_LEXER_H__
#define __MMIX_ASM_LEXER_H__

#include "../../lib/common/include/lexer/state/core.h"

/*
digraph G {
    s0;
    s1[label="s1:: decimal_constant"];
    
    s2[label="local_operand"];
    s3[label="local_label"]
    s4[label="s4:: decimal_constant"]
    s6[label="hex_constant"]
    s9[label="char_constant"]
    s10[label="symbol"]
    s11[label="unary_operator"]
    s12[label="weak_operator"]
    s14[label="strong_operator"]
    s18[label="@"]
    
    s0 -> s1[label="[0-9]"];
    s1 -> s2[label="B|F"];
    s1 -> s3[label="H"];
    s1 -> s4[label="[0-9]"];
    s4 -> s4[label="[0-9]"];
    
    s0 -> s5[label="#"]
    s5 -> s6[label="[0-9]"]
    s6 -> s6[label="[0-9]"]
    
    s0 -> s7[label="['`]"]
    s7 -> s8[label="[A-Za-z0-9]"]
    s8 -> s9[label="['`]"]
    
    s0 -> s10[label="[A-Za-z]"]
    s10 -> s10[label="[A-Za-z0-9_]"]
    
    s0 -> s11[label="[+-~$&]"]
    s0 -> s12[label="[+-|^]"]
    
    s0 -> s14[label="[/*%&]"]
    s0 -> s15[label="[<]"]
    s15 -> s14[label="[<]"]
    s0 -> s16[label="[/]"]
    s16 -> s14[label="[/]"]
    s0 -> s17[label="[>]"]
    s17 -> s14[label="[>]"]
    
    s0 -> s18[label="@"]
}
*/

typedef enum {
    INVALID,
    SYMBOL,
    DECIMAL_CONSTANT,
    CHAR_CONSTANT,
    HEX_CONSTANT,
    LOCAL_OPERAND,
    LOCAL_LABEL,
    UNARY_OPERATOR,
    WEAK_OPERATOR,
    STRONG_OPERATOR,
    AROBASE
} mmix_token_type_t;

const char symbol_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789";
const char char_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789";
const char hex_digits_chars[] = "ABCDEFacbdef0123456789";
const char decimal_digits_chars[] = "0123456789";

lexer_state_t states[19];

lexer_transition_t s0_transitions[10];
lexer_transition_t s1_transitions[3];
lexer_transition_t s4_transitions[1];
lexer_transition_t s5_transitions[1];
lexer_transition_t s6_transitions[1];
lexer_transition_t s7_transitions[1];
lexer_transition_t s8_transitions[1];
lexer_transition_t s10_transitions[1];
lexer_transition_t s15_transitions[1];
lexer_transition_t s16_transitions[1];
lexer_transition_t s17_transitions[1];

bool lexer_initialised = false;

lexer_state_t* get_lexer()
{
    if(lexer_initialised)
        return &states[0];
    
    states[0].type = INVALID;
    states[1].type = DECIMAL_CONSTANT;
    states[2].type = LOCAL_OPERAND;
    states[3].type = INVALID;
    states[4].type = DECIMAL_CONSTANT;
    states[5].type = INVALID;
    states[6].type = HEX_CONSTANT;
    states[7].type = INVALID;
    states[8].type = INVALID;
    states[9].type = CHAR_CONSTANT;
    states[10].type = SYMBOL;
    states[11].type = UNARY_OPERATOR;
    states[12].type = WEAK_OPERATOR;
    states[13].type = INVALID;
    states[14].type = STRONG_OPERATOR;
    states[15].type = INVALID;
    states[16].type = INVALID;
    states[17].type = INVALID;
    states[18].type = AROBASE;

    lexer_initialised = true;
    return &states[0];
}

#endif