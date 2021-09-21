#include "../include/testing/utils.h"
#include "../include/lexer/state/core.h"

define_test(lexer_transition, test_print("Lexer transition"))
{
  const char* letter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char* number = "0123456789";

  allocator_t allocator = GLOBAL_ALLOCATOR;
  lexer_state_t* st_ptr;

  lexer_state_t states[3] = {
    lexer_state(2, -1, &allocator), 
    lexer_state(1, 0, &allocator), 
    lexer_state(1, 1, &allocator)
  };
  
  lexer_transition_t transitions[2] = {
    lexer_transition_const_chars(letter, &states[1]), 
    lexer_transition_const_chars(number, &states[2])
  };

  lexer_state_copy_add_transition(&states[0], &transitions[0]);
  lexer_state_copy_add_transition(&states[0], &transitions[1]);
  lexer_state_copy_add_transition(&states[1], &transitions[0]); 
  lexer_state_copy_add_transition(&states[2], &transitions[1]); 
  
  test_check(
    test_print("Check that we got a valid transition"),
    lexer_next_transition(&states[0], 'c', &st_ptr),
    test_failure("No valid transition was found...")
  );

  test_check(
    test_print("Check the next state"),
    &states[1] == st_ptr,
    test_failure("Wrong state")
  );

  
  test_check(
    test_print("Check that we got an invalid transition"),
    !lexer_next_transition(&states[0], '@', &st_ptr),
    test_failure("A valid transition was found...")
  );

  test_success;
  test_teardown {
    lexer_state_destruct(&states[0]);
    lexer_state_destruct(&states[1]);
    lexer_state_destruct(&states[2]);

    lexer_transition_destruct(&transitions[0]);
    lexer_transition_destruct(&transitions[1]);
  }
  test_end;
}

define_test(lexer_step, test_print("Lexer step"))
{
  const char* letter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char* number = "0123456789";

  const char* phrase = "this@";
  const char** stream = &phrase;

  allocator_t allocator = GLOBAL_ALLOCATOR;
  buffer_t buf = buffer(32, &allocator);
  lexer_state_t* st_ptr;

  lexer_state_t states[3] = {
    lexer_state(2, -1, &allocator), 
    lexer_state(1, 0, &allocator), 
    lexer_state(1, 1, &allocator)
  };
  
  lexer_transition_t transitions[2] = {
    lexer_transition_const_chars(letter, &states[1]), 
    lexer_transition_const_chars(number, &states[2])
  };

  lexer_state_copy_add_transition(&states[0], &transitions[0]);
  lexer_state_copy_add_transition(&states[0], &transitions[1]);
  lexer_state_copy_add_transition(&states[1], &transitions[0]); 
  lexer_state_copy_add_transition(&states[2], &transitions[1]); 
  
  st_ptr = lexer_step(&states[0], &buf, stream);

  test_check(
    test_print("Check the next state"),
    &states[1] == st_ptr,
    test_failure("Wrong state")
  );
  
  test_success;
  test_teardown {
    lexer_state_destruct(&states[0]);
    lexer_state_destruct(&states[1]);
    lexer_state_destruct(&states[2]);

    lexer_transition_destruct(&transitions[0]);
    lexer_transition_destruct(&transitions[1]);
  }
  test_end;
}

define_test(lexer_run, test_print("Lexer run"))
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

  token_vector_t etoks, toks;
  
  toks = token_vector(6, &allocator); 
  etoks = token_vector(6, &allocator);
  
  token_vector_copy_add(&etoks, &tokens[0]);
  token_vector_copy_add(&etoks, &tokens[1]);
  token_vector_copy_add(&etoks, &tokens[2]);
  token_vector_copy_add(&etoks, &tokens[3]);

  lexer_state_copy_add_transition(&states[0], &transitions[0]);
  lexer_state_copy_add_transition(&states[0], &transitions[1]);
  lexer_state_copy_add_transition(&states[1], &transitions[0]); 
  lexer_state_copy_add_transition(&states[2], &transitions[1]); 

  lexer_run(&toks, &states[0], phrase, &allocator);
  
  token_vector_iterator_t it = token_vector_iterator_init;
  token_vector_iter(&toks, &it);

  while(it.next(&it)) 
  {
    test_print("%s\n", string_raw(&it.get(&it)->value));
  }

  test_check(
    test_print("Check the tokens obtained from the lexer."),
    token_vector_eq(&toks, &etoks),
    test_failure("The list of tokens is wrong...")
  );

  test_success;
  test_teardown;
  token_vector_destruct(&toks);
  token_vector_destruct(&etoks);

  lexer_state_destruct(&states[0]);
  lexer_state_destruct(&states[1]);
  lexer_state_destruct(&states[2]);

  lexer_transition_destruct(&transitions[0]);
  lexer_transition_destruct(&transitions[1]);
  test_end;
}

define_test_chapter(lexer, test_print("Lexer"), 
  lexer_transition, 
  lexer_step, 
  lexer_run
)
