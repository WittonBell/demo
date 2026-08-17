#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "c11_threads.h"
#include "co.h"

// 在MinGW中printf函数输出UTF8的汉字会有问题，它内部是一个字节一个字节输出的，需要改为一次性输出
#if defined(__MINGW32__) || defined(__MINGW64__)
#include <windows.h>
#include <stdarg.h>

#define printf __Print

static int __Print(const char* fmt, ...) {
  va_list va = {};
  va_start(va, fmt);
  char buffer[8192];
  int n = vsnprintf(buffer, sizeof(buffer), fmt, va);
  if (n < sizeof(buffer))
    fputs(buffer, stdout);
  else {
    char* p = (char*)malloc(n + 1);
    n = vsnprintf(p, n + 1, fmt, va);
    fputs(p, stdout);
    free(p);
  }
  va_end(va);
  return n;
}
#endif

static uint64_t thread_id() {
  thrd_t t = thrd_current();
#ifdef _MSC_VER
  return t._Tid;
#else
  return (uint64_t)t;
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
    // 在MinGW下使用GDB调试器调试时，下面这句会经常出现段错误。建议使用LLDB调试器。
    printf("thread[%" PRIu64 "] coroutine %d value %d\n", id, co_id(), start + i);
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
  co_wait();
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
#ifdef _WIN32
  SetConsoleOutputCP(65001);
#endif
#define N 10
  thrd_t ar_id[N];
  for (int i = 0; i < N; ++i) {
    (void)thrd_create(&ar_id[i], worker, NULL);
  }
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 1000;
  (void)thrd_sleep(&ts, &ts);
  for (int i = 0; i < N; ++i) {
    int res = 0;
    (void)thrd_join(ar_id[i], &res);
  }
  return 0;
}
