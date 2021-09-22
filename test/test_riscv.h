#include <string.h> 

#include "../lib/common/include/testing/utils.h"
#include "../lib/common/include/allocator.h"
#include "../lib/common/include/stream/buffer.h"

#include "../src/system.h"
#include "../src/memory/core.h"

#include "../src/riscv/core.h"

system_t* riscv_bootstrap(char* prog, size_t prog_length, size_t heap_memory)
{
  allocator_t allocator = GLOBAL_ALLOCATOR;
  riscv_processor_cfg_t cfg;

  cfg.boot_address = 0;
  cfg.frequency    = 500;
  cfg.memory_size  = prog_length + heap_memory;

  system_t* sys = riscv_new(&allocator, &cfg);

  sys_add_memory(sys, &allocator, (void*) 0x00, cfg.memory_size);
   
  void* addr = (void*) 0x00;
  char* it = prog;
  
  while(prog_length) 
  {
    sys_store_byte(sys, addr, *it);
    it++;
    addr = addr + 1;
    prog_length--;
  }

  return sys;
}

tetra riscv_add(byte rd, byte rs1, byte rs2)
{
  tetra t = 0;
  t |= ((rs2 & 0x1F) << 15) | ((rs1 & 0x1F) << 15) | ((rd & 0x1F) << 7) | 0x51;
  return t;
}

define_test(riscv_add, test_print("RISCV_ADD"))
{
    tetra encoded = riscv_add(3, 1, 2);
    tetra stored = 0;
    octa expected = 15;
    
    system_t* sys = riscv_bootstrap((char*) &encoded, 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[3] = 0;
    proc->regs[1] = 10;
    proc->regs[2] = 5;

    sys_load_tetra(sys, (void*) 0x0, &stored);

    riscv_step(sys);
    
    test_print("%d\n", encoded == stored);
    test_print("%d\n", proc->current_control.decoded.opcode);

    test_check(
      test_print("Check that %lld + %lld = %lld", proc->regs[1], proc->regs[2], expected),
      proc->regs[3] == expected,
      test_failure("Expecting %lld, got %lld", expected, proc->regs[3])
    );

    test_success;
    test_teardown;
    sys_delete(sys);
    test_end;
}

define_test_chapter(
  riscv, test_print("RISCV"),
  riscv_add
)
