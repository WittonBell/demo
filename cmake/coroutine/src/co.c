#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c11_threads.h"

#ifdef USE_NATIVE_CO
#if __APPLE__ && __MACH__
// 在macOS 10.5之后，ucontext系列函数已被弃用
#error macOS not supported
#include <ucontext.h>
#elif defined(__linux__)
#include <ucontext.h>
typedef ucontext_t coctx;
#elif defined(_WIN32)
#include <windows.h>
typedef LPVOID coctx;
#endif
#else
#include "ctx.h"
typedef co_ctx* coctx;
#endif

#if __STDC_VERSION__ >= 202311L
#ifdef NULL
#undef NULL
#define NULL nullptr
#endif
#endif

#include "co.h"

static thread_local co_env* env = NULL;

enum {
  STACK_SIZE = (1024 * 1024)  // 协程默认共享栈大小
};
enum {
  DEFAULT_COROUTINE = 15  // 默认的协程容量
};

typedef enum state {
  CO_NONE = 0,     // 不可调用resume的状态
  CO_READY = 1,    // 就绪状态
  CO_RUNNING = 2,  // 运行状态
  CO_SUSPEND = 3,  // 挂起状态
  CO_STOPED = 4    // 停止等待回收的状态，在Win32下，纤程需要在主函数中才能删除
} state;

typedef struct coroutine {
  co_func func;    // 协程函数
  void* arg;       // 协程函数参数
  coctx ctx;       // 协程上下文
  co_env* env;     // 协程环境
  state status;    // 协程状态
  ptrdiff_t cap;   //
  ptrdiff_t size;  // 在协程挂起时，保存的栈大小
#if !(defined(USE_NATIVE_CO) && defined(_WIN32))
  char* stack;  // 在协程挂起时，保存的栈数据
#endif
} coroutine, *coroutine_p;

struct co_env {
  coctx main;          // 主函数上下文
  int nco;             // 协程数量
  int cap;             // 协程容量
  co_id_t running_id;  // 当前运行的协程ID
  coroutine_p* co;     // 协程数组，大小为cap
#if !(defined(USE_NATIVE_CO) && defined(_WIN32))
  size_t stack_size;  // 共享栈大小
  char stack[0];      // 共享栈内存
#endif
};

// 协程入口函数
static void _co_main() {
  co_id_t id = env->running_id;  // 运行的ID也是环境管理器中的协程索引
  coroutine* co = env->co[id];   // 根据索引取协程
  co->func(co->arg);             // 调用协程函数
  co->status = CO_STOPED;        // 设置为停止状态，等待回收资源
  --env->nco;                    // 当前协程数量减1
  env->running_id = -1;          // 设置为-1，表示当前没有协程在执行
#ifdef USE_NATIVE_CO
#ifdef _WIN32
  SwitchToFiber(env->main);  // 在Win32下需要切换到主函数
#endif
#else
  swap_ctx(co->ctx, env->main);
#endif
}

static coroutine* _co_new(co_func func, void* arg, size_t stack_size) {
  coroutine* co = (coroutine*)calloc(1, sizeof(*co));
  if (co == NULL) {
    return NULL;
  }
  co->func = func;
  co->arg = arg;
  co->env = env;
  co->cap = 0;
  co->size = 0;
  co->status = CO_READY;  // 设置为就绪状态
#ifdef USE_NATIVE_CO
#ifdef _WIN32
  // 创建纤程
  co->ctx = CreateFiber(stack_size, (LPFIBER_START_ROUTINE)_co_main, env);
#else
  co->stack = NULL;
#endif
#else
  co->ctx = new_ctx();
  co->stack = NULL;
#endif
  return co;
}

static void _co_delete(coroutine* co) {
#ifdef USE_NATIVE_CO
#ifdef _WIN32
  // 释放纤程，但是不能在纤程函数中释放
  if (co->ctx != NULL) {
    DeleteFiber(co->ctx);
    co->ctx = NULL;
  }
#else
  // 释放私有栈内存
  if (co->stack != NULL) {
    free(co->stack);
    co->stack = NULL;
  }
#endif
#else
  if (co->ctx != NULL) {
    free_ctx(co->ctx);
  }
  // 释放私有栈内存
  if (co->stack != NULL) {
    free(co->stack);
    co->stack = NULL;
  }
#endif
  free(co);
}

static inline bool _IsValidCap(size_t n) {
  return ((n + 1) & n) == 0 && n > 0;
}

static inline int _align(uint16_t cap) {
  int n = 1;
  while (n <= cap) {
    n <<= 1;
  }
  assert(_IsValidCap(n - 1));
  return n - 1;
}

#define ALIGN(x, a) ((x + a - 1) & (~(a - 1)))

bool co_open(uint16_t cap, size_t share_stack_size) {
  if (share_stack_size == 0) {
    share_stack_size = STACK_SIZE;
  }
  share_stack_size = ALIGN(share_stack_size, 8);
  env = (co_env*)calloc(1, sizeof(co_env) + share_stack_size);
  if (env == NULL) {
    return false;
  }
#if !defined(USE_NATIVE_CO) || defined(USE_NATIVE_CO) && !defined(_WIN32) 
  env->stack_size = share_stack_size;
#endif
  env->nco = 0;
  if (cap == 0) {
    assert(_IsValidCap(DEFAULT_COROUTINE));
    env->cap = DEFAULT_COROUTINE;
  } else {
    env->cap = (int)_align(cap);
  }
  env->running_id = -1;
  env->co = (coroutine**)calloc(env->cap, sizeof(coroutine*));
  if (env->co == NULL) {
    free(env);
    return false;
  }
#ifdef USE_NATIVE_CO
#ifdef _WIN32
  // 将线程转为纤程，并得到纤程句柄
  env->main = ConvertThreadToFiber(env);
  if (env->main == NULL) {
    free(env);
    return false;
  }
#endif
#else
  env->main = new_ctx();
  if (env->main == NULL) {
    free(env->co);
    free(env);
    return false;
  }
#endif
  return true;
}

