#include <string.h> 

#include "../lib/common/include/testing/utils.h"
#include "../lib/common/include/allocator.h"
#include "../lib/common/include/stream/buffer.h"

#include "../src/system.h"
#include "../src/riscv.h"

system_t* riscv_bootstrap(byte* prog, size_t prog_length, size_t heap_memory)
{
  allocator_t allocator = GLOBAL_ALLOCATOR;
  riscv_processor_cfg_t cfg;

  cfg.boot_address = 0;
  cfg.frequency    = 1000; // 1 kHz

  system_t* sys = riscv_new(&allocator, &cfg);
  riscv_processor_t* proc = __get_riscv_proc(sys);
   
  octa addr = 0x00;
  byte* it = prog;
  
  // Write directly in the CPU cache (don't need to install memory system)
  while(prog_length) 
  {
    data_cache_write(&proc->l1, (octa)(uintptr_t)(addr), *it, 0);
    it++;
    addr++;
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
tetra riscv_lb(byte rs1, byte rd, tetra offset)
{
  return encode_i_type(offset) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b000) | encode_opcode(0b0000011);
}
tetra riscv_lh(byte rs1, byte rd, tetra offset)
{
  return encode_i_type(offset) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b001) | encode_opcode(0b0000011);
}
tetra riscv_lw(byte rs1, byte rd, tetra offset)
{
  return encode_i_type(offset) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b010) | encode_opcode(0b0000011);
}
tetra riscv_lbu(byte rs1, byte rd, tetra offset)
{
  return encode_i_type(offset) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b100) | encode_opcode(0b0000011);
}
tetra riscv_lhu(byte rs1, byte rd, tetra offset)
{
  return encode_i_type(offset) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b101) | encode_opcode(0b0000011);
}
tetra riscv_sb(byte base, byte src, tetra offset) 
{
  return encode_s_type(offset) | encode_rs1(base) | encode_rs2(src) | encode_funct3(0b000) | encode_opcode(0b0100011);
}
tetra riscv_sh(byte rs1, byte rs2, tetra offset) 
{
  return encode_s_type(offset) | encode_rs1(rs1) | encode_rs2(rs2) | encode_funct3(0b001) | encode_opcode(0b0100011);
}
tetra riscv_sw(byte rs1, byte rs2, tetra offset) 
{
  return encode_s_type(offset) | encode_rs1(rs1) | encode_rs2(rs2) | encode_funct3(0b010) | encode_opcode(0b0100011);
}
tetra riscv_sd(byte rs1, byte rs2, tetra offset) 
{
  return encode_s_type(offset) | encode_rs1(rs1) | encode_rs2(rs2) | encode_funct3(0b011) | encode_opcode(0b0100011);
}
tetra riscv_addi(byte rs1, byte rd, tetra arg)
{
  return encode_i_type(arg) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b000) | encode_opcode(0b0010011);
}
tetra riscv_slti(byte rs1, byte rd, tetra arg)
{
  return encode_i_type(arg) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b010) | encode_opcode(0b0010011);
}
tetra riscv_sltiu(byte rs1, byte rd, tetra arg)
{
  return encode_i_type(arg) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b011) | encode_opcode(0b0010011);
}
tetra riscv_xori(byte rs1, byte rd, tetra arg)
{
  return encode_i_type(arg) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b100) | encode_opcode(0b0010011);
}
tetra riscv_ori(byte rs1, byte rd, tetra arg)
{
  return encode_i_type(arg) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b110) | encode_opcode(0b0010011);
}
tetra riscv_andi(byte rs1, byte rd, tetra arg)
{
  return encode_i_type(arg) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b111) | encode_opcode(0b0010011);
}
tetra riscv_slli(byte rs1, byte rd, byte shamt)
{
  return encode_s_type(0) | encode_rs1(rs1) | encode_rs2(shamt) | encode_funct3(0b001) | encode_rd(rd) | encode_opcode(0b0010011);
}
tetra riscv_srli(byte rs1, byte rd, byte shamt)
{
  return encode_s_type(0) | encode_rs1(rs1) | encode_rs2(shamt) | encode_funct3(0b101) | encode_rd(rd) | encode_opcode(0b0010011);
}
tetra riscv_srai(byte rs1, byte rd, byte shamt)
{
  return encode_funct7(0b0100000) | encode_rs1(rs1) | encode_rs2(shamt) | encode_funct3(0b101) | encode_rd(rd) | encode_opcode(0b0010011);
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
tetra riscv_sll(byte rd, byte rs1, byte rs2)
{
  return encode_funct7(0) | encode_rs1(rs1) | encode_rs2(rs2) | encode_rd(rd) | encode_funct3(0b001) | encode_opcode(0b0110011);
}
tetra riscv_slt(byte rd, byte rs1, byte rs2)
{
  return encode_funct7(0) | encode_rs1(rs1) | encode_rs2(rs2) | encode_rd(rd) | encode_funct3(0b010) | encode_opcode(0b0110011);
}
tetra riscv_sltu(byte rd, byte rs1, byte rs2)
{
  return encode_funct7(0) | encode_rs1(rs1) | encode_rs2(rs2) | encode_rd(rd) | encode_funct3(0b011) | encode_opcode(0b0110011);
}
tetra riscv_xor(byte rd, byte rs1, byte rs2)
{
  return encode_funct7(0) | encode_rs1(rs1) | encode_rs2(rs2) | encode_rd(rd) | encode_funct3(0b100) | encode_opcode(0b0110011);
}
tetra riscv_srl(byte rd, byte rs1, byte rs2)
{
  return encode_funct7(0) | encode_rs1(rs1) | encode_rs2(rs2) | encode_rd(rd) | encode_funct3(0b101) | encode_opcode(0b0110011);
}
tetra riscv_sra(byte rd, byte rs1, byte rs2)
{
  return encode_funct7(0b0100000) | encode_rs1(rs1) | encode_rs2(rs2) | encode_rd(rd) | encode_funct3(0b101) | encode_opcode(0b0110011);
}
tetra riscv_or(byte rd, byte rs1, byte rs2)
{
  return encode_funct7(0) | encode_rs1(rs1) | encode_rs2(rs2) | encode_rd(rd) | encode_funct3(0b110) | encode_opcode(0b0110011);
}
tetra riscv_and(byte rd, byte rs1, byte rs2)
{
  return encode_funct7(0) | encode_rs1(rs1) | encode_rs2(rs2) | encode_rd(rd) | encode_funct3(0b111) | encode_opcode(0b0110011);
}
tetra riscv_fence(byte fm, byte pred, byte succ, byte rs1, byte rd)
{
  return encode_s_type(((fm & 0xf) << 8) | ((pred & 0xf) << 4) | (succ & 0xf)) | encode_rs1(rs1) | encode_rd(rd) | encode_opcode(0b0001111);
}
tetra riscv_ecall()
{
  return 115;
}
tetra riscv_ebreak()
{
  return 1048691;
}
tetra riscv_csrrw(byte rd, byte rs1, tetra csr)
{
  return encode_i_type(csr) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b001) | encode_opcode(0b1110011);
}
tetra riscv_csrrs(byte rd, byte rs1, tetra csr)
{
  return encode_i_type(csr) | encode_rs1(rs1) | encode_rd(rd) | encode_funct3(0b010) | encode_opcode(0b1110011);
}

