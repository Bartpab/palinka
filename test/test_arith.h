#include "utils.h"
#include "../src/arith.h"


define_test(
  octa_bdif, test_print("Octa byte diff")
) {
  char sx[32], sy[32], sz[32], se[32];
  octa x, y, z, e;
  
  y = byte_to_octa(0, 0, 0, 0, 0, 0, 0x10, 0x09);
  z = byte_to_octa(0, 0, 0, 0, 0, 0, 0x09, 0x10);
  e = byte_to_octa(0, 0, 0, 0, 0, 0, 0x10 - 0x09, 0);
  
  x = octa_bdif(y, z);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("bdif %s - %s = %s", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  )
  
  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_wdif, test_print("Octa word diff")
) {
  char sx[32], sy[32], sz[32], se[32];
  octa x, y, z, e;
  
  y = word_to_octa(0x0, 0x0, 0x10, 0x09);
  z = word_to_octa(0x0, 0x0, 0x09, 0x10);
  e = word_to_octa(0x0, 0x0, 0x10 - 0x09, 0x0);
  
  x = octa_wdif(y, z);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("wdif %s - %s = %s", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  )
  
  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_tdif, test_print("Octa tetra diff")
) {
  char sx[32], sy[32], sz[32], se[32];
  octa x, y, z, e;
  
  y = tetra_to_octa(0x10, 0x9);
  z = tetra_to_octa(0x09, 0x10);
  e = tetra_to_octa(0x10 - 0x09, 0x0);
  
  x = octa_tdif(y, z);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("tdif %s - %s = %s", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  )
  
  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_odif, test_print("Octa tetra diff")
) {
  char sx[32], sy[32], sz[32], se[32];
  octa x, y, z, e;
  
  y = int_to_octa(5);
  z = int_to_octa(10);
  e = int_to_octa(0);
  
  x = octa_tdif(y, z);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("odif %s - %s = %s", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  )
  
  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_right_shift, test_print("Octa right shift")
) {
  char sx[32], sy[32], se[32];
  octa x, y, e;
  
  y = int_to_octa(4);
  e = int_to_octa(2);

  x = octa_right_shift(y, 1, 0);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(e, se, 32);

  test_check(
    test_print("%s >> 1 = %s", sy, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  );

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_left_shift, test_print("Octa left shift")
) {
  char sx[32], sy[32], se[32];
  octa x, y, e;
  
  y = int_to_octa(2);
  e = int_to_octa(4);

  x = octa_left_shift(y, 1);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(e, se, 32);

  test_check(
    test_print("%s >> 1 = %s", sy, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  );

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_plus, test_print("Octa plus")
) {
  char sx[32], sy[32], sz[32], se[32];
  octa x, y, z, e;
  bool overflow;

  test_print("Perform an addition without overflow.");

  overflow = false;
  y = 10;
  z = 5;
  e = 15;

  x = octa_plus(y, z, &overflow);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("%s + %s = %s", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  );

  test_check(
    test_print("Must not have overflowed."),
    !overflow,
    test_failure("Expecting %d, got %d", 0, overflow)
  );

  test_print("Perform a signed addition with an overflow.\n");

  overflow = false;
  y = INT64_MAX_VALUE;
  z = 1;
  e = (octa)(1) << 63;

  x = octa_plus(y, z, &overflow);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);
  
  test_check(
    test_print("%s + %s = %s (Overflow)", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sz)    
  )

  test_check(
    test_print("Must have overflowed."),
    overflow,
    test_failure("Expecting %d, got %d", true, overflow)
  );

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_minus, test_print("Octa minus")
) {
  char sx[32], sy[32], sz[32], se[32];
  octa x, y, z, e;
  bool overflow;

  test_print("Perform a subtraction without overflow.\n");

  overflow = false;
  y = 0;
  z = 5;
  e = -5;
  
  x = octa_minus(y, z, &overflow);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);


  test_check(
    test_print("%s - %s = %s", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  );

  test_check(
    test_print("Must not have overflowed."),
    !overflow,
    test_failure("Expecting %d, got %d", false, overflow)
  );

  test_print("Perform a substraction with overflow.\n");
  overflow = false;
  
  y = -INT64_MAX_VALUE - 1;
  z = 1;
  e = 0x7fffffffffffffff; // Overflowed value

  x = octa_minus(y, z, &overflow);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);
 
  test_check(
    test_print("%s - %s = %s (Overflow)", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)    
  )

  test_check(
    test_print("Must have overflowed."),
    overflow,
    test_failure("Expecting %d, got %d", true, overflow)
  );

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_mult, test_print("Octa mult")
) {
  char sx[32], sy[32], sz[32], se[32];
  octa x, y, z, e, aux;
  bool overflow;

  y = 10;
  z = 10;
  e = 100;
  overflow = false;

  test_print("Perform a multiplication without overflow.\n");

  x = octa_mult(y, z, &aux, &overflow);

  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("%s * %s = %s", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)    
  )

  test_print("Perform a multiplication with overflow.\n");

  y = UINT64_MAX_VALUE;
  z = 2;
  e = UINT64_MAX_VALUE - 1;
  overflow = false;
  aux = 0;

  x = octa_mult(y, z, &aux, &overflow);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("%s * %s = %s (Overflow)", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)    
  )

  test_check(
    test_print("Must have overflowed."),
    overflow,
    test_failure("Expecting %d, got %d", true, overflow)
  );

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_div, test_print("Octa unsigned long division")
) {
  char sx[32], sz[32], sq[32], sr[32], se[32];

  octa z, q, r, e;
  
  hexadeca x = {1, 01};
  z = (octa)(1) << 63;

  hexadeca_str(x, sx, 32), octa_str(z, sz, 32);
  
  test_print("Perform euclidean division: %s = q * %s + r\n", sx, sz);
  q = octa_div(x, z, &r), octa_str(q, sq, 32), octa_str(r, sr, 32);
  e = uint_to_octa(2), octa_str(e, se, 32);
  
  test_check(
    test_print("q == %s", se),
    q == e,
    test_failure("Expecting %s, got %s", se, sq)
  );

  e = int_to_octa(1), octa_str(e, se, 32);
  test_check(
    test_print("rem == %s", se),
    r == e,
    test_failure("Expecting %s, got %s", se, sr)
  );

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_signed_div, test_print("Octa signed long division")
) {
  char sx[32], sz[32], sq[32], sr[32], se[32];
  octa z, q, r, e;
  
  hexadeca x = {sign_bit | 1, 1};
  z = (octa)(1) << 62;

  hexadeca_str(x, sx, 32), octa_str(z, sz, 32);
  
  test_print("Perform euclidean division: %s = q * %s + r\n", sx, sz);
  q = octa_signed_div(x, z, &r);
  octa_str(q, sq, 32), octa_str(r, sr, 32);

  e = int_to_octa(-4), octa_str(e, se, 32);
  
  test_check(
    test_print("q == %s", sq),
    q == int_to_octa(-4),
    test_failure("Expecting %s, got %s", se, sq)
  );
  
  e = int_to_octa(-1), octa_str(e, se, 32);
  
  test_check(
    test_print("r == %s", se),
    r == e,
    test_failure("Expecting %s, got %s", se, sr)
  );

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_fdiv, test_print("Octa 64-bit float division.")
) {
  octa exceptions;

  octa y  = double_to_octa(64.0);
  octa z  = double_to_octa(32.0);
  octa er = double_to_octa(2.0);

  octa x = octa_fdiv(y, z, &exceptions);

  test_check(
    test_print("%#lx / %#lx == %#lx", y, z, er),
    x == double_to_octa(2.0),
    test_print("Expecting %#lx, got %#lx", er, x)
  )

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_fmult, test_print("Octa 64-bit float multiplication")
) {
  char sx[32], sy[32], sz[32], se[32];

  octa x, y, z, e;
  octa exceptions;

  y = double_to_octa(32.0), z = double_to_octa(2.0), e = double_to_octa(64.0);

  x = octa_fmult(y, z, &exceptions);
  
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("%s * %s == %s", sy, sz, sz),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  );

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_frem, test_print("Octa 64-bit float remainder")
) {
  char sx[32], sy[32], sz[32], se[32];

  octa x, y, z, e;
  octa exceptions;

  y = double_to_octa(65.0), z = double_to_octa(2), e = double_to_octa(1.0);

  x = octa_frem(y, z, &exceptions);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("%s = q * %s + r; r == %s", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  )

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_fsqrt, test_print("Octa 64-bit float square root.")
) {
  double dx;
  double dz = 49.0;
  octa exceptions;

  octa z = *(octa*)&dz;
  octa x = octa_fsqrt(z, ROUND_OFF, &exceptions);

  dx = *(double*)&x;

  test_check(
    test_print("sqrt(49.0) == 7.0"),
    dx == 7.0,
    test_failure("Expecting %f, got %f", 7.0, dx)
  )

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_fint, test_print("Octa 64-bit float integer.")
) {
  double dx;
  double dz = 49.3;
  octa exceptions;

  octa x = octa_fint(double_to_octa(dz), ROUND_OFF, &exceptions);

  test_check(
    test_print("No rouding of 49.3, should be 49.3"),
    octa_to_double(x) == 49.3,
    test_failure("Expecting %f, got %f", 49.3, dx)
  )

  x = octa_fint(double_to_octa(dz), ROUND_NEAR, &exceptions);
  test_check(
    test_print("Round near 49.3, should be 49.0"),
    octa_to_double(x) == 49.0,
    test_failure("Expecting %f, got %f", 49.0, dx)
  )

  x = octa_fint(double_to_octa(dz), ROUND_DOWN, &exceptions);
  test_check(
    test_print("Round down 49.3, should be 49.0"),
    octa_to_double(x) == 49.0,
    test_failure("Expecting %f, got %f", 49.0, dx)
  ) 

  x = octa_fint(double_to_octa(dz), ROUND_UP, &exceptions);
  test_check(
    test_print("Round up 49.3, should be 50.0"),
    octa_to_double(x) == 50.0,
    test_failure("Expecting %f, got %f", 50.0, dx)
  )

  test_success;
  test_teardown {}
  test_end;
}

