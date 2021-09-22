#include <string.h> 

#include "../lib/common/include/testing/utils.h"
#include "../lib/common/include/allocator.h"

#include "../src/system.h"
#include "../src/memory/core.h"

#include "../src/mmix/core.h"
#include "../src/mmix/instr.h"

void test_trap_handler(system_t* sys, instr_t* instr)
{
  mmix_processor_t* proc = __get_mmix_proc(sys);
  mmix_set_regv(proc, 0xD00D, 0x40);
}

void* mmix_install_memory(system_t* sys, void* vaddr, size_t len)
{
  allocator_t allocator = GLOBAL_ALLOCATOR;
  unsigned long offset = PAGE_OFFSET(vaddr);
  void* pblock = mem_alloc_managed(&sys->mem, &allocator, vaddr, len);
  return (pblock + offset);
}

system_t* mmix_bootstrap(octa* prog, size_t len)
{
  len *= sizeof(octa);

  allocator_t allocator = GLOBAL_ALLOCATOR;

  mmix_cfg_t cfg; mmix_cfg_init(&cfg);
  system_t* sys = mmix_create(&allocator, &cfg);

  assert(sys != NULL);

  unsigned long offset = PAGE_OFFSET(MMIX_START_ADDR);

  void* vaddr = (void*) MMIX_START_ADDR;
  octa* dest = (octa*) mem_alloc_managed(
    &sys->mem, 
    &allocator, 
    vaddr, 
    len
  );
  memcpy(dest + offset, prog, len);
  
  return sys;
}

