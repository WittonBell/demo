#ifndef _EXCEPTION_H_INCLUDE_
#define _EXCEPTION_H_INCLUDE_

#include <setjmp.h>

extern jmp_buf __global_jmp_buf__[];
extern unsigned int __global_jmp_buf_depth___;

#if __linux__
#define TRY \
  if (sigsetjmp(__global_jmp_buf__[__global_jmp_buf_depth___++], 1) == 0) {
#elif _WIN32
#define TRY if (setjmp(__global_jmp_buf__[__global_jmp_buf_depth___++]) == 0) {
#endif

#define CATCH                  \
  --__global_jmp_buf_depth___; \
  }                            \
  else

void init_try_catch();

#endif