define_test(
  octa_fcmp, test_print("Octa 64-bit float comparaison")
) {
  char sx[32], sy[32], sz[32], se[32];
  octa x, y, z, e;
  octa exceptions;

  y = double_to_octa(10.0);
  z = double_to_octa(20.0);
  e = int_to_octa(-1);

  x = octa_fcmp(y, z, &exceptions);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("%s < %s, should return %s", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  )
  
  z = double_to_octa(5.0);
  e = int_to_octa(1);
  x = octa_fcmp(y, z, &exceptions);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("%s > %s, should return %s", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  )

  z = double_to_octa(10.0);
  e = int_to_octa(0);
  x = octa_fcmp(y, z, &exceptions);
  octa_str(x, sx, 32), octa_str(y, sy, 32), octa_str(z, sz, 32), octa_str(e, se, 32);

  test_check(
    test_print("%s == %s, should be %s", sy, sz, se),
    x == e,
    test_failure("Expecting %s, got %s", se, sx)
  )

  test_success;
  test_teardown {} 
  test_end;
}

define_test(
  octa_fun, test_print("Octa 64-bit float unordered")
) {
  octa x;
  double dy, dz;
  
  dy = NAN;
  dz = NAN;

  x = octa_fun(double_to_octa(dy), double_to_octa(dz));

  test_check(
    test_print("y == NaN && z == NaN; expecting result to be 1."),
    x == 1,
    test_failure("Expecting %d, got %lu", 1, x)
  )

  test_success;
  test_teardown {} 
  test_end;
}

