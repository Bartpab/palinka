#include <string.h> 

#include "utils.h"
#include "../src/allocator.h"
#include "../src/sys.h"
#include "../src/mem.h"

#include "../src/mmix/api.h"
#include "../src/mmix/instr.h"

void test_trap_handler(system_t* sys, instr_t* instr)
{
  mmix_processor_t* proc = __get_mmix_proc(sys);
  proc->reg[0x40] = 0xD00D;
}

void* mmix_install_memory(system_t* sys, void* vaddr, size_t len)
{
  allocator_t allocator = GLOBAL_ALLOCATOR;
  unsigned long offset = PAGE_OFFSET(vaddr);
  void* pblock = mem_alloc_managed(&sys->mem, &allocator, vaddr, len);
  return (pblock + offset);
}

system_t* mmix_bootstrap(octa* prog, size_t len){
  len *= sizeof(octa);

  allocator_t allocator = GLOBAL_ALLOCATOR;

  system_t* sys = mmix_create(&allocator);

  assert(sys != NULL);

  unsigned long offset = PAGE_OFFSET(MMIX_START_ADDR);

  void* vaddr = (void*) MMIX_START_ADDR;
  octa* dest = (octa*) mem_alloc_managed(&sys->mem, &allocator, vaddr, len);
  memcpy(dest + offset, prog, len);
  
  return sys;
}

