#ifndef _CO_CTX_H_INCLUDE_
#define _CO_CTX_H_INCLUDE_

typedef struct co_ctx co_ctx;
typedef void (*fnCoMain)(void* arg);

typedef void* REG;

struct co_ctx {
#if defined(__x86_64__) || defined(_WIN64)
  REG ret, rbx, rcx, rdx, r8, r9, r12, r13, r14, r15, rdi, rsi, rbp, rsp;
#elif defined(__i386__) || defined(_WIN32)
  REG ret, ebx, ecx, edx, edi, esi, ebp, esp;
#endif
  size_t ss_size;  // 栈大小
  char* ss_sp;     // 栈地址
};

#if defined(_WIN32)
#define asm(x)
#endif

co_ctx* new_ctx();
void free_ctx(co_ctx*);

void make_ctx(co_ctx* ctx, fnCoMain fn, void* arg);

void swap_ctx(co_ctx* from, co_ctx* to) asm("swap_ctx");

#endif