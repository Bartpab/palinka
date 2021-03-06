#include <string.h>

#include "../include/testing/utils.h"
#include "../include/buffer.h"
#include "../include/allocator.h"

#include "../include/string/core.h"
#include "../include/string/iterator.h"
#include "../include/string/vector.h"
#include "../include/string/process.h"

define_test(basic_string, test_print("Basic string")) 
{
  const char* e = "this"; 
  allocator_t allocator = GLOBAL_ALLOCATOR;

  string_t s1, s2; 
  s1 = s2 = string_init;

  string_move_from_const_char(&s1, e, 0);

  test_print("Move the raw string of characters to the managed string_t\n");

  test_check(
    test_print("Compare the content of the string."),
    strcmp(string_raw(&s1), e) == 0,
    test_failure("Expecting '%s', got '%s'", e, string_raw(&s1))
  );  

  test_check(
    test_print("Copy the raw string of characters to the managed string_t"),
    string_copy_from_const_char(&s2, e, &allocator),
    test_failure("Couldn't copy the content to a string_t")
  );

  test_check(
    test_print("Compare both strings"),
    string_compare(&s1, &s2) == 0,
    test_failure("Both strings are not equal...")
  )

  test_success;
  test_teardown {
    string_destruct(&s1);
    string_destruct(&s2);
  }
  test_end;
}

define_test(string_concat, test_print("String concat")) 
{
  const char* e1 = "this"; 
  const char* e2 = " is";
  const char* e3 = "this is";

  allocator_t allocator = GLOBAL_ALLOCATOR;

  string_t s1, s2, s3, s4; 
  s1 = s2 = s3 = s4 = string_init;

  string_move_from_const_char(&s1, e1, 0);
  string_move_from_const_char(&s2, e2, 0);
  string_move_from_const_char(&s3, e3, 0);

  string_concat(&s4, &s1, &s2, &allocator);

  test_check(
    test_print("Compare both strings"),
    string_compare(&s3, &s4) == 0,
    test_failure("Expecting '%s', got '%s'", string_raw(&s3), string_raw(&s4))
  );

  test_success;
  test_teardown {
    string_destruct(&s1);
    string_destruct(&s2);
    string_destruct(&s3);
    string_destruct(&s4);
  }
  test_end;
}

define_test(basic_string_vector, test_print("String vector"))
{
  string_t s = string_init;
  string_t* sp;

  string_vector_t vec = string_vector_init;
  allocator_t allocator = GLOBAL_ALLOCATOR;

  test_check(
    test_print("Check that the type descriptor has a copy routine."),
    string_desc.copy != NULL,
    test_failure("String type descriptor misses copy routine.")
  );

  test_check(
    test_print("Initialise a vector of string"),
    string_vector_create(&vec, 10, &allocator),
    test_failure("Could not initialise the vector...")
  );

  // Move the const char in a string
  string_move_from_const_char(&s, "this is a test", 0);
   
  // Copy the string to a cell in the vector
  test_check(
    test_print("Add the string to the vector"),
    string_vector_copy_add(&vec, &s),
    test_failure("Could not add the string to the vector")
  );
    
  test_check(
    test_print("Get the element at the index 0 of the vector."),
    string_vector_get(&vec, &sp, 0),
    test_failure("No element was found at index 0.")
  );
  
  test_check(
    test_print("Compare the stored string with the original string."),
    string_compare(sp, &s) == 0,
    test_failure("Expecting '%s', got '%s'", string_raw(&s), string_raw(sp))
  );

  test_success;
  test_teardown {
    string_destruct(&s);
    string_vector_destruct(&vec);
  }
  test_end;
}

define_test(string_vector_eq, test_print("String vector equality"))
{
  string_t s1, s2;
  s1 = s2 = string_init;

  string_vector_t v1, v2;
  v1 = v2 = string_vector_init;

  allocator_t allocator = GLOBAL_ALLOCATOR;

  test_check(
    test_print("Initialise a vector of string"),
    string_vector_create(&v1, 10, &allocator),
    test_failure("Could not initialise the vector...")
  );

  test_check(
    test_print("Initialise a vector of string"),
    string_vector_create(&v2, 10, &allocator),
    test_failure("Could not initialise the vector...")
  );

  // Move the const char in a string
  string_move_from_const_char(&s1, "this is a test", 0);
  string_move_from_const_char(&s2, "this is another test", 0);
   
  // Copy the string to a cell in the vector
  test_check(
    test_print("Add the string to the vector"),
    string_vector_copy_add(&v1, &s1),
    test_failure("Could not add the string to the vector")
  );

  test_check(
    test_print("Add the string to the vector"),
    string_vector_copy_add(&v1, &s2),
    test_failure("Could not add the string to the vector")
  );

  test_check(
    test_print("Add the string to the vector"),
    string_vector_copy_add(&v2, &s1),
    test_failure("Could not add the string to the vector")
  );
    
  test_check(
    test_print("Add the string to the vector"),
    string_vector_copy_add(&v2, &s2),
    test_failure("Could not add the string to the vector")
  );

  test_check(
    test_print("Compare both list"),
    string_vector_eq(&v1, &v2),
    test_failure("Lists are not the same.")
  );
  
  test_success;
  test_teardown {
    string_destruct(&s1);
    string_destruct(&s2);
    string_vector_destruct(&v1);
    string_vector_destruct(&v2);
  }
  test_end;
}

