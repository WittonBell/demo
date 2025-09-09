#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include "co.h"

uint64_t thread_id() {
  thrd_t t = thrd_current();
#ifdef _WIN32
  return t._Tid;
#else
  return t;
#endif
}

typedef struct args {
  int n;
} args;

static void foo(void* ud) {
  struct args* arg = ud;
  int start = arg->n;
  uint64_t id = thread_id();
  for (int i = 0; i < 3; i++) {
    printf("[%" PRIu64 "] coroutine %d : %d\n", id, co_id(), start + i);
    co_swap();
  }
}

static void test() {
#define NUM 10
  args* arg = (args*)calloc(NUM, sizeof(args));
  if (arg == NULL) {
    return;
  }
  int* ar_id = (int*)calloc(NUM, sizeof(int));
  if (ar_id == NULL) {
    free(arg);
    return;
  }
  for (int i = 0; i < NUM; ++i) {
    arg[i].n = i * 100;
    ar_id[i] = co_new(foo, &arg[i], 0);
  }
  uint64_t id = thread_id();
  printf("[%" PRIu64 "] main start\n", id);
  bool isOK = false;
  do {
    isOK = false;
    for (int i = 0; i < NUM; ++i) {
      isOK = co_resume(ar_id[i]) || isOK;
    }
  } while (isOK);
  printf("[%" PRIu64 "] main end\n", id);
  free(arg);
  free(ar_id);
}

static int worker(void* arg) {
  if (!co_open(0, 0)) {
    return 1;
  }
  test();
  co_close();
  return 0;
}

int main() {
#define N 10
  thrd_t ar_id[N];
  for (int i = 0; i < N; ++i) {
    thrd_create(&ar_id[i], worker, NULL);
  }
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 1000;
  thrd_sleep(&ts, &ts);
  for (int i = 0; i < N; ++i) {
    int res;
    thrd_join(ar_id[i], &res);
  }

  return 0;
}