define_test(mmix_mux, test_print("MUX")) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(MUX, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0b11), 
    0xC1
  );
  
  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0b11, 0b11, 0), 
    0xC2
  );
  
  mmix_set_sregv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0b11, 0, 0b11), 
    rM
  );

  e = byte_to_octa(0, 0, 0, 0, 0, 0, 0b11, 0b11);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute MUX 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(mmix_muxi, test_print("MUXI")) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(MUXI, 0xC0, 0xC1, 0b01)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0b10), 
    0xC1
  );
  
  mmix_set_sregv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0b10), 
    rM
  );

  e = byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0b11);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute MUX 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(mmix_or, test_print("OR")) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(OR, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);
  mmix_set_regv(proc, tetra_to_octa(1, 0), 0xC1);
  mmix_set_regv(proc, tetra_to_octa(0, 1), 0xC2);

  e = tetra_to_octa(1, 1);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute OR 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_ori, test_print("ORI")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(ORI, 0xC0, 0xC1, 0x01)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);
  mmix_set_regv(proc, tetra_to_octa(1, 0), 0xC1);

  e = tetra_to_octa(1, 1);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute ORI 0xC0 0xC1 0x02.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_and, test_print("AND")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(AND, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);
  mmix_set_regv(proc, tetra_to_octa(1, 0), 0xC1);
  mmix_set_regv(proc, tetra_to_octa(1, 1), 0xC2);

  e = tetra_to_octa(1, 0);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute ORI 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_andi, test_print("ANDI")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(ANDI, 0xC0, 0xC1, 0x1)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);
  mmix_set_regv(proc, tetra_to_octa(1, 1), 0xC1);

  e = tetra_to_octa(0, 1);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute ANDI 0xC0 0xC1 0x1.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_xor, test_print("XOR")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(XOR, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);
  mmix_set_regv(proc, tetra_to_octa(1, 0), 0xC1);
  mmix_set_regv(proc, tetra_to_octa(1, 1), 0xC2);

  e = tetra_to_octa(0, 1);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute XOR 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_xori, test_print("XORI")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(XORI, 0xC0, 0xC1, 0x0)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);
  mmix_set_regv(proc, tetra_to_octa(1, 0), 0xC1);

  e = tetra_to_octa(1, 0);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute XOR 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_nxor, test_print("NXOR")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(NXOR, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, ~0), 
    0xC1
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, ~0, ~0), 
    0xC2
  );

  e = byte_to_octa(~0, ~0, ~0, ~0, ~0, ~0, 0, ~0);
    
  // Execute the instruction
  sys_step(sys);

  test_print("Execute NXOR 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_nxori, test_print("NXORI")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(NXORI, 0xC0, 0xC1, 0xFF)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0), 
    0xC1
  );

  e = byte_to_octa(~0, ~0, ~0, ~0, ~0, ~0, ~0, 0);
    
  // Execute the instruction
  sys_step(sys);

  test_print("Execute XOR 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_orn, test_print("ORN")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(ORN, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, ~0), 
    0xC1
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, ~0, ~0), 
    0xC2
  );

  e = byte_to_octa(~0, ~0, ~0, ~0, ~0, ~0, 0, ~0);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute ORN 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_orni, test_print("ORNI")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(ORNI, 0xC0, 0xC1, 0xFF)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0), 
    0xC1
  );

  e = byte_to_octa(~0, ~0, ~0, ~0, ~0, ~0, ~0, 0);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute ORNI 0xC0 0xC1 0xFF.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_nor, test_print("NOR")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(NOR, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, ~0, 0), 
    0xC1
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, ~0), 
    0xC2
  );

  e = byte_to_octa(~0, ~0, ~0, ~0, ~0, ~0, ~0, 0);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute NOR 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_nori, test_print("NORI")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(NORI, 0xC0, 0xC1, 0xFF)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0), 
    0xC1
  );

  e = byte_to_octa(~0, ~0, ~0, ~0, ~0, ~0, ~0, 0);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute NORI 0xC0 0xC1 0xFF.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_nand, test_print("NAND")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(NAND, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xFF), 
    0xC1
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xFF), 
    0xC2
  );

  e = byte_to_octa(~0, ~0, ~0, ~0, ~0, ~0, ~0, 0);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute NAND 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_nandi, test_print("NANDI")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(NANDI, 0xC0, 0xC1, 0xFF)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xFF), 
    0xC1
  );

  e = byte_to_octa(~0, ~0, ~0, ~0, ~0, ~0, ~0, 0);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute NANDI 0xC0 0xC1 0xFF.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_andn, test_print("ANDN")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(ANDN, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xFF), 
    0xC1
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0), 
    0xC2
  );

  e = byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xFF);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute ANDN 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_andni, test_print("ANDNI")
) {
  char se[32], s0[32];
  octa e;
  octa prog [] = {
    __mmix_instr(ANDNI, 0xC0, 0xC1, 0x00)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xFF), 
    0xC1
  );

  e = byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xFF);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute ANDNI 0xC0 0xC1 0x00.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_bdif, test_print("BDIF")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(BDIF, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xFF), 
    0xC1
  );

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xC0), 
    0xC2
  );

  e = byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xFF - 0xC0);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute BDIF 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_bdifi, test_print("BDIFI")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(BDIFI, 0xC0, 0xC1, 0xC0)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, octa_zero, 0xC0);

  mmix_set_regv(
    proc, 
    byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xFF), 
    0xC1
  );

  e = byte_to_octa(0, 0, 0, 0, 0, 0, 0, 0xFF - 0xC0);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute BDIFI 0xC0 0xC1 0xC0.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_wdif, test_print("WDIF")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(WDIF, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    word_to_octa(0, 0, 0, 0xFFFF), 
    0xC1
  );

  mmix_set_regv(
    proc, 
    word_to_octa(0, 0, 0, 0xF00F), 
    0xC2
  );

  e = word_to_octa(0, 0, 0, 0xFFFF-0xF00F);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute WDIF 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_wdifi, test_print("WDIFI")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(WDIFI, 0xC0, 0xC1, 0xF0)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    word_to_octa(0, 0, 0, 0x0FFF), 
    0xC1
  );

  e = word_to_octa(0, 0, 0, 0x0FFF-0xF0);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute WDIFI 0xC0 0xC1 0xF0.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_tdif, test_print("TDIF")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(TDIF, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    tetra_to_octa(0, 0xFFFFFFFF), 
    0xC1
  );

  mmix_set_regv(
    proc, 
    tetra_to_octa(0, 0xFFF000FF), 
    0xC2
  );

  e = tetra_to_octa(0, 0xFFFFFFFF-0xFFF000FF);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute TDIF 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_tdifi, test_print("TDIFI")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(TDIFI, 0xC0, 0xC1, 0xFF)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    tetra_to_octa(0, 0x0FFFFFFF), 
    0xC1
  );

  e = tetra_to_octa(0, 0x0FFFFFFF-0xFF);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute TDIFI 0xC0 0xC1 0xFF.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_odif, test_print("ODIF")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(ODIF, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    tetra_to_octa(0xFFFFFFFF, 0xFFFFFFFF), 
    0xC1
  );

  mmix_set_regv(
    proc, 
    tetra_to_octa(0xFFFFF000, 0xFFFFF000), 
    0xC2
  );

  e = tetra_to_octa(0xFFFFFFFF - 0xFFFFF000, 0xFFFFFFFF-0xFFFFF000);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute ODIF 0xC0 0xC1 0xC2.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_odifi, test_print("ODIFI")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(ODIFI, 0xC0, 0xC1, 0xFF)
  };

  system_t* sys = mmix_bootstrap(prog, 1); 
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(
    proc, 
    octa_zero, 
    0xC0
  );

  mmix_set_regv(
    proc, 
    tetra_to_octa(0x0FFFFFFF, 0xFFFFFFFF), 
    0xC1
  );

  e = tetra_to_octa(0x0FFFFFFF, 0xFFFFFFFF-0xFF);
  
  // Execute the instruction
  sys_step(sys);

  test_print("Execute ODIFI 0xC0 0xC1 0xFF.\n");

  octa_str(e, se, 32), octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end; 
}