define_test(
  mmix_mul, test_print("MUL")
){

  octa prog [] = {
    __mmix_instr(MUL, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[0xC0] = int_to_octa(0);
  proc->reg[0xC1] = int_to_octa(6);
  proc->reg[0xC2] = int_to_octa(-5);

  // Execute the instruction
  mmix_step(sys);

  test_check(
    test_print("Execute MUL 0XC0 0XC1 0XC2; Expecint reg[0xC0] == -30"),
    proc->reg[0xC0] == int_to_octa(-30),
    test_failure("Expecting %d, got %lu", -30, proc->reg[0xC0])
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_muli, test_print("MULI")
){

  octa prog [] = {
    __mmix_instr(MUL, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[0xC0] = int_to_octa(0);
  proc->reg[0xC1] = int_to_octa(-6);
  proc->reg[0xC2] = int_to_octa(5);

  // Execute the instruction
  mmix_step(sys);

  test_check(
    test_print("Execute MULI 0XC0 0XC1 0XC2; Expecint reg[0xC0] == -30"),
    proc->reg[0xC0] == -30,
    test_failure("Expecting %d, got %lu", -30, proc->reg[0xC0])
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_mulu, test_print("MULU")
){

  octa prog [] = {
    __mmix_instr(MUL, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[0xC0] = 0;
  proc->reg[0xC1] = 6;
  proc->reg[0xC2] = 5;

  // Execute the instruction
  mmix_step(sys);

  test_check(
    test_print("Execute MULU 0XC0 0XC1 0XC2; Expecint reg[0xC0] == 30"),
    proc->reg[0xC0] == 30,
    test_failure("Expecting %d, got %lu", 30, proc->reg[0xC0])
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_mului, test_print("MULUI")
){

  octa prog [] = {
    __mmix_instr(MUL, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[0xC0] = 0;
  proc->reg[0xC1] = 6;
  proc->reg[0xC2] = 5;

  // Execute the instruction
  mmix_step(sys);

  test_check(
    test_print("Execute MULUI 0XC0 0XC1 0XC2; Expecint reg[0xC0] == 30"),
    proc->reg[0xC0] == 30,
    test_failure("Expecting %d, got %lu", 30, proc->reg[0xC0])
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_div, test_print("DIV")
){

  octa prog [] = {
    __mmix_instr(DIV, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[0xC0] = 0;
  proc->reg[0xC1] = 65;
  proc->reg[0xC2] = 2;

  // Execute the instruction
  mmix_step(sys);

  test_check(
    test_print("Execute DIV 0XC0 0XC1 0XC2 \\w reg[0XC1] == 65, reg[0xC2] == 2; Expecting reg[0xC0] == 32"),
    proc->reg[0xC0] == 32,
    test_failure("Expecting %d, got %lu", 32, proc->reg[0xC0])
  )

  test_check(
    test_print("Execute DIV 0XC0 0XC1 0XC2 \\w reg[0XC1] == 65, reg[0xC2] == 2; Expecting aux == 1"),
    proc->reg[rR] == 1,
    test_failure("Expecting %d, got %lu", 1, proc->reg[rR])
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_divi, test_print("DIVI")
){

  octa prog [] = {
    __mmix_instr(DIVI, 0xC0, 0xC1, 0x02)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[0xC0] = 0;
  proc->reg[0xC1] = 65;

  // Execute the instruction
  mmix_step(sys);
  
  test_print("Execute r[%#x] = r[%#x] / %#x [0x02]\n", 0xC0, 0xC01, 0x02);

  test_check(
    test_print("Expecting r[0xC0] == %d", 32),
    proc->reg[0xC0] == 32,
    test_failure("Expecting %d, got %lu", 32, proc->reg[0xC0])
  )

  test_check(
    test_print("Expecting rR == 1"),
    proc->reg[rR] == 1,
    test_failure("Expecting %d, got %lu", 1, proc->reg[rR])
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_divu, test_print("DIVU")
){

  octa prog [] = {
    __mmix_instr(DIVU, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[rD]   = 1;
  proc->reg[0xC0] = 0;
  proc->reg[0xC1] = 1;
  proc->reg[0xC2] = (octa)(1) << 63;

  // Execute the instruction
  mmix_step(sys);

  test_print("Set reg[rD] to %lu\n", proc->reg[rD]);
  test_print("Set reg[0xC0] to %lu\n", proc->reg[0xC0]);
  test_print("Set reg[0xC1] to %lu\n", proc->reg[0xC1]);
  test_print("Set reg[0xC2] to %lu\n", proc->reg[0xC2]);

  test_print("Execute DIVUI 0XC0 0XC1 0XC2\n")
  test_check(
    test_print("reg[0xC0] should be 2"),
    proc->reg[0xC0] == 2,
    test_failure("Expecting %d, got %lu", 2, proc->reg[0xC0])
  )

  test_check(
    test_print("reg[rR] should be 1"),
    proc->reg[rR] == 1,
    test_failure("Expecting %d, got %lu", 1, proc->reg[rR])
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_divui, test_print("DIVUI")
){

  char sd[32], s0[32], s1[32], se[32];

  octa prog [] = {
    __mmix_instr(DIVUI, 0xC0, 0xC1, 0x02)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[rD]   = tetra_to_octa(0, 1);
  proc->reg[0xC0] = int_to_octa(0);
  proc->reg[0xC1] = tetra_to_octa(0, 1);
  octa e = tetra_to_octa(1 << 31, 0);
  
  octa_str(e, se, 32);
  octa_str(proc->reg[rD], sd, 32), octa_str(proc->reg[0xC0], s0, 32), octa_str(proc->reg[0xC1], s1, 32);
  
  test_print("Set reg[rD] to %s\n", sd);
  test_print("Set reg[0xC0] to %s\n", s0);
  test_print("Set reg[0xC1] to %s\n", s1);
  test_print("Set Z to %d\n", 0x02);

  test_print("Execute DIVUI 0xC0 0xC1 0x02\n")

  mmix_step(sys);

  octa_str(e, se, 32);
  octa_str(proc->reg[rD], sd, 32), octa_str(proc->reg[0xC0], s0, 32), octa_str(proc->reg[0xC1], s1, 32);

  test_check(
    test_print("r[0xC0] should be %s", se),
    proc->reg[0xC0] == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  e = uint_to_octa(1);
  octa_str(e, se, 32);
  test_check(
    test_print("r[rR] should be %s", se),
    proc->reg[rR] == e,
    test_failure("Expecting %s, got %lu", se, proc->reg[rR])
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_fsqrt, test_print("FSQRT")
){

  octa prog [] = {
    __mmix_instr(FSQRT, 0xC0, ROUND_OFF, 0xC1)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);
  
  // Store it as a double, then cast it to an octa properly.
  double dy = 49.0;
  proc->reg[0xC1] = double_to_octa(dy);

  // Execute the step.
  mmix_step(sys);

  // Cast back to a double.
  double dx = octa_to_double(proc->reg[0xC0]);

  test_check(
    test_print("Execute FSQRT $C0 $00 $C1, \\w reg[$C1] = 49.0; Expecting reg[$C0] == 7.0"),
    dx == 7.0,
    test_failure("Expecting %f, got %f", 10.0, dx)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_trap, test_print("TRAP")
){
  octa prog [] = {
    __mmix_instr(TRAP, 0xC0, 0x00, 0x00)
  };  

  system_t* sys = mmix_bootstrap(prog, 1);
  mmix_processor_t* proc = __get_mmix_proc(sys);

  // Register the interrupt handler
  mmix_set_interrupt_handler(sys, 0xC0, test_trap_handler);

  // Execute on step
  mmix_step(sys);

  // Get the result.
  test_check(
    test_print("The trap handler set the 0x40 reg value to 0xD00D."),
    proc->reg[0x40] == 0xD00D,
    test_failure("Expecting %#x, got %#lx", 0xD00D, proc->reg[0x40])
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(mmix_ldb, test_print("LDB")) 
{
  octa prog [] = {
    __mmix_instr(LDB, 0xC0, 0xC1, 0xC2)
  };  
  
  char se[32], s0[32];
  octa e;

  system_t* sys = mmix_bootstrap(prog, 1);
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[0xC1] = 0x10;
  proc->reg[0xC2] = 0x05;

  // Install memory and define an initial state
  octa* stored_value = (octa*) mmix_install_memory(sys, (void*)(0x10 + 0x05), sizeof(octa));

  assert(stored_value);
  *stored_value = byte_to_octa(0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11);

  proc->reg[0xC0] = 0;

  e = tetra_to_octa(0, 0x11);

  // Execute one step
  mmix_step(sys);

  octa_str(e, se, 32), octa_str(proc->reg[0xC0], s0, 32);

  test_check(
    test_print("r[0xC0] == %s", se),
    proc->reg[0xC0] == e,
    test_failure("Expecting %s, got %s", se, s0)
  );

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(mmix_ldw, test_print("LDW")) 
{
  octa prog [] = {
    __mmix_instr(LDW, 0xC0, 0xC1, 0xC2)
  };  
  
  char se[32], s0[32];
  octa e;

  system_t* sys = mmix_bootstrap(prog, 1);
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[0xC1] = 0x10;
  proc->reg[0xC2] = 0x05;

  // Install memory and define an initial state
  octa* stored_value = (octa*) mmix_install_memory(sys, (void*)(0x10 + 0x05), sizeof(octa));

  assert(stored_value);
  *stored_value = word_to_octa(0x8877, 0x6655, 0x4433, 0x2211);

  proc->reg[0xC0] = 0;

  e = word_to_octa(0, 0, 0, 0x2211);

  // Execute one step
  mmix_step(sys);

  octa_str(e, se, 32), octa_str(proc->reg[0xC0], s0, 32);

  test_check(
    test_print("r[0xC0] == %s", se),
    proc->reg[0xC0] == e,
    test_failure("Expecting %s, got %s", se, s0)
  );

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(mmix_ldt, test_print("LDT")) 
{
  octa prog [] = {
    __mmix_instr(LDT, 0xC0, 0xC1, 0xC2)
  };  
  
  char se[32], s0[32];
  octa e;

  system_t* sys = mmix_bootstrap(prog, 1);
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[0xC1] = 0x10;
  proc->reg[0xC2] = 0x05;

  // Install memory and define an initial state
  octa* stored_value = (octa*) mmix_install_memory(sys, (void*)(0x10 + 0x05), sizeof(octa));

  assert(stored_value);
  *stored_value = tetra_to_octa(0x88776655, 0x44332211);

  proc->reg[0xC0] = 0;

  e = tetra_to_octa(0, 0x44332211);

  // Execute one step
  mmix_step(sys);

  octa_str(e, se, 32), octa_str(proc->reg[0xC0], s0, 32);

  test_check(
    test_print("r[0xC0] == %s", se),
    proc->reg[0xC0] == e,
    test_failure("Expecting %s, got %s", se, s0)
  );

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(mmix_ldo, test_print("LDO")) 
{
  octa prog [] = {
    __mmix_instr(LDO, 0xC0, 0xC1, 0xC2)
  };  
  
  char se[32], s0[32];
  octa e;

  system_t* sys = mmix_bootstrap(prog, 1);
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->reg[0xC1] = 0x10;
  proc->reg[0xC2] = 0x05;

  // Install memory and define an initial state
  octa* stored_value = (octa*) mmix_install_memory(sys, (void*)(0x10 + 0x05), sizeof(octa));

  assert(stored_value);
  *stored_value = tetra_to_octa(0x88776655, 0x44332211);

  proc->reg[0xC0] = 0;

  e = tetra_to_octa(0x88776655, 0x44332211);

  // Execute one step
  mmix_step(sys);

  octa_str(e, se, 32), octa_str(proc->reg[0xC0], s0, 32);

  test_check(
    test_print("r[0xC0] == %s", se),
    proc->reg[0xC0] == e,
    test_failure("Expecting %s, got %s", se, s0)
  );

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}


define_test_chapter(
  mmix_arith_1, test_print("MMIX ALU #1"), 
  mmix_mul, 
  mmix_muli,
  mmix_mulu,
  mmix_mului,
  mmix_div,
  mmix_divi,
  mmix_divu,
  mmix_divui,
  mmix_fsqrt
)

define_test_chapter(
  mmix_arith, test_print("MMIX ALU"),
  mmix_arith_1 
)

define_test_chapter(
  mmix_flow_control, test_print("MMIX Flow Control"),
  mmix_trap
)

define_test_chapter(
  mmix_load_store, test_print("MMIX Flow Control"),
  mmix_ldb,
  mmix_ldw,
  mmix_ldt,
  mmix_ldo
)

define_test_chapter(
  mmix, test_print("MMIX"),
  mmix_arith,
  mmix_flow_control,
  mmix_load_store
)
