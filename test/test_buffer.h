#include <string.h>

#include "utils.h"
#include "../src/buffer.h"
#include "../src/allocator.h"

define_test(basic_buffer, test_print("Basic buffer")) 
{
  const char* e = "this";
  
  allocator_t allocator = GLOBAL_ALLOCATOR;
  buffer_t buffer = buffer_init;
  string_t content = str_init;

  test_check(
    test_print("Create a buffer"),
    buffer_create(&buffer, 16, &allocator),
    test_failure("Failed to create a buffer...")
  );
  
  buffer_write_char(&buffer, 't');
  buffer_write_char(&buffer, 'h');
  buffer_write_char(&buffer, 'i');
  buffer_write_char(&buffer, 's');

  content = buffer_to_str(&buffer);

  test_check(
    test_print("Check the content of the buffer"),
    strcmp(content.base, e) == 0,
    test_failure("Expecting '%s', got '%s'", e, content.base)
  );  

  test_success;
  test_teardown {
    buffer_delete(&buffer);
  }
  test_end;
}

define_test_chapter(
  buffer, test_print("Buffer"), 
  basic_buffer
)
