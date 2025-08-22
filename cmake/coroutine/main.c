#include <stdio.h>
#include <stdlib.h>
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
#define NUM 10000
  args* arg = (args*)calloc(NUM, sizeof(args));
  if (arg == NULL) {
    return;
  }
  int* arID = (int*)calloc(NUM, sizeof(int));
  if (arID == NULL) {
    free(arg);
    return;
  }
  for (int i = 0; i < NUM; ++i) {
    arg[i].n = i * 100;
    arID[i] = co_new(env, foo, &arg[i], 0);
  }
  printf("main start\n");
  bool isOK = false;
  do {
    isOK = false;
    for (int i = 0; i < NUM; ++i) {
      isOK = co_resume(env, arID[i]) || isOK;
    }
  } while (isOK);
  printf("main end\n");
  free(arg);
  free(arID);
}

int main() {
  co_env* env = co_open(0, 0);
  test(env);
  co_close(env);

  return 0;
}
