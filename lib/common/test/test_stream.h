#include "../include/testing/utils.h"
#include "../include/stream/buffer.h"
#include "../include/allocator.h"

define_test(stream_read, test_print("Read buffer")) 
{
  const char* e = "this is a test.";
  char txt[100];
  stream_t stream = stream_init;

  test_check(
    test_print("Open the stream"),
    stream_open_file("../test/assets/read.txt", "rb", &stream),
    test_failure("Failed to open the stream...")
  )

  test_check(
    test_print("Read the data from the stream"),
    stream_read(&stream, txt, sizeof(char) * 100) > 0,
    test_failure("Reading failed")
  );

  test_check(
    test_print("Check the content of the stream is '%s'", e),
    strcmp(txt, e) == 0,
    test_failure("Expecting '%s', got '%s'", e, txt)
  );  

  test_success;
  test_teardown {
    stream_close(&stream);
  }
  test_end;
}

define_test(stream_write, test_print("Read buffer")) 
{
  const char* w = "this is a writing test.";
  char txt[200];
  const size_t sw = strlen(w);
  stream_t stream = stream_init;

  test_check(
    test_print("Open the stream"),
    stream_open_file("../test/assets/write.txt", "wb", &stream),
    test_failure("Failed to open the stream...")
  );

  test_check(
    test_print("Write the data in the stream"),
    stream_write(&stream, w, sw) == sw,
    test_failure("Writing failed")
  );

  // Close the stream
  stream_close(&stream);

  test_check(
    test_print("Open the stream"),
    stream_open_file("../test/assets/write.txt", "rb", &stream),
    test_failure("Failed to open the stream...")
  );

  test_check(
    test_print("Read the data from the stream"),
    stream_read(&stream, txt, sizeof(char) * 100) > 0,
    test_failure("Reading failed")
  );

  test_check(
    test_print("Check the content of the stream is '%s'", w),
    strcmp(txt, w) == 0,
    test_failure("Expecting '%s', got '%s'", w, txt)
  );  

  test_success;
  test_teardown {
    stream_close(&stream);
  }
  test_end;
}

define_test(rbuffer, test_print("Reading buffer")) {
  //const char* e = "this is a test.";

  allocator_t allocator = GLOBAL_ALLOCATOR;
  stream_t stream = stream_init;
  rbuffer_t buffer = rbuffer_init;
  string_t content = string_init;

  test_check(
    test_print("Open the stream"),
    stream_open_file("../test/assets/read.txt", "rb", &stream),
    test_failure("Failed to open the stream...")
  );

  test_check(
    test_print("Create the reading buffer"),
    rbuffer_create(&buffer, &stream, 16, &allocator),
    test_failure("Failed to create the buffer...")
  );


  test_success;
  test_teardown {
    string_delete(&content);
    rbuffer_delete(&buffer);
    stream_close(&stream);
  }
  test_end;
}

define_test(wbuffer, test_print("Writing buffer")) 
{
  const char* w = "this is a writing test.";
  const size_t sw = strlen(w);

  allocator_t allocator = GLOBAL_ALLOCATOR;
  stream_t stream = stream_init;

  wbuffer_t wbuffer = wbuffer_init;
  rbuffer_t rbuffer = rbuffer_init;
  string_t content = string_init;

  test_check(
    test_print("Open the stream"),
    stream_open_file("../test/assets/buffer_write.txt", "wb", &stream),
    test_failure("Failed to open the stream...")
  );

  test_check(
    test_print("Create the buffer"),
    wbuffer_create(&wbuffer, &stream, 200, &allocator),
    test_failure("Failed to create the buffer...")
  );

  test_check(
    test_print("Write in the buffer"),
    wbuffer_write(&wbuffer, w, sw) == sw,
    test_failure("Failed to write in the buffer...")
  );

  test_check(
    test_print("Check the size of the buffer"),
    wbuffer.size == sw,
    test_failure("%lld", wbuffer.size)
  );
  
  wbuffer_flush(&wbuffer);
  wbuffer_delete(&wbuffer);
  stream_close(&stream);

  test_check(
    test_print("Open the stream"),
    stream_open_file("../test/assets/buffer_write.txt", "rb", &stream),
    test_failure("Failed to open the stream...")
  ); 

  rbuffer_create(&rbuffer, &stream, 64, &allocator);
  content = rbuffer_read_all_str(&rbuffer, 16, &allocator);

  test_check(
    test_print("Check the written file"),
    content.length == sw + 1, // Count the '\0' at the end
    test_failure("Wrong size")
  );

  test_check(
    test_print("Check the content of the stream is '%s'", w),
    strcmp(content.base, w) == 0,
    test_failure("Expecting '%s', got '%s'", w, content.base)
  );  

  test_success;
  test_teardown {
    string_delete(&content);
    wbuffer_delete(&wbuffer);
    rbuffer_delete(&rbuffer);
    stream_close(&stream);
  }
  test_end;
}

define_test_chapter(
  stream, test_print("Stream"), 
  rbuffer//, wbuffer,
  //stream_read, stream_write
)
