#include "../include/testing/utils.h"
#include "../include/lexer/state/core.h"

lexer_state_t s0;
lexer_state_t s1;



define_test(basic_lexer, test_print("Basic lexer"))
{
  allocator_t         allocator = GLOBAL_ALLOCATOR;
  lexer_state_t       states[3] = {lexer_state_init, lexer_state_init, lexer_state_init};
  lexer_transition_t  transitions[2] = {lexer_transition_init, lexer_transition_init};
  token_t             tokens[6] = {token_init, token_init, token_init, token_init, token_init, token_init};
  token_vector_t      toks = token_vector_init, etoks = token_vector_init;

  const char* phrase = "this is a word 90 1234";
  const char* letter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char* number = "0123456789";

  token_create_const_chars(&tokens[0], 0, "this", 0, 0);
  token_create_const_chars(&tokens[1], 0, "is", 0, 0);
  token_create_const_chars(&tokens[2], 0, "a", 0, 0);
  token_create_const_chars(&tokens[3], 0, "word", 0, 0);
  token_create_const_chars(&tokens[4], 0, "90", 0, 0);
  token_create_const_chars(&tokens[5], 0, "1234", 0, 0);

  token_vector_create(&etoks, 6, &allocator);

  lexer_state_create(&states[0], 10, -1, &allocator); // Init
  lexer_state_create(&states[1], 10, 0, &allocator); // Word [a-zA-Z]+
  lexer_state_create(&states[2], 10, 1, &allocator); // Number [0-9]+

  lexer_transition_create_const_chars(&transitions[0], number, &states[1]); // -[a-zA-Z]-> sWord
  lexer_transition_create_const_chars(&transitions[1], letter, &states[2]); // -[0-9]-> sNumber

  lexer_state_copy_add_transition(&states[0], &transitions[0], &allocator);
  lexer_state_copy_add_transition(&states[0], &transitions[1], &allocator);
  lexer_state_copy_add_transition(&states[1], &transitions[0], &allocator); 
  lexer_state_copy_add_transition(&states[2], &transitions[1], &allocator); 

  toks = lexer_run(&states[0], phrase, &allocator);

  test_check(
    test_print("Check the tokens obtained from the lexer."),
    token_vector_eq(&toks, &etoks),
    test_failure("The list of tokens is wrong...")
  );

  test_success;
  test_teardown;
  token_vector_delete(&toks);
  token_vector_delete(&etoks);

  lexer_state_delete(&states[0]);
  lexer_state_delete(&states[1]);
  lexer_state_delete(&states[2]);

  lexer_transition_delete(&transitions[0]);
  lexer_transition_delete(&transitions[1]);
  test_end;
}

define_test_chapter(
  lexer, test_print("Lexer"), 
  basic_lexer
)
