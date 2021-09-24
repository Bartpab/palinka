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
  cfg.frequency    = 100000000; // 50 MHz
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

tetra riscv_nop()
{
  return 0;
}

tetra riscv_auipc(byte rd, tetra imm)
{
  return ((imm & 0x7ffff) << 12 ) | ((rd & 0x1F) << 7) | 23;
}

tetra riscv_jal(byte rd, tetra imm) {
  tetra tmp = 0;
  tmp |= (imm & 0x3ff) << 9;
  tmp |= ((imm >> 11) & 1) << 8; 
  tmp |= ((imm >> 12) & 0xFF);
  tmp |= ((imm >> 20) & 1) << 19;
  return (tmp << 12) |((rd & 0x1F) << 7) | 111;
}

tetra riscv_add(byte rd, byte rs1, byte rs2)
{
  tetra t = 0;
  t |= ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((rd & 0x1F) << 7) | 0x33;
  return t;
}
tetra riscv_sub(byte rd, byte rs1, byte rs2)
{
  tetra t = 0;
  t |= (0b0100000 << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((rd & 0x1F) << 7) | 0x33;
  return t;
}

tetra riscv_ebreak()
{
  return 1048691;
}

define_test(riscv_auipc, test_print("RISCV_AUIPC"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_auipc(28, 1),
      riscv_ebreak()
    };

    octa expected = 8 + (1 << 12);
    
    system_t* sys = riscv_bootstrap((char*) &prog, 3 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 0;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check the AUIPC result"),
      proc->regs[28] == expected,
      test_failure("Expecting %lld, got %lld", expected, proc->regs[28])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}

define_test(riscv_jal, test_print("RISCV_JAL"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_jal(28, 2),
      riscv_ebreak(),
      riscv_ebreak()
    };

    octa expected_reg = 8;
    octa expected_pc  = 16;
    
    system_t* sys = riscv_bootstrap((char*) &prog, 4 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 0;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check the JAL result"),
      proc->regs[28] == expected_reg,
      test_failure("Expecting %lld, got %lld", expected_reg, proc->regs[28])
    );

    test_check(
      test_print("Check the JAL result"),
      proc->pc == expected_pc,
      test_failure("Expecting %lld, got %lld", expected_pc, proc->pc)
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}


define_test(riscv_add, test_print("RISCV_ADD"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_add(28, 29, 30),
      riscv_ebreak()
    };

    octa expected = 15;
    
    system_t* sys = riscv_bootstrap((char*) &prog, 8, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 0;
    proc->regs[29] = 10;
    proc->regs[30] = 5;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld + %lld = %lld", proc->regs[29], proc->regs[30], expected),
      proc->regs[28] == expected,
      test_failure("Expecting %lld, got %lld", expected, proc->regs[28])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}

define_test(riscv_sub, test_print("RISCV_SUB"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    tetra prog[] = {
      riscv_sub(28, 29, 30),
      riscv_ebreak()
    };
    octa expected = 5;
    
    system_t* sys = riscv_bootstrap((char*) &prog, 8, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 0;
    proc->regs[29] = 10;
    proc->regs[30] = 5;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld + %lld = %lld", proc->regs[29], proc->regs[30], expected),
      proc->regs[28] == expected,
      test_failure("Expecting %lld, got %lld", expected, proc->regs[28])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}

define_test_chapter(
  riscv, test_print("RISCV"),
  riscv_auipc,
  riscv_jal,
  riscv_add,
  riscv_sub
)