void co_close() {
  if (env == NULL) {
    return;
  }
  // 如果有协程没释放，则释放
  for (int i = 0; i < env->cap; i++) {
    coroutine* co = env->co[i];
    if (co) {
      _co_delete(co);
      env->co[i] = NULL;
    }
  }
  free((void*)env->co);
#ifdef USE_NATIVE_CO
#ifdef _WIN32
  ConvertFiberToThread();
#endif
#else
  free_ctx(env->main);
#endif
  free(env);
  env = NULL;
}

int co_new(co_func func, void* arg, size_t stack_size) {
  // 分配协程内存
  coroutine* co = _co_new(func, arg, stack_size);
  // 如果当前协程数量达到最大容量，则扩容
  if (env->nco >= env->cap) {
    int id = env->cap;
    // 这里新容量使用2的n次方-1的方式对齐，方便后面取索引时，使用位与的方式快速计算
    int cap = id * 2 + 1;
    assert(_IsValidCap(cap));
    void* p = realloc((void*)env->co, (size_t)cap * sizeof(env->co));
    if (p == NULL) {
      return -1;
    }
    env->co = (coroutine**)p;
    env->cap = cap;
    // 将后面新增的内存置0
    memset((void*)&env->co[id], 0, (size_t)(cap - id) * sizeof(env->co));
    env->co[id] = co;
    ++env->nco;
    return id;
  }
  for (int i = 0; i < env->cap; i++) {
    // 由于容量使用的是2的n次方-1的方式对齐，这里使用位与的方式快速计算索引
    int id = (i + env->nco) & env->cap;
    if (env->co[id] == NULL) {
      env->co[id] = co;
      ++env->nco;
      return id;
    }
  }
  assert(0);
  return -1;
}

bool co_resume(co_id_t id) {
  assert(env->running_id == -1);
  assert(id >= 0 && id < env->cap);
  coroutine* co = env->co[id];
  if (co == NULL) {
    return false;
  }
  int status = co->status;
  switch (status) {
    case CO_READY:
      env->running_id = id;
      co->status = CO_RUNNING;
#ifdef USE_NATIVE_CO
#if defined(_WIN32)
      if (co->ctx != NULL)
        SwitchToFiber(co->ctx);
#else
      // 获取协程的上下文
      getcontext(&co->ctx);
      co->ctx.uc_stack.ss_sp = env->stack;
      co->ctx.uc_stack.ss_size = env->stack_size;
      co->ctx.uc_link = &env->main;  // 结束后需要返回主函数，否则就会退出程序
      makecontext(&co->ctx, (void (*)(void))_co_main, 0);
      swapcontext(&env->main, &co->ctx);
#endif
#else
      co->ctx->ss_sp = env->stack;
      co->ctx->ss_size = env->stack_size;
      make_ctx(co->ctx, _co_main);
      swap_ctx(env->main, co->ctx);
#endif
      return true;
    case CO_SUSPEND:
      env->running_id = id;
      co->status = CO_RUNNING;
#ifdef USE_NATIVE_CO
#ifdef _WIN32
      if (co->ctx != NULL)
        SwitchToFiber(co->ctx);
#else
      memcpy(env->stack + env->stack_size - co->size, co->stack, co->size);
      swapcontext(&env->main, &co->ctx);
#endif
#else
      memcpy(env->stack + env->stack_size - co->size, co->stack, co->size);
      swap_ctx(env->main, co->ctx);
#endif
      return true;
    case CO_STOPED:
      // 这里统一删除协程，释放资源
      _co_delete(co);
      env->co[id] = NULL;
      return false;
    default:
      assert(0);
      return false;
  }
}

#if !(defined(USE_NATIVE_CO) && defined(_WIN32))
// 保存栈数据
static void _save_stack(coroutine* co) {
  assert((char*)&co > env->stack);
  const char* top = env->stack + env->stack_size;
  char dummy = 0;
  assert(top - &dummy <= co->env->stack_size);
  if (co->cap < top - &dummy) {
    if (co->stack) {
      free(co->stack);
    }
    co->cap = top - &dummy;
    co->stack = malloc(co->cap);
  }
  co->size = top - &dummy;
  if (co->stack) {
    memcpy(co->stack, &dummy, co->size);
  }
}
#endif

void co_swap() {
  co_id_t id = env->running_id;
  assert(id >= 0);
  coroutine* co = env->co[id];
  co->status = CO_SUSPEND;
  env->running_id = -1;
#ifdef USE_NATIVE_CO
#ifdef _WIN32
  SwitchToFiber(env->main);
#else
  _save_stack(co);
  swapcontext(&co->ctx, &env->main);
#endif
#else
  _save_stack(co);
  swap_ctx(co->ctx, env->main);
#endif
}

co_id_t co_id() {
  return env->running_id;
}
