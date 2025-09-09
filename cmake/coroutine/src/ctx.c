#include <string.h>
#include <stdlib.h>
#include "co.h"
#include "ctx.h"

co_ctx* new_ctx() {
  return (co_ctx*)calloc(1, sizeof(co_ctx));
}

void free_ctx(co_ctx* p) {
  free(p);
}

void make_ctx(co_ctx* ctx, fnCoMain fn/*, void* arg*/) {
	// 这里需要减一个sizeof(void*)，才能指向最后一个能存放一个指针的有效地址
  char* sp = ctx->ss_sp + ctx->ss_size - sizeof(void*);
  // 16字节对齐
  sp = (char*)((size_t)sp & -16LL);
  // 将所有寄存器清零，但不能把后面的栈信息清掉了
  memset(ctx, 0, sizeof(*ctx) - sizeof(ctx->ss_size) - sizeof(char*));

  ctx->ret = fn;
#ifdef _WIN64
  // Win64下的调用约定有影子内存：
  // https://learn.microsoft.com/zh-cn/cpp/build/x64-calling-convention?view=msvc-170#calling-convention-defaults
  // 调用方必须始终分配足够的空间来存储 4 个寄存器参数，即使被调用方不使用这么多参数
  ctx->rsp = (char*)(sp) - 32;
  // Win64 前4个整数/指针参数：RCX, RDX, R8, R9
  //ctx->rcx = arg;
#elif defined(__x86_64__)
  // 设置栈指针
  ctx->rsp = sp;
  // Linux/MacOS 前6个整数/指针参数：RDI, RSI, RDX, RCX, R8, R9
  //ctx->rdi = arg;
#else
  // 设置栈指针
  //*(void**)sp = arg;
  ctx->esp = (char*)(sp) - sizeof(void*);
#endif
}
