#ifndef _CO_CTX_H_INCLUDE_
#define _CO_CTX_H_INCLUDE_

typedef struct co_ctx co_ctx;
typedef void (*fnCoMain)();

typedef void* REG;

struct co_ctx {
#if defined(_WIN64)
  // x64 ABI 将寄存器 RBX、RBP、RDI、RSI、RSP、R12、R13、R14、R15 和 XMM6 -
  // XMM15 视为非易失性。 它们必须由使用它们的函数保存和还原。
  // https://learn.microsoft.com/zh-cn/cpp/build/x64-calling-convention?view=msvc-170
  // win64 这里需要保存rcx，因为rcx需要用来传递co_main的参数
  REG ret, rbx, r12, r13, r14, r15, rbp, rsp, rdi, rsi, rcx;
#elif defined(__x86_64__)
  // System V ABI需要保存rbx, r12, r13, r14, r15, rbp, rsp
  // 这里同样需要保存调用约定传参寄存器rdi,需要用来传递co_main的参数
  REG ret, rbx, r12, r13, r14, r15, rbp, rsp, rdi;
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

void make_ctx(co_ctx* ctx, fnCoMain fn/*, void* arg*/);

void swap_ctx(co_ctx* from, co_ctx* to) asm("swap_ctx");

#endif