define_test(
  octa_fix, test_print("Octa 64-vbit float to signed fixed")
) {
  octa exceptions;
  double dz = -64.15;
  octa x;

  x = octa_fix(double_to_octa(dz), &exceptions);

  test_check(
    test_print("int(-64.15) == -64"),
    x == -64,
    test_failure("Expecting %d, got %lu", -64, x)
  )

  test_success;
  test_teardown {} 
  test_end;
}

define_test(
  octa_fixu, test_print("Octa 64-bit float to unsigned fixed")
) {
  octa exceptions;
  double dz = -64.15;
  octa x;

  x = octa_fixu(double_to_octa(dz), &exceptions);

  test_check(
    test_print("int(-64.15) == 64"),
    x == 64,
    test_failure("Expecting %d, got %lu", 64, x)
  )

  test_success;
  test_teardown {} 
  test_end;
}

define_test(
  octa_float, test_print("Octa 64-bit int to float")
) {
  octa exceptions;
  octa z = 10;
  octa x;
  double dx;

  x = octa_flot(z, ROUND_OFF, &exceptions);
  dx = octa_to_double(x);

  test_check(
    test_print("float(10) == 10.0"),
    dx == 10.0,
    test_failure("Expecting %f, got %f", 10.0, dx)
  )

  test_success;
  test_teardown {} 
  test_end;
}

define_test(
  octa_fqle, test_print("Octa 64-bit floating equivalent")
) {
  octa exceptions;
  octa x;
  double dx, dy, dz, de;
  
  dy = 10.0;
  dz = 10.2;
  de = 0.5;

  x = octa_feqle(double_to_octa(dy), double_to_octa(dz), double_to_octa(de), &exceptions);

  test_check(
    test_print("10.0 == 10.2 [e=0.2]"),
    x == 1,
    test_failure("Expecting %d, got %lu", 1, x)
  )

  test_success;
  test_teardown {} 
  test_end;
}

define_test_chapter(
  arith_1, test_print("Arithmetics #1"),
  octa_bdif,
  octa_wdif,
  octa_tdif,
  octa_odif,
  octa_right_shift,
  octa_left_shift
)


define_test_chapter(
  arith_2, test_print("Arithmetics #2"),
  octa_plus,
  octa_minus,
  octa_mult,
  octa_div,
  octa_signed_div
)

define_test_chapter(
  arith_3, test_print("Arithmetics #3"),
  octa_fdiv,
  octa_fmult,
  octa_frem,
  octa_fsqrt,
  octa_fint,
  octa_fcmp,
  octa_fun,
  octa_fix,
  octa_fixu,
  octa_float,
  octa_fqle
)

define_test_chapter(
  arith, test_print("Arithmetics"), 
  arith_1,
  arith_2,
  arith_3
)