define_test(string_split_char, test_print("String split"))
{
  string_t s1, s2;
  s1 = s2 = string_init;

  string_vector_t v1, v2;
  v1 = v2 = string_vector_init;

  string_vector_iterator_t it;

  allocator_t allocator = GLOBAL_ALLOCATOR;

  test_check(
    test_print("Initialise a vector of string"),
    string_vector_create(&v1, 10, &allocator),
    test_failure("Could not initialise the vector...")
  );

  test_check(
    test_print("Initialise a vector of string"),
    string_vector_create(&v2, 10, &allocator),
    test_failure("Could not initialise the vector...")
  );

  // Move the const char in a string
  string_move_from_const_char(&s1, "this", 0); string_vector_copy_add(&v1, &s1);
  string_move_from_const_char(&s1, "is", 0); string_vector_copy_add(&v1, &s1);
  string_move_from_const_char(&s1, "a", 0); string_vector_copy_add(&v1, &s1),
  string_move_from_const_char(&s1, "test", 0); string_vector_copy_add(&v1, &s1),

  string_move_from_const_char(&s2, "this is a test", 0);
   
  // Copy the string to a cell in the vector
  test_check(
    test_print("Split the string "),
    string_split_char(&v2, &s2, ' '),
    test_failure("Could not split the string")
  );

  string_vector_iter(&v2, &it);

  while(it.next(&it)) {
    test_print("'%s'\n", string_raw(it.get(&it)));
  }

  test_check(
    test_print("Compare both list"),
    string_vector_eq(&v1, &v2),
    test_failure("Lists are not the same.")
  );
  
  test_success;
  test_teardown {
    string_destruct(&s1);
    string_destruct(&s2);
    string_vector_destruct(&v1);
    string_vector_destruct(&v2);
  }
  test_end;
}

define_test(string_join_char, test_print("String join"))
{
  string_t s1, s2;
  s1 = s2 = string_init;

  string_vector_t v1;
  v1 = string_vector_init;

  string_vector_iterator_t it;

  allocator_t allocator = GLOBAL_ALLOCATOR;

  test_check(
    test_print("Initialise a vector of string"),
    string_vector_create(&v1, 10, &allocator),
    test_failure("Could not initialise the vector...")
  );

  // Move the const char in a string
  string_move_from_const_char(&s1, "this", 0); 
  string_vector_copy_add(&v1, &s1);
  
  string_move_from_const_char(&s1, "is", 0); 
  string_vector_copy_add(&v1, &s1);
  
  string_move_from_const_char(&s1, "a", 0); 
  string_vector_copy_add(&v1, &s1),
  
  string_move_from_const_char(&s1, "test", 0); 
  string_vector_copy_add(&v1, &s1),

  string_move_from_const_char(&s2, "this is a test", 0);
   
  string_vector_iter(&v1, &it);

  // Copy the string to a cell in the vector
  test_check(
    test_print("Split the string "),
    string_join_char(&s1, (string_iterator_t*)(&it), ' '),
    test_failure("Could not split the string")
  );

  test_check(
    test_print("Compare both string"),
    string_eq(&s1, &s2),
    test_failure("Expecting '%s', got '%s'.", string_raw(&s2), string_raw(&s1))
  );
  
  test_success;
  test_teardown {
    string_destruct(&s1);
    string_destruct(&s2);
    string_vector_destruct(&v1);
  }
  test_end;
}

define_test(string_concat_it, test_print("String concat iterator"))
{
  string_t e = string_init, s = string_init;

  string_vector_t vec = string_vector_init;
  string_vector_iterator_t it = string_vector_iterator_init;

  allocator_t allocator = GLOBAL_ALLOCATOR;

  test_check(
    test_print("Initialise a vector of string"),
    string_vector_create(&vec, 1, &allocator),
    test_failure("Could not initialise the vector...")
  );

  string_move_from_const_char(&e, "this is a test", 0);

  string_move_from_const_char(&s, "this is a ", 0);
  string_vector_copy_add(&vec, &s);

  string_move_from_const_char(&s, "test", 0);
  string_vector_copy_add(&vec, &s);
  
  // Create an iterator
  string_vector_iter(&vec, &it);

  size_t size = 0;

  while(it.next(&it)) size++;

  test_check(
    test_print("Check the size of the vector"),
    size == 2,
    test_failure("Wrong size, expecting 2, got %u", (unsigned int)size)
  );

  // Create an iterator
  string_vector_iter(&vec, &it);
  
  // Concat the strings
  string_concat_it(&s, (string_iterator_t*)(&it), &allocator);  

  test_check(
    test_print("Compare the concat string with the expected string."),
    string_eq(&e, &s),
    test_failure("Expecting '%s', got '%s'", string_raw(&e), string_raw(&s))
  );

  test_success;
  test_teardown {
    string_destruct(&s);
    string_vector_destruct(&vec);
  }
  test_end;
}

define_test_chapter(string, test_print("String"), 
  basic_string, 
  string_concat, 
  basic_string_vector, 
  string_vector_eq,
  string_split_char,
  string_join_char,
  string_concat_it
)
