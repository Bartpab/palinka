#ifndef __TEST_UTILS_H__
#define __TEST_UTILS_H__

#include <assert.h>
#include <stdio.h>

#include "../src/macro.h"

typedef enum {
  IDLE,
  TESTING,
} test_state_t;

typedef enum {
  BEGIN_TEST,
  FAILED_TEST,
  SUCCEEDED_TEST,
} test_transition_t;

typedef struct 
{
  char buffer[0xFF][0xFF];
  unsigned char cursor;
  int state;

  int total;
  int failed;
  int success;

} test_context_t;

void test_display_buffer(test_context_t* ctx) 
{
  for(unsigned char i = 1; i <= ctx->cursor; i++ )
    printf("%s", ctx->buffer[i]);
}

void test_transition(test_context_t* ctx, int transition) 
{
  switch(transition) {
    case BEGIN_TEST:
      ctx->state = TESTING;
      ctx->cursor = 0;
      ctx->total++;
    break;
    case FAILED_TEST:
      test_display_buffer(ctx);
      ctx->state = IDLE;
      ctx->failed++;
    break;
    case SUCCEEDED_TEST:
      printf(".");
      ctx->state = IDLE;
      ctx->success++;
    break;
  }
}

#define passert(x) for ( ; !(x) ; assert(x) )

#define test_print(fmt, ...) tctx->cursor++; snprintf(tctx->buffer[tctx->cursor], 0xFF, fmt, ## __VA_ARGS__); 
#define test_check(msg_stmt, test, failure) test_print("[CHECK L%i]: ", __LINE__); msg_stmt; test_print("\n"); passert(test) { failure; }  test_print("[CHECK OK].\n");

#define test_begin(msg_stmt) test_transition(tctx, BEGIN_TEST); test_print("\n[TEST %s] ", __FILE__); msg_stmt; test_print("\n");
#define test_failure(msg, ...) test_print("[CHECK FAILURE]: ");  test_print(msg, ##__VA_ARGS__); \
  test_print("\n");\
  test_print("[TEST FAILED]\n");\
  test_transition(tctx, FAILED_TEST);\
  goto lbl_teardown_test;

#define test_success test_transition(tctx, SUCCEEDED_TEST);
#define test_end return;

#define define_test(id, msg_stmt) \
void __test_ ## id (test_context_t* tctx); \
void test_ ## id (test_context_t* tctx) { \
  test_begin(msg_stmt);\
  __test_ ##id (tctx); \
} \
void __test_ ##id (test_context_t* tctx) 

#define test_teardown lbl_teardown_test: 

#define exec_test(id) __exec_test__(id)
#define __exec_test__(id) test_ ##id (tctx);
#define exec_tests(...) MAP(exec_test, __VA_ARGS__)

#define exec_test_chapter(id) test_##id(tctx);
#define exec_test_chapters(...) MAP(exec_test_chapter, __VA_ARGS__)

#define define_test_chapter(id, msg_stmt, ...) \
void test_ ##id(test_context_t* tctx) {\
   exec_tests(__VA_ARGS__)\
};

#define set_tests(...) \
void test() {\
  test_context_t ctx;\
  ctx.total = 0, ctx.success = 0, ctx.failed = 0;\
  ctx.cursor = 0;\
  ctx.state = IDLE;\
\
  test_context_t* tctx = &ctx;\
  test_print("");\
\
  exec_test_chapters(__VA_ARGS__); \
  printf("\n");\
  printf("PASSED: %d /  %d.\n", ctx.success, ctx.total);\
}

#endif