define_test(riscv_auipc, test_print("RISCV_AUIPC"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_auipc(28, 1),
      riscv_ebreak()
    };

    octa expected = octa_plus(int_to_octa(4), octa_left_shift(int_to_octa(1), 12), 0);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
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
      riscv_auipc(29, 0),
      riscv_ebreak()
    };

    octa expected_reg = int_to_octa(8);
    octa expected_pc  = int_to_octa(12);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = octa_zero;

    sys_run(sys, 1);

    test_check(
      test_print("Check the value of the register x28"),
      octa_eq(proc->regs[28], expected_reg),
      test_failure("Expecting %lld, got %lld", expected_reg, proc->regs[28])
    );

    test_check(
      test_print("Check the value of the register x29"),
      octa_eq(proc->regs[29], expected_pc),
      test_failure("Expecting %lld, got %lld", expected_pc, proc->regs[29])
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
      riscv_auipc(30, 0),
      riscv_ebreak()
    };

    octa expected_reg = int_to_octa(8);
    octa expected_pc  = int_to_octa(16);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = int_to_octa(12);
    proc->regs[30] = octa_zero;

    sys_run(sys, 1);
  
    test_check(
      test_print("Check the value of x28"),
      octa_eq(proc->regs[28], expected_reg),
      test_failure("Expecting %lld, got %lld", expected_reg, proc->regs[28])
    );

    test_check(
      test_print("Check the value of x29"),
      octa_eq(proc->regs[30], expected_pc),
      test_failure("Expecting %lld, got %lld", expected_pc, proc->regs[30])
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
      riscv_auipc(30, 0),
      riscv_ebreak()
    };

    octa expected_pc = int_to_octa(16);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = int_to_octa(12);
    proc->regs[30] = octa_zero;

    sys_run(sys, 1);
  
    test_check(
      test_print("Check the BEQ result"),
      octa_eq(proc->regs[30], expected_pc),
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
      riscv_auipc(30, 0),
      riscv_ebreak()
    };

    octa expected_pc  = int_to_octa(16);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = int_to_octa(6);
    proc->regs[30] = octa_zero;

    sys_run(sys, 1);
  
    test_check(
      test_print("Check the BNE result"),
      decode_b_type(riscv_bne(28, 29, 2)) == 2,
      test_failure("Expecting %d, got %d", 2, decode_b_type(riscv_bne(28, 29, 2)))
    );

    test_check(
      test_print("Check the BNE result"),
      octa_eq(proc->regs[30], expected_pc),
      test_failure("Expecting %lld, got %lld", expected_pc, proc->regs[30])
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
      riscv_auipc(30, 0),
      riscv_ebreak()
    };

    octa expected_pc = int_to_octa(16);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(6);
    proc->regs[29] = int_to_octa(12);
    proc->regs[30] = octa_zero;

    sys_run(sys, 1);

    test_check(
      test_print("Check the BLT result"),
      octa_eq(proc->regs[30], expected_pc),
      test_failure("Expecting %lld, got %lld", expected_pc, proc->regs[30])
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
      riscv_auipc(30, 0),
      riscv_ebreak()
    };

    octa expected_pc  = int_to_octa(16);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 5 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = int_to_octa(6);
    proc->regs[30] = octa_zero;

    sys_run(sys, 1);

    test_check(
      test_print("Check the BGE result"),
      octa_eq(proc->regs[30], expected_pc),
      test_failure("Expecting %lld, got %lld", expected_pc, proc->regs[30])
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
      riscv_auipc(30, 0),
      riscv_ebreak()
    };

    octa expected_pc  = int_to_octa(16);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(6);
    proc->regs[29] = int_to_octa(12);
    proc->regs[30] = octa_zero;

    sys_run(sys, 1);

    test_check(
      test_print("Check the BLTU result"),
      octa_eq(proc->regs[30], expected_pc),
      test_failure("Expecting %lld, got %lld", expected_pc, proc->regs[30])
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
      riscv_auipc(30, 0),
      riscv_ebreak()
    };

    octa expected_pc  = int_to_octa(16);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = int_to_octa(6);
    proc->regs[30] = octa_zero;

    sys_run(sys, 1);

    test_check(
      test_print("Check the BGEU result"),
      octa_eq(proc->regs[30], expected_pc),
      test_failure("Expecting %lld, got %lld", expected_pc, proc->regs[30])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_lb, test_print("RISCV_LB"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_lb(28, 29, 0),
      riscv_ebreak(),
      0xD00D
    };

    octa expected  = int_to_octa(0x0D);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = octa_zero;

    sys_run(sys, 1);

    test_check(
      test_print("Check the LB result"),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_lh, test_print("RISCV_LH"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_lh(28, 29, 0),
      riscv_ebreak(),
      0x1234D00D
    };

    octa expected  = int_to_octa(0xD00D);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = octa_zero;

    sys_run(sys, 100);

    test_check(
      test_print("Check the LH result"),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_lw, test_print("RISCV_LW"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_lw(28, 29, 0),
      riscv_ebreak(),
      0x1234D00D,
      0x56789
    };

    octa expected  = int_to_octa(0x1234D00D);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 5 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = octa_zero;

    sys_run(sys, 1);

    test_check(
      test_print("Check the LH result"),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_lbu, test_print("RISCV_LBU"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_lbu(28, 29, 0),
      riscv_ebreak(),
      0xD00D
    };

    octa expected  = int_to_octa(0x0D);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 4 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = octa_zero;

    sys_run(sys, 1);

    test_check(
      test_print("Check the LBU result"),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_lhu, test_print("RISCV_LHU"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_lhu(28, 29, 0),
      riscv_ebreak(),
      0x1234D00D
    };

    octa expected  = int_to_octa(0xD00D);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 4 * 4, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = octa_zero;

    sys_run(sys, 1);

    test_check(
      test_print("Check the LH result"),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_sb, test_print("RISCV_SB"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_sb(28, 29, 0),
      riscv_ebreak(),
      0
    };

    octa expected  = int_to_octa(0x0D);
    byte result = octa_zero;
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = int_to_octa(0xD00D);

    sys_run(sys, 1);

    test_check(
      test_print("Try to load memory value"),
      data_cache_read(&proc->l1, 12, &result, 0),
      test_failure("Failed to load memory value")
    );

    test_check(
      test_print("Check the SB result"),
      octa_eq(result, expected),
      test_failure("Expecting %lld, got %d", expected, result)
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_sh, test_print("RISCV_SH"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_sh(28, 29, 0),
      riscv_ebreak(),
      0
    };

    octa expected  = int_to_octa(0xD00D);
    octa result = octa_zero;
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = int_to_octa(0xD00D);

    sys_run(sys, 1);

    test_check(
      test_print("Try to load memory value"),
      data_cache_read_word(&proc->l1, 12, (word*) &result, 0),
      test_failure("Failed to load memory value")
    );

    test_check(
      test_print("Check the SH result"),
      octa_eq(result, expected),
      test_failure("Expecting %lld, got %lld", expected, result)
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_sw, test_print("RISCV_SW"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_sw(28, 29, 0),
      riscv_ebreak(),
      0
    };

    octa expected  = int_to_octa(0x1234D00D);
    octa result = octa_zero;
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = int_to_octa(0x1234D00D);

    sys_run(sys, 1);

    test_check(
      test_print("Try to load memory value"),
      data_cache_read_tetra(&proc->l1, 12, (tetra*) &result, 0),
      test_failure("Failed to load memory value")
    );

    test_check(
      test_print("Check the SW result"),
      octa_eq(result, expected),
      test_failure("Expecting %lld, got %lld", expected, result)
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_sd, test_print("RISCV_SD"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_nop(),
      riscv_sd(28, 29, 0),
      riscv_ebreak(),
      0,
      0
    };

    octa expected  = ll_int_to_octa(0x1122334455667788);
    octa result = octa_zero;
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(12);
    proc->regs[29] = ll_int_to_octa(0x1122334455667788);

    sys_run(sys, 1);

    test_check(
      test_print("Try to load memory value"),
      data_cache_read_octa(&proc->l1, 12, &result, 0),
      test_failure("Failed to load memory value")
    );

    test_check(
      test_print("Check the SD result"),
      octa_eq(result, expected),
      test_failure("Expecting %#llx, got %#llx", expected, result)
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_addi, test_print("RISCV_ADDI"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_addi(28, 29, 10),
      riscv_ebreak()
    };

    octa expected = int_to_octa(15);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(5);
    proc->regs[29] = octa_zero;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld + %d = %lld", proc->regs[28], 10, expected),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_slti, test_print("RISCV_SLTI"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_slti(28, 29, 1),
      riscv_ebreak()
    };

    octa expected = 1;
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = -octa_int_max;
    proc->regs[29] = octa_zero;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld < %d = %lld", proc->regs[28], 1, expected),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_sltiu, test_print("RISCV_SLTIU"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_sltiu(28, 29, 1),
      riscv_ebreak()
    };

    octa expected = 0;
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_uint_max;
    proc->regs[29] = octa_zero;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld < %d = %lld", proc->regs[28], 1, expected),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_xori, test_print("RISCV_XORI"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_xori(28, 29, 1),
      riscv_ebreak()
    };

    octa expected = int_to_octa(1);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = octa_zero;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld ^ %d = %lld", proc->regs[28], 1, expected),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_ori, test_print("RISCV_XORI"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_ori(28, 29, 1),
      riscv_ebreak()
    };

    octa expected = 1;
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = octa_zero;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld | %d = %lld", proc->regs[28], 1, expected),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_andi, test_print("RISCV_ANDI"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_andi(28, 29, 1),
      riscv_ebreak()
    };

    octa expected = int_to_octa(1);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(1);
    proc->regs[29] = octa_zero;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld & %d = %lld", proc->regs[28], 1, expected),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_slli, test_print("RISCV_SLLI"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_slli(28, 29, 1),
      riscv_ebreak()
    };

    octa expected = octa_left_shift(int_to_octa(1), 1);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(1);
    proc->regs[29] = octa_zero;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld << %d = %lld", proc->regs[28], 1, expected),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_srli, test_print("RISCV_SRLI"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_srli(28, 29, 1),
      riscv_ebreak()
    };

    octa expected = octa_right_shift(int_to_octa(2), 1, 0);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(2);
    proc->regs[29] = octa_zero;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld << %d = %lld", proc->regs[28], 1, expected),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_srai, test_print("RISCV_SRAI"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;

    tetra prog[] = {
      riscv_srai(28, 29, 1),
      riscv_ebreak()
    };

    octa expected = octa_right_shift(int_to_octa(2), 1, 0);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = int_to_octa(2);
    proc->regs[29] = octa_zero;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld << %d = %lld", proc->regs[28], 1, expected),
      octa_eq(proc->regs[29], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[29])
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

    octa expected = int_to_octa(15);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = int_to_octa(10);
    proc->regs[30] = int_to_octa(5);

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld + %lld = %lld", proc->regs[29], proc->regs[30], expected),
      octa_eq(proc->regs[28], expected),
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
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 8, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = 0;
    proc->regs[29] = 10;
    proc->regs[30] = 5;

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld + %lld = %lld", proc->regs[29], proc->regs[30], expected),
      octa_eq(proc->regs[28], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[28])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_slt, test_print("RISCV_SLT"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    tetra prog[] = {
      riscv_slt(28, 29, 30),
      riscv_ebreak()
    };
    octa expected = int_to_octa(0);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 8, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = int_to_octa(2);
    proc->regs[30] = int_to_octa(1);

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld < %lld = %lld", proc->regs[29], proc->regs[30], expected),
      octa_eq(proc->regs[28], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[28])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_sltu, test_print("RISCV_SLTU"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    tetra prog[] = {
      riscv_sltu(28, 29, 30),
      riscv_ebreak()
    };
    octa expected = int_to_octa(1);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 8, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = int_to_octa(1);
    proc->regs[30] = octa_compl(octa_int_max);

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld < %lld = %lld", proc->regs[29], proc->regs[30], expected),
      octa_eq(proc->regs[28], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[28])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_xor, test_print("RISCV_XOR"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    tetra prog[] = {
      riscv_xor(28, 29, 30),
      riscv_ebreak()
    };
    octa expected = int_to_octa(1);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 8, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = int_to_octa(1);
    proc->regs[30] = octa_compl(0);

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld ^ %lld = %lld", proc->regs[29], proc->regs[30], expected),
      octa_eq(proc->regs[28], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[28])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_srl, test_print("RISCV_SRL"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    tetra prog[] = {
      riscv_srl(28, 29, 30),
      riscv_ebreak()
    };
    octa expected = int_to_octa(1);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 8, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = int_to_octa(2);
    proc->regs[30] = int_to_octa(1);

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld >> %lld = %lld", proc->regs[29], proc->regs[30], expected),
      octa_eq(proc->regs[28], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[28])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_sll, test_print("RISCV_SLL"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    tetra prog[] = {
      riscv_sll(28, 29, 30),
      riscv_ebreak()
    };
    octa expected = int_to_octa(4);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 8, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = int_to_octa(2);
    proc->regs[30] = int_to_octa(1);

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld << %lld = %lld", proc->regs[29], proc->regs[30], expected),
      octa_eq(proc->regs[28], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[28])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_or, test_print("RISCV_OR"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    tetra prog[] = {
      riscv_or(28, 29, 30),
      riscv_ebreak()
    };
    octa expected = int_to_octa(3);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 8, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = int_to_octa(2);
    proc->regs[30] = int_to_octa(1);

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld | %lld = %lld", proc->regs[29], proc->regs[30], expected),
      octa_eq(proc->regs[28], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[28])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_and, test_print("RISCV_AND"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    tetra prog[] = {
      riscv_and(28, 29, 30),
      riscv_ebreak()
    };
    octa expected = int_to_octa(2);
    
    system_t* sys = riscv_bootstrap((byte*) &prog, 8, 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = int_to_octa(2);
    proc->regs[30] = int_to_octa(3);

    sys_run(sys, 100);
  
    test_check(
      test_print("Check that %lld & %lld = %lld", proc->regs[29], proc->regs[30], expected),
      octa_eq(proc->regs[28], expected),
      test_failure("Expecting %lld, got %lld", expected, proc->regs[28])
    );

    test_success;
    test_teardown;
    sys_delete(sys, &allocator);
    test_end;
}
define_test(riscv_csrrw, test_print("RISCV_CSRRW"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    tetra prog[] = {
      riscv_csrrw(28, 29, 30),
      riscv_ebreak()
    };
    octa expected[2] = {int_to_octa(6), int_to_octa(12)};
    
    system_t* sys = riscv_bootstrap((byte*) &prog, sizeof(prog), 0);
    riscv_processor_t* proc = __get_riscv_proc(sys);

    proc->regs[28] = octa_zero;
    proc->regs[29] = int_to_octa(6);
    proc->csrs[30] = int_to_octa(12);

    test_print("CSRRW rd(x28), rs1(x29), csr(30)\n");

    sys_run(sys, 100);

    test_check(
      test_print("Check that csr30 := x29"),
      octa_eq(proc->csrs[30], expected[0]),
      test_failure("Expecting %lld, got %lld", expected[0], proc->csrs[30])
    );

    test_check(
      test_print("Check that x28 := csr30"),
      octa_eq(proc->regs[28], expected[1]),
      test_failure("Expecting %lld, got %lld", expected[1], proc->regs[28])
    );

    sys_delete(sys, &allocator);

    tetra prog_2[] = {
      riscv_csrrw(0, 29, 30),
      riscv_ebreak()      
    };

    sys = riscv_bootstrap((byte*) &prog_2, sizeof(prog_2), 0);
    proc = __get_riscv_proc(sys);

    proc->regs[29] = int_to_octa(6);
    proc->csrs[30] = int_to_octa(12);

    test_print("CSRRW rd(x0), rs1(x29), csr(30)\n");

    sys_run(sys, 100); 

    expected[0] = int_to_octa(6);
    expected[1] = octa_zero;

    test_check(
      test_print("Check that csr30 := x29"),
      octa_eq(proc->csrs[30], expected[0]),
      test_failure("Expecting %lld, got %lld", expected[0], proc->csrs[30])
    );

    test_check(
      test_print("Check that x0 != csr30"),
      octa_eq(proc->regs[0], expected[1]),
      test_failure("Expecting %lld, got %lld", expected[1], proc->regs[0])
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
  riscv_memory, test_print("RISCV Memory"),
  riscv_lb, riscv_lh, riscv_lw, riscv_lbu, riscv_lhu,
  riscv_sb, riscv_sh, riscv_sw, riscv_sd
)

define_test_chapter(
  riscv_alu_1, test_print("RISCV ALU #1"),
  riscv_addi, riscv_slti, riscv_sltiu,
  riscv_xori, riscv_ori, riscv_andi,
  riscv_slli, riscv_srli, riscv_srai,
  riscv_add, riscv_sub
)

define_test_chapter(
  riscv_alu_2, test_print("RISCV ALU #2"),
  riscv_slt, riscv_sltu, riscv_xor,
  riscv_srl, riscv_sll, riscv_or, 
  riscv_and
)

define_test_chapter(
  riscv_alu, test_print("RISCV ALU"),
  riscv_alu_1, riscv_alu_2
)

define_test_chapter(
  riscv_csr, test_print("RISCV CSR"),
  riscv_csrrw
)

define_test_chapter(
  riscv, test_print("RISCV"),
  riscv_auipc,
  riscv_branching,
  riscv_memory,
  riscv_alu,
  riscv_csr
)
