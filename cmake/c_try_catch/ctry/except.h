#ifndef _EXCEPTION_H_INCLUDE_
#define _EXCEPTION_H_INCLUDE_

#include <setjmp.h>

typedef struct jump_buffer {
  jmp_buf buf;
  int result;
} jump_buffer;

extern jump_buffer __global_jmp_buf__[];
extern unsigned int __global_jmp_buf_depth___;

#if __linux__
#define TRY \
  if (sigsetjmp(__global_jmp_buf__[__global_jmp_buf_depth___++].buf, 1) == 0) {
#elif _WIN32
#define TRY \
  if (setjmp(__global_jmp_buf__[__global_jmp_buf_depth___++].buf) == 0) {
#endif

#define CATCH(x)               \
  --__global_jmp_buf_depth___; \
  }                            \
  else if (__global_jmp_buf__[__global_jmp_buf_depth___].result == (x))

#define EXCEPT                 \
  --__global_jmp_buf_depth___; \
  }                            \
  else

#ifdef _WIN32
#define THROW(x)                                                \
  __global_jmp_buf__[--__global_jmp_buf_depth___].result = (x); \
  longjmp(__global_jmp_buf__[__global_jmp_buf_depth___].buf, (x));
#elif __linux__
#define THROW(x)                                                \
  __global_jmp_buf__[--__global_jmp_buf_depth___].result = (x); \
  siglongjmp(__global_jmp_buf__[__global_jmp_buf_depth___].buf, (x));
#endif

void init_try_catch();

#endif
