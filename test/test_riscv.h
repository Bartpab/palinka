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
tetra riscv_jal(byte rd, tetra imm) 
{
  return encode_j_type(imm) | ((rd & 0x1F) << 7) | 0b1101111;
}
tetra riscv_jalr(byte dest, byte base, tetra offset)
{
  return encode_i_type(offset) | encode_rs1(base) | encode_rd(dest) | encode_opcode(0b1100111);
}
tetra riscv_beq(byte rs1, byte rs2, tetra offset)
{
  return encode_b_type(offset) | encode_rs1(rs1) | encode_rs2(rs2) | encode_funct3(0b000) | encode_opcode(0b1100011);
}
tetra riscv_bne(byte rs1, byte rs2, tetra offset)
{
  return encode_b_type(offset) | encode_rs1(rs1) | encode_rs2(rs2) | encode_funct3(0b001) | encode_opcode(0b1100011);
}
tetra riscv_blt(byte rs1, byte rs2, tetra offset)
{
  return encode_b_type(offset) | encode_rs1(rs1) | encode_rs2(rs2) | encode_funct3(0b100) | encode_opcode(0b1100011);
}
tetra riscv_bge(byte rs1, byte rs2, tetra offset)
{
  return encode_b_type(offset) | encode_rs1(rs1) | encode_rs2(rs2) | encode_funct3(0b101) | encode_opcode(0b1100011);
}
tetra riscv_bltu(byte rs1, byte rs2, tetra offset)
{
  return encode_b_type(offset) | encode_rs1(rs1) | encode_rs2(rs2) | encode_funct3(0b110) | encode_opcode(0b1100011);
}
tetra riscv_bgeu(byte rs1, byte rs2, tetra offset)
{
  return encode_b_type(offset) | encode_rs1(rs1) | encode_rs2(rs2) | encode_funct3(0b111) | encode_opcode(0b1100011);
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

    sys_run(sys, 1);
  
    test_check(
      test_print("Check the imm encoding/decoding for J-type based instruction."),
      decode_j_type(encode_j_type(1)) == 1,
      test_failure("Expecting %d, got %d", 1, decode_j_type(encode_j_type(1)))
    );

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
define_test(riscv_jalr, test_print("RISCV_JALR"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_jalr(28, 29, 1),
      riscv_ebreak(),
      riscv_ebreak(),
      riscv_ebreak()
    };

    octa expected_reg = 8;
    octa expected_pc  = 20;
    
    system_t* sys = riscv_bootstrap((char*) &prog, 5 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 0;
    proc->regs[29] = 12;

    sys_run(sys, 1);
  
    test_check(
      test_print("Check the JALR result"),
      proc->regs[28] == expected_reg,
      test_failure("Expecting %lld, got %lld", expected_reg, proc->regs[28])
    );

    test_check(
      test_print("Check the JALR result"),
      proc->pc == expected_pc,
      test_failure("Expecting %lld, got %lld", expected_pc, proc->pc)
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_beq, test_print("RISCV_BEQ"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_beq(28, 29, 2),
      riscv_ebreak(),
      riscv_ebreak(),
      riscv_ebreak()
    };

    octa expected_pc  = 20;
    
    system_t* sys = riscv_bootstrap((char*) &prog, 5 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 12;
    proc->regs[29] = 12;

    sys_run(sys, 1);
  
    test_check(
      test_print("Check the BEQ result"),
      proc->pc == expected_pc,
      test_failure("Expecting %lld, got %lld", expected_pc, proc->pc)
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_bne, test_print("RISCV_BNE"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_bne(28, 29, 2),
      riscv_ebreak(),
      riscv_ebreak(),
      riscv_ebreak()
    };

    octa expected_pc  = 20;
    
    system_t* sys = riscv_bootstrap((char*) &prog, 5 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 12;
    proc->regs[29] = 6;

    sys_run(sys, 1);
  
    test_check(
      test_print("Check the BNE result"),
      decode_b_type(riscv_bne(28, 29, 2)) == 2,
      test_failure("Expecting %d, got %d", 2, decode_b_type(riscv_bne(28, 29, 2)))
    );

    test_check(
      test_print("Check the BNE result"),
      proc->pc == expected_pc,
      test_failure("Expecting %lld, got %lld", expected_pc, proc->pc)
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_blt, test_print("RISCV_BLT"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_blt(28, 29, 2),
      riscv_ebreak(),
      riscv_ebreak(),
      riscv_ebreak()
    };

    octa expected_pc  = 20;
    
    system_t* sys = riscv_bootstrap((char*) &prog, 5 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 6;
    proc->regs[29] = 12;

    sys_run(sys, 1);

    test_check(
      test_print("Check the BLT result"),
      proc->pc == expected_pc,
      test_failure("Expecting %lld, got %lld", expected_pc, proc->pc)
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_bge, test_print("RISCV_BGE"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_bge(28, 29, 2),
      riscv_ebreak(),
      riscv_ebreak(),
      riscv_ebreak()
    };

    octa expected_pc  = 20;
    
    system_t* sys = riscv_bootstrap((char*) &prog, 5 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 12;
    proc->regs[29] = 6;

    sys_run(sys, 1);

    test_check(
      test_print("Check the BGE result"),
      proc->pc == expected_pc,
      test_failure("Expecting %lld, got %lld", expected_pc, proc->pc)
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_bltu, test_print("RISCV_BLTU"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_bltu(28, 29, 2),
      riscv_ebreak(),
      riscv_ebreak(),
      riscv_ebreak()
    };

    octa expected_pc  = 20;
    
    system_t* sys = riscv_bootstrap((char*) &prog, 5 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 6;
    proc->regs[29] = 12;

    sys_run(sys, 1);

    test_check(
      test_print("Check the BLTU result"),
      proc->pc == expected_pc,
      test_failure("Expecting %lld, got %lld", expected_pc, proc->pc)
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_bgeu, test_print("RISCV_BGEU"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_bgeu(28, 29, 2),
      riscv_ebreak(),
      riscv_ebreak(),
      riscv_ebreak()
    };

    octa expected_pc  = 20;
    
    system_t* sys = riscv_bootstrap((char*) &prog, 5 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 12;
    proc->regs[29] = 6;

    sys_run(sys, 1);

    test_check(
      test_print("Check the BGEU result"),
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
  riscv_branching, test_print("RISCV Branching"),
  riscv_jal,
  riscv_jalr,
  riscv_beq, 
  riscv_bne, 
  riscv_blt, 
  riscv_bge, 
  riscv_bltu, 
  riscv_bgeu
)

define_test_chapter(
  riscv, test_print("RISCV"),
  riscv_auipc,
  riscv_branching,
  riscv_add,
  riscv_sub
)
