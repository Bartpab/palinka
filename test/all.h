#include "utils.h"

#include "test_arith.h"
#include "test_mmix.h"
#include "test_mem.h"
#include "test_buffer.h"
//#include "test_stream.h"
#include "test_string.h"

set_tests(
  string, buffer,
  arith, mmix, mem
);
