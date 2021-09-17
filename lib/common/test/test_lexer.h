#include "../include/testing/utils.h"
#include "../include/lexer/state/core.h"

lexer_state_t s0;
lexer_state_t s1;



define_test(basic_lexer, test_print("Basic lexer"))
{
  const char* phrase = "this is 90 1234";
  const char* letter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char* number = "0123456789";

  allocator_t allocator = GLOBAL_ALLOCATOR;
  
  lexer_state_t states[3] = {
    lexer_state(2, -1, &allocator), 
    lexer_state(1, 0, &allocator), 
    lexer_state(1, 1, &allocator)
  };
  
  lexer_transition_t transitions[2] = {
    lexer_transition_const_chars(letter, &states[1]), 
    lexer_transition_const_chars(number, &states[2])
  };
  
  token_t tokens[4] = {
    token_const_chars(0, "this", 0, 0),
    token_const_chars(0, "is", 0, 0),
    token_const_chars(1, "90", 0, 0),
    token_const_chars(1, "1234", 0, 0)
  };

  token_vector_t toks = token_vector(6, &allocator), etoks = token_vector(4, &allocator);

  return;

  token_vector_copy_add(&etoks, &tokens[1], &allocator);
  token_vector_copy_add(&etoks, &tokens[2], &allocator);
  token_vector_copy_add(&etoks, &tokens[3], &allocator);
  token_vector_copy_add(&etoks, &tokens[4], &allocator);


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
