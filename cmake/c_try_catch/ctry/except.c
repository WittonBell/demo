#include "except.h"
#include <signal.h>
#if __linux__
#include <stdlib.h>
#include <string.h>
#endif

#if _MSC_VER
#include <windows.h>

LONG WINAPI seh_handler(EXCEPTION_POINTERS* e) {
  if (e->ExceptionRecord->ExceptionCode == EXCEPTION_INT_DIVIDE_BY_ZERO) {
    raise(SIGFPE);
    return EXCEPTION_CONTINUE_EXECUTION;
  }
  return EXCEPTION_CONTINUE_SEARCH;
}
#endif

jmp_buf __global_jmp_buf__[256] = {};
unsigned int __global_jmp_buf_depth___ = 0;

static void sig(int s) {
  switch (s) {
    case SIGSEGV:
    case SIGFPE:
#if _WIN32
      (void)signal(s, sig);
      longjmp(__global_jmp_buf__[--__global_jmp_buf_depth___], 1);
#elif __linux__
      siglongjmp(__global_jmp_buf__[--__global_jmp_buf_depth___], 1);
#endif
      break;
    default:
      break;
  }
}

void init_try_catch() {
#if _MSC_VER
  AddVectoredExceptionHandler(1, seh_handler);
#endif

#if __linux__
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  if (sigfillset(&sa.sa_mask)) {
    abort();
  }
  sa.sa_handler = sig;
  if (sigaction(SIGFPE, &sa, NULL) == -1) {
    abort();
  }
  if (sigaction(SIGSEGV, &sa, NULL) == -1) {
    abort();
  }
#elif _WIN32
  (void)signal(SIGFPE, sig);
  (void)signal(SIGSEGV, sig);
#endif
}
