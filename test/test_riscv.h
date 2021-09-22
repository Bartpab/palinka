#include <string.h> 

#include "../lib/common/include/testing/utils.h"
#include "../lib/common/include/allocator.h"
#include "../lib/common/include/stream/buffer.h"

#include "../src/system.h"
#include "../src/memory/core.h"

#include "../src/riscv/core.h"

system_t* riscv_bootstrap(char* prog, size_t prog_length, size_t memory)
{
  allocator_t allocator = GLOBAL_ALLOCATOR;

  system_t* sys = riscv_new(&allocator);

  sys_add_memory(sys, &allocator, (void*)RISCV_START_ADDRESS, prog_length);
   
  void* addr = (void*) RISCV_START_ADDRESS;
  char* it = prog;
  
  while(prog_length) {
    sys_store_byte(sys, addr, *it);
    it++;
    addr = addr + 1;
    prog_length--;
  }

  return sys;
}

define_test(riscv_add, test_print("RISCV_ADD"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    stream_t stream = stream_init;
    buffer_t buf = buffer(32, &allocator);

    const char* fp = "../test/assets/riscv/add.bin";
    
    test_check(
        test_print("Open the binary file"),
        stream_open_file(fp, "r", &stream),
        test_failure("Could not open the file at \"%s\"...", fp)
    );

    test_check(
        test_print("Read all the stream"),
        stream_exhaust(&buf, &stream, 32, &allocator),
        test_failure("Could not read the whole file content...")
    );

    test_success;
    test_teardown;
    stream_close(&stream);
    test_end;
}

define_test_chapter(
  riscv, test_print("RISCV"),
  riscv_add
)
