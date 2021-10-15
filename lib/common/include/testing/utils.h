#ifndef __TEST_UTILS_H__
#define __TEST_UTILS_H__

#include <assert.h>
#include <stdio.h>

#include "../macro.h"

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

test_context_t g_test_context;

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

#define test_print(fmt, ...) __test_context->cursor++; snprintf(__test_context->buffer[__test_context->cursor], 0xFF, fmt, ## __VA_ARGS__)
#define test_check(msg_stmt, test, failure) test_print("[CHECK L%i]: ", __LINE__); msg_stmt; test_print("\n"); if((test) == false) { failure } test_print("[CHECK OK].\n");
#define test_begin(msg_expr) test_transition(__test_context, BEGIN_TEST); test_print("\n[TEST %s] ", __FILE__); msg_expr; test_print("\n");
#define test_success test_transition(__test_context, SUCCEEDED_TEST);
#define test_end return;
#define test_failure(msg, ...) test_print("[CHECK FAILURE]: "); test_print(msg, ##__VA_ARGS__); test_print("\n"); test_print("[TEST FAILED]\n"); test_transition(__test_context, FAILED_TEST); goto lbl_teardown_test;
#define test_teardown lbl_teardown_test:

#define define_test(id, msg_expr) void __test_ ## id (test_context_t* __test_context); void test_ ## id (test_context_t* __test_context) {\
  test_begin(msg_expr);\
  __test_ ##id (__test_context);\
}\
void __test_ ##id (test_context_t* __test_context) 

#define exec_test(id) __exec_test__(id)
#define __exec_test__(id) test_ ##id (__test_context);
#define exec_tests(...) MAP(exec_test, __VA_ARGS__)

#define exec_test_chapter(id) test_##id(__test_context);
#define exec_test_chapters(...) MAP(exec_test_chapter, __VA_ARGS__)

#define define_test_chapter(id, msg_stmt, ...) \
void test_ ##id(test_context_t* __test_context) {\
  exec_tests(__VA_ARGS__)\
};

#define set_tests(...) \
int test() {\
  printf("TEST\n");\
  g_test_context.cursor = g_test_context.total = g_test_context.success = g_test_context.failed = 0;\
  g_test_context.state = IDLE;\
\
  test_context_t* __test_context = &g_test_context;\
  test_print(" ");\
\
  exec_test_chapters(__VA_ARGS__);\
  printf("\nPASSED: %d / %d.\n", g_test_context.success, g_test_context.total);\
  return g_test_context.failed > 0 ? 1 : 0;\
}

#endif