define_test(
  mmix_cmp, test_print("CMP")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(CMP, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(30), 0xC1);
  mmix_set_regv(proc, int_to_octa(-5), 0xC2);

  e = int_to_octa(1);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute CMP 0xC0 0xC1 0xC2.\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  mmix_restart(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(-5), 0xC1);
  mmix_set_regv(proc, int_to_octa(30), 0xC2);

  e = int_to_octa(-1);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute CMP 0xC0 0xC1 0xC2.\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  mmix_restart(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(30), 0xC1);
  mmix_set_regv(proc, int_to_octa(30), 0xC2);

  e = int_to_octa(0);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute CMP 0xC0 0xC1 0xC2.\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_cmpu, test_print("CMPU")
) {
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(CMPU, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, octa_zero, 0xC1);
  mmix_set_regv(proc, octa_uint_max, 0xC2);

  e = int_to_octa(-1);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute CMPU 0xC0 0xC1 0xC2.\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  mmix_restart(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, octa_uint_max, 0xC1);
  mmix_set_regv(proc, octa_zero, 0xC2);

  e = int_to_octa(1);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute CMPU 0xC0 0xC1 0xC2.\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  mmix_restart(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(30), 0xC1);
  mmix_set_regv(proc, int_to_octa(30), 0xC2);

  e = int_to_octa(0);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute CMPU 0xC0 0xC1 0xC2.\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_add, test_print("ADD")
){
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(ADD, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(30), 0xC1);
  mmix_set_regv(proc, int_to_octa(5), 0xC2);

  e = int_to_octa(35);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute ADD 0xC0 0xC1 0xC2 (no overflow).\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  /////////////
  // Restart //
  /////////////
  
  mmix_restart(sys);

  mmix_set_regv(proc, int_to_octa(-10), 0xC0);
  mmix_set_regv(proc, octa_int_max, 0xC1);
  mmix_set_regv(proc, int_to_octa(1), 0xC2);

  e = sign_bit;
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute ADD 0xC0 0xC1 0xC2 (overflow).\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_check(
    test_print("Expecting rA to have V_BIT flag."),
    ol(mmix_get_sregv(proc, rA)) & V_BIT,
    test_failure("V_BIT is not set in rA.")
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_addi, test_print("ADDI")
){
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(ADDI, 0xC0, 0xC1, 0x01)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(30), 0xC1);

  e = int_to_octa(31);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute ADD 0xC0 0xC1 0x01 (no overflow).\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  /////////////
  // Restart //
  /////////////
  
  mmix_restart(sys);

  mmix_set_regv(proc, int_to_octa(-10), 0xC0);
  mmix_set_regv(proc, octa_int_max, 0xC1);

  e = sign_bit;
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute ADD 0xC0 0xC1 0x01 (overflow).\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_check(
    test_print("Expecting rA to have V_BIT flag."),
    ol(mmix_get_sregv(proc, rA)) & V_BIT,
    test_failure("V_BIT is not set in rA.")
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_neg, test_print("NEG")
){
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(NEG, 0xC0, 0x00, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(5), 0xC2);

  e = int_to_octa(-5);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute NEG 0xC0 0x00 0xC2.\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_negi, test_print("NEGI")
){
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(NEGI, 0xC0, 0x00, 0x05)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);

  e = int_to_octa(-5);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute NEG 0xC0 0x00 0xC2.\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_sub, test_print("SUB")
){
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(SUB, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(30), 0xC1);
  mmix_set_regv(proc, int_to_octa(5), 0xC2);

  e = int_to_octa(25);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute SUB 0xC0 0xC1 0xC2.\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  mmix_restart(sys);

  mmix_set_regv(proc, int_to_octa(-10), 0xC0);
  mmix_set_regv(proc, sign_bit, 0xC1);
  mmix_set_regv(proc, int_to_octa(1), 0xC2);

  e = octa_int_max;

  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute SUB 0xC0 0xC1 0xC2 (overflow).\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_check(
    test_print("Expecting rA to have V_BIT flag."),
    ol(mmix_get_sregv(proc, rA)) & V_BIT,
    test_failure("V_BIT is not set in rA.")
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_subi, test_print("SUBI")
){
  char se[32], s0[32];
  octa e;

  octa prog [] = {
    __mmix_instr(SUBI, 0xC0, 0xC1, 0x01)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(30), 0xC1);

  e = int_to_octa(29);
  
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute SUBI 0xC0 0xC1 0x01.\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  mmix_restart(sys);

  mmix_set_regv(proc, int_to_octa(-10), 0xC0);
  mmix_set_regv(proc, sign_bit, 0xC1);

  e = octa_int_max;

  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(e, se, 32);

  test_print("Execute SUBI 0xC0 0xC1 0xC2 (overflow).\n");

  test_check(
    test_print("Expecting r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  )

  test_check(
    test_print("Expecting rA to have V_BIT flag."),
    ol(mmix_get_sregv(proc, rA)) & V_BIT,
    test_failure("V_BIT is not set in rA.")
  )

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(
  mmix_mul, test_print("MUL")
){

  char sx[32], se[32];
  octa x, e;
  octa prog [] = {
    __mmix_instr(MUL, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(-30), 0xC0);
  mmix_set_regv(proc, int_to_octa(6), 0xC1);
  mmix_set_regv(proc, int_to_octa(-5), 0xC2);

  // Execute the instruction
  sys_step(sys);

  e = int_to_octa(-30);
  x = mmix_get_regv(proc, 0xC0);
  octa_str(x, sx, 32), octa_str(e, se, 32);

  test_check(
    test_print("Execute MUL 0XC0 0XC1 0XC2; Expecting r[0xC0] == -30"),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
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
  char sx[32], se[32];
  octa x, e;

  octa prog [] = {
    __mmix_instr(MUL, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(6), 0xC1);
  mmix_set_regv(proc, int_to_octa(-5), 0xC2);

  // Execute the instruction
  sys_step(sys);

  e = int_to_octa(-30);
  x = mmix_get_regv(proc, 0xC0);
  octa_str(x, sx, 32), octa_str(e, se, 32);

  test_check(
    test_print("Execute MULI 0XC0 0XC1 0XC2; Expecint reg[0xC0] == -30"),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
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
  char sx[32], se[32];
  octa x, e;

  octa prog [] = {
    __mmix_instr(MUL, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(6), 0xC1);
  mmix_set_regv(proc, int_to_octa(5), 0xC2);

  // Execute the instruction
  sys_step(sys);

  e = int_to_octa(30);
  x = mmix_get_regv(proc, 0xC0);
  octa_str(x, sx, 32), octa_str(e, se, 32);

  test_check(
    test_print("Execute MULU 0XC0 0XC1 0XC2; Expecint reg[0xC0] == 30"),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
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
  char sx[32], se[32];
  octa x, e;

  octa prog [] = {
    __mmix_instr(MULUI, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(6), 0xC1);
  mmix_set_regv(proc, int_to_octa(5), 0xC2);

  // Execute the instruction
  sys_step(sys);

  e = int_to_octa(30);
  x = mmix_get_regv(proc, 0xC0);
  
  octa_str(x, sx, 32), octa_str(e, se, 32);

  test_check(
    test_print("Execute MULUI 0XC0 0XC1 0XC2; Expecint reg[0xC0] == 30"),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
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
  char sx[32], se[32];
  octa x, e;

  octa prog [] = {
    __mmix_instr(DIV, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, 0,  0xC0);
  mmix_set_regv(proc, 65, 0xC1);
  mmix_set_regv(proc, 2,  0xC2);
  
  // Execute the instruction
  sys_step(sys);

  x = mmix_get_regv(proc, 0xC0);

  e = int_to_octa(32);
  octa_str(x, sx, 32), octa_str(e, se, 32);

  test_check(
    test_print("Execute DIV 0XC0 0XC1 0XC2 \\w reg[0XC1] == 65, reg[0xC2] == 2; Expecting reg[0xC0] == 32"),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  )

  x = mmix_get_sregv(proc, rR);
  e = int_to_octa(1);
  octa_str(x, sx, 32), octa_str(e, se, 32);

  test_check(
    test_print("Execute DIV 0XC0 0XC1 0XC2 \\w reg[0XC1] == 65, reg[0xC2] == 2; Expecting aux == 1"),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
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
  char se[32], s0[32];
  octa prog [] = {
    __mmix_instr(DIVI, 0xC0, 0xC1, 0x02)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, 0, 0xC0);
  mmix_set_regv(proc, 65, 0xC1);

  // Execute the instruction
  sys_step(sys);
  
  test_print("Execute r[%#x] = r[%#x] / %#x [0x02]\n", 0xC0, 0xC01, 0x02);

  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);
  octa_str(int_to_octa(32), se, 32);

  test_check(
    test_print("Expecting r[0xC0] == %d", 32),
    mmix_get_regv(proc, 0xC0) == int_to_octa(32),
    test_failure("Expecting %s, got %s", se, s0)
  )

  octa_str(mmix_get_sregv(proc, rR), s0, 32);
  octa_str(int_to_octa(1), se, 32);

  test_check(
    test_print("Expecting rR == 1"),
    mmix_get_sregv(proc, rR) == int_to_octa(1),
    test_failure("Expecting %s, got %s", se, s0)
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
  char s[5][32];

  octa prog [] = {
    __mmix_instr(DIVU, 0xC0, 0xC1, 0xC2)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_sregv(proc, int_to_octa(1), rD);
  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(1), 0xC1);
  mmix_set_regv(proc, tetra_to_octa(1 << 31, 0), 0xC2);
 
  // Execute the instruction
  sys_step(sys);

  octa_str(mmix_get_sregv(proc, rD),  s[0], 32);
  octa_str(mmix_get_regv(proc, 0xC0), s[1], 32);
  octa_str(mmix_get_regv(proc, 0xC1), s[2], 32);
  octa_str(mmix_get_regv(proc, 0xC2), s[3], 32);
  octa_str(mmix_get_sregv(proc, rR),  s[4], 32);

  test_print("Set reg[rD] to %s\n",   s[0]);
  test_print("Set reg[0xC1] to %s\n", s[2]);
  test_print("Set reg[0xC2] to %s\n", s[3]);

  test_print("Execute DIVUI 0XC0 0XC1 0XC2\n")
  test_check(
    test_print("reg[0xC0] should be 2"),
    mmix_get_regv(proc, 0xC0) == int_to_octa(2),
    test_failure(
      "Expecting %d, got %s", 2, s[1]
    )
  )

  test_check(
    test_print("reg[rR] should be 1"),
    mmix_get_sregv(proc, rR) == int_to_octa(1),
    test_failure("Expecting %d, got %s", 1, s[4])
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

  char s[5][32];

  octa prog [] = {
    __mmix_instr(DIVUI, 0xC0, 0xC1, 0x02)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_sregv(proc, int_to_octa(1), rD);
  mmix_set_regv(proc, int_to_octa(0), 0xC0);
  mmix_set_regv(proc, int_to_octa(1), 0xC1);

  octa e = tetra_to_octa(1 << 31, 0);
  test_print("Execute DIVUI 0xC0 0xC1 0x02\n")

  sys_step(sys);

  octa_str(mmix_get_sregv(proc, rD),  s[0], 32);
  octa_str(mmix_get_regv(proc, 0xC0), s[1], 32);
  octa_str(mmix_get_regv(proc, 0xC1), s[2], 32);
  octa_str(e, s[3], 32);
  octa_str(mmix_get_sregv(proc, rR), s[4], 32);

  test_check(
    test_print("r[0xC0] should be %s", s[3]),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", s[3], s[1])
  )

  e = uint_to_octa(1);
  octa_str(e, s[3], 32);
  
  test_check(
    test_print("r[rR] should be %s", s[3]),
    mmix_get_sregv(proc, rR) == e,
    test_failure("Expecting %s, got %s", s[3], s[4])
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

  char s0[32], se[32];
  octa e;
  octa prog [] = {
    __mmix_instr(FSQRT, 0xC0, ROUND_OFF, 0xC1)
  };

  system_t* sys = mmix_bootstrap(prog, 1);  
  
  // Setup the init state.
  mmix_processor_t* proc = __get_mmix_proc(sys);

  e = double_to_octa(7.0);
  mmix_set_regv(proc, double_to_octa(49.0), 0xC1);

  // Execute the step.
  sys_step(sys);

  test_print("Execute FSQRT $C0 $00 $C1\n");
  
  octa_str(e, se, 32);
  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("Expecting reg[$C0] == 7.0"),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
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
  char s[32];
  octa prog [] = {
    __mmix_instr(TRAP, 0xC0, 0x00, 0x00)
  };  

  system_t* sys = mmix_bootstrap(prog, 1);
  mmix_processor_t* proc = __get_mmix_proc(sys);

  // Register the interrupt handler
  mmix_set_interrupt_handler(sys, 0xC0, test_trap_handler);

  // Execute on step
  sys_step(sys);

  octa_str(mmix_get_regv(proc, 0x40), s, 32);

  // Get the result.
  test_check(
    test_print("The trap handler set the 0x40 reg value to 0xD00D."),
    mmix_get_regv(proc, 0x40) == 0xD00D,
    test_failure("Expecting %#x, got %s", 0xD00D, s)
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

  mmix_set_regv(proc, 0x0, 0xC0);
  mmix_set_regv(proc, 0x10, 0xC1);
  mmix_set_regv(proc, 0x05, 0xC2);

  // Install memory and define an initial state
  void* vbase = (void*) (0x10 + 0x05);
  octa* stored_value = (octa*) mmix_install_memory(sys, vbase, sizeof(octa));

  assert(stored_value);
  *stored_value = byte_to_octa(0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11);

  e = tetra_to_octa(0, 0x11);

  // Execute one step
  sys_step(sys);

  octa_str(e, se, 32);
  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
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

  mmix_set_regv(proc, 0x0, 0xC0);
  mmix_set_regv(proc, 0x10, 0xC1);
  mmix_set_regv(proc, 0x05, 0xC2);

  // Install memory and define an initial state
  void* vbase = (void*) (0x10 + 0x05);
  octa* stored_value = (octa*) mmix_install_memory(sys, vbase, sizeof(octa));
  assert(stored_value);

  *stored_value = word_to_octa(0x8877, 0x6655, 0x4433, 0x2211);

  e = word_to_octa(0, 0, 0, 0x2211);

  // Execute one step
  sys_step(sys);

  octa_str(e, se, 32);
  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
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

  mmix_set_regv(proc, 0x0, 0xC0);
  mmix_set_regv(proc, 0x10, 0xC1);
  mmix_set_regv(proc, 0x05, 0xC2);

  // Install memory and define an initial state
  void* vbase = (void*) (0x10 + 0x05);
  octa* stored_value = (octa*) mmix_install_memory(sys, vbase, sizeof(octa));

  assert(stored_value);
  *stored_value = tetra_to_octa(0x88776655, 0x44332211);

  e = tetra_to_octa(0, 0x44332211);

  // Execute one step
  sys_step(sys);

  octa_str(e, se, 32);
  octa_str(mmix_get_regv(proc, 0xC0), s0, 32);

  test_check(
    test_print("r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
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

  mmix_set_regv(proc, 0x0, 0xC0);
  mmix_set_regv(proc, 0x10, 0xC1);
  mmix_set_regv(proc, 0x05, 0xC2);

  // Install memory and define an initial state
  void* vbase = (void*) (0x10 + 0x05);
  octa* stored_value = (octa*) mmix_install_memory(sys, vbase, sizeof(octa));

  assert(stored_value);
  *stored_value = tetra_to_octa(0x88776655, 0x44332211);

  e = tetra_to_octa(0x88776655, 0x44332211);

  // Execute one step
  sys_step(sys);

  octa_str(e, se, 32), octa_str(proc->g[0xC0], s0, 32);

  test_check(
    test_print("r[0xC0] == %s", se),
    mmix_get_regv(proc, 0xC0) == e,
    test_failure("Expecting %s, got %s", se, s0)
  );

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(mmix_stb, test_print("STB")) 
{
  octa prog [] = {
    __mmix_instr(STB, 0xC0, 0xC1, 0xC2)
  };  
  
  char se[32], s0[32];
  octa e;

  system_t* sys = mmix_bootstrap(prog, 1);
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, 0x10, 0xC1);
  mmix_set_regv(proc, 0x05, 0xC2);

  // Install memory and define an initial state
  void* vbase = (void*) (0x10 + 0x05);
  octa* stored_value = (octa*) mmix_install_memory(sys, vbase, sizeof(octa));

  assert(stored_value != NULL);

  *stored_value = tetra_to_octa(0, 0x11);

  mmix_set_regv(
    proc,
    byte_to_octa(0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11),
    0xC0
  );

  e = tetra_to_octa(0, 0x11);

  // Execute one step
  sys_step(sys);

  octa_str(e, se, 32), octa_str(*stored_value, s0, 32);
  test_check(
    test_print("r[0xC0] == %s", se),
    *stored_value == e,
    test_failure("Expecting %s, got %s", se, s0)
  );

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(mmix_stw, test_print("STW")) 
{
  octa prog [] = {
    __mmix_instr(STW, 0xC0, 0xC1, 0xC2)
  };  
  
  char se[32], s0[32];
  octa e;

  system_t* sys = mmix_bootstrap(prog, 1);
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, 0x10, 0xC1);
  mmix_set_regv(proc, 0x05, 0xC2);

  // Install memory and define an initial state
  void* vbase = (void*) (0x10 + 0x05);
  octa* stored_value = (octa*) mmix_install_memory(sys, vbase, sizeof(octa));

  assert(stored_value);
  *stored_value = tetra_to_octa(0, 0);

  mmix_set_regv(
    proc,
    word_to_octa(0x8877, 0x6655, 0x4433, 0x2211),
    0xC0
  );

  e = word_to_octa(0, 0, 0, 0x2211);

  // Execute one step
  sys_step(sys);

  octa_str(e, se, 32), octa_str(*stored_value, s0, 32);

  test_check(
    test_print("r[0xC0] == %s", se),
    *stored_value == e,
    test_failure("Expecting %s, got %s", se, s0)
  );

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(mmix_stt, test_print("STT")) 
{
  octa prog [] = {
    __mmix_instr(STT, 0xC0, 0xC1, 0xC2)
  };  
  
  char se[32], s0[32];
  octa e;

  system_t* sys = mmix_bootstrap(prog, 1);
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, 0x10, 0xC1);
  mmix_set_regv(proc, 0x05, 0xC2);

  // Install memory and define an initial state
  void* vbase = (void*) (0x10 + 0x05);
  octa* stored_value = (octa*) mmix_install_memory(sys, vbase, sizeof(octa));

  assert(stored_value);
  *stored_value = tetra_to_octa(0, 0);

  mmix_set_regv(
    proc,
    tetra_to_octa(0x88776655, 0x44332211),
    0xC0
  );

  e = tetra_to_octa(0, 0x44332211);

  // Execute one step
  sys_step(sys);

  octa_str(e, se, 32), octa_str(*stored_value, s0, 32);

  test_check(
    test_print("r[0xC0] == %s", se),
    *stored_value == e,
    test_failure("Expecting %s, got %s", se, s0)
  );

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test(mmix_sto, test_print("STO")) 
{
  octa prog [] = {
    __mmix_instr(STO, 0xC0, 0xC1, 0xC2)
  };  
  
  char se[32], s0[32];
  octa e;

  system_t* sys = mmix_bootstrap(prog, 1);
  mmix_processor_t* proc = __get_mmix_proc(sys);

  mmix_set_regv(proc, 0x10, 0xC1);
  mmix_set_regv(proc, 0x05, 0xC2);

  // Install memory and define an initial state
  void* vbase = (void*) (0x10 + 0x05);
  octa* stored_value = (octa*) mmix_install_memory(sys, vbase, sizeof(octa));

  assert(stored_value);
  *stored_value = tetra_to_octa(0, 0);

  mmix_set_regv(
    proc,
    tetra_to_octa(0x88776655, 0x44332211),
    0xC0
  );
  
  e = tetra_to_octa(0x88776655, 0x44332211);

  // Execute one step
  sys_step(sys);

  octa_str(e, se, 32), octa_str(*stored_value, s0, 32);

  test_check(
    test_print("r[0xC0] == %s", se),
    *stored_value == e,
    test_failure("Expecting %s, got %s", se, s0)
  );

  test_success;  
  test_teardown {
    sys_delete(sys);
  }
  test_end;
}

define_test_chapter(
  mmix_arith_1, test_print("MMIX ALU #1 - Integer"), 
  mmix_add, mmix_addi,
  mmix_neg, mmix_negi,
  mmix_sub, mmix_subi,
  mmix_mul, mmix_muli,
  mmix_mulu, mmix_mului,
  mmix_div, mmix_divi,
  mmix_divu, mmix_divui
)

define_test_chapter(
  mmix_arith_2, test_print("MMIX ALU #2 - Integer"), 
  mmix_cmp,
  mmix_cmpu
)


define_test_chapter(
  mmix_arith_3, test_print("MMIX ALU #2 - Floating"),
  mmix_fsqrt
) 

define_test_chapter(
  mmix_arith_4, test_print("MMIX ALU #3 - Binary"),
  mmix_mux, mmix_muxi,
  mmix_or, mmix_ori,
  mmix_and, mmix_andi,
  mmix_xor, mmix_xori,
  mmix_nxor, mmix_nxori,
  mmix_orn, mmix_orni,
  mmix_nor, mmix_nori,
  mmix_nand, mmix_nandi,
  mmix_andn, mmix_andni
)

define_test_chapter(
  mmix_arith_5, test_print("MMIX ALU #4 - Binary Diff"),
  mmix_bdif, mmix_bdifi,
  mmix_wdif, mmix_wdifi,
  mmix_tdif, mmix_tdifi,
  mmix_odif, mmix_odifi
)

define_test_chapter(
  mmix_arith, test_print("MMIX ALU"),
  mmix_arith_1,
  mmix_arith_2,
  mmix_arith_3,
  mmix_arith_4,
  mmix_arith_5
)

define_test_chapter(
  mmix_flow_control, test_print("MMIX Flow Control"),
  mmix_trap
)

define_test_chapter(
  mmix_load_store, test_print("MMIX Store/Load"),
  mmix_ldb,
  mmix_ldw,
  mmix_ldt,
  mmix_ldo,
  mmix_stb,
  mmix_stw,
  mmix_stt,
  mmix_sto
)

define_test_chapter(
  mmix, test_print("MMIX"),
  mmix_arith,
  mmix_flow_control,
  mmix_load_store
)
