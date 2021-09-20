#include <string.h>

#include "../include/testing/utils.h"
#include "../include/buffer.h"
#include "../include/string/core.h"
#include "../include/allocator.h"

define_test(basic_buffer, test_print("Basic buffer")) 
{
  const char* e = "this";
  
  allocator_t allocator = GLOBAL_ALLOCATOR;
  buffer_t buffer = buffer_init;
  string_t content = string_init;

  test_check(
    test_print("Create a buffer"),
    buffer_create(&buffer, 16, &allocator),
    test_failure("Failed to create a buffer...")
  );
  
  string_move_from_const_char(&content, e, 0);
  buffer_write_string(&buffer, &content);
  buffer_move_to_string(&content, &buffer);

  test_check(
    test_print("Check the content of the buffer"),
    strcmp(string_raw(&content), e) == 0,
    test_failure("Expecting '%s', got '%s'", e, string_raw(&content))
  );  

  test_success;
  test_teardown {
    buffer_destruct(&buffer);
    string_destruct(&content);
  }
  test_end;
}

define_test_chapter(buffer, test_print("Buffer"), basic_buffer)
