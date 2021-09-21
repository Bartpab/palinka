#include "../lib/common/include/testing/utils.h"

#include "../lib/common/test/test_alu.h"
#include "../lib/common/test/test_buffer.h"
#include "../lib/common/test/test_string.h"
#include "../lib/common/test/test_lexer.h"

#include "test_mmix.h"
#include "test_mem.h"

set_tests(
  string, buffer, lexer,
  arith, mem
  //, mmix
);
