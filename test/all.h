#include "../lib/common/include/testing/utils.h"

#include "../lib/common/test/test_alu.h"
#include "../lib/common/test/test_buffer.h"
#include "../lib/common/test/test_string.h"
#include "../lib/common/test/test_lexer.h"
#include "../lib/common/test/test_transaction.h"

#include "test_riscv.h"
#include "test_system.h"
#include "test_memory.h"

set_tests(
  transaction
  //,riscv, system, string, buffer, 
  //arith, memory
  //, mmix
  //, lexer
);
