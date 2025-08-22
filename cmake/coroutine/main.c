#include <stdio.h>
#include "co.h"

typedef struct args {
  int n;
} args;

static void foo(co_env* env, void* ud) {
  struct args* arg = ud;
  int start = arg->n;
  for (int i = 0; i < 3; i++) {
    printf("coroutine %d : %d\n", co_id(env), start + i);
    co_swap(env);
  }
}

static void test(co_env* env) {
  args arg1 = {10};
  args arg2 = {20};
  args arg3 = {30};
  args arg4 = {40};

  int co1 = co_new(env, foo, &arg1, 0);
  int co2 = co_new(env, foo, &arg2, 0);
  int co3 = co_new(env, foo, &arg3, 0);
  int co4 = co_new(env, foo, &arg4, 0);
  printf("main start\n");
  bool isOK = false;
  do {
    isOK = co_resume(env, co1);
    isOK = co_resume(env, co2) || isOK;
    isOK = co_resume(env, co3) || isOK;
    isOK = co_resume(env, co4) || isOK;
  } while (isOK);
  printf("main end\n");
}

int main() {
  co_env* s = co_open(0, 0);
  test(s);
  co_close(s);

  return 0;
}
