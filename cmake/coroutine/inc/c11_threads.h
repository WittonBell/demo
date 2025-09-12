#ifndef _C11_THREADS_H_
#define _C11_THREADS_H_

#ifdef __has_include
#if __has_include(<threads.h>)
#include <threads.h>
#define HAS_THREADS_H 1
#endif
#endif

#if !defined(HAS_THREADS_H) && (defined(__MINGW32__) || defined(__MINGW64__))

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <time.h>

// 检测编译器是否支持thread_local
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_THREADS__)
// C11及以上版本，且支持线程
#define thread_local _Thread_local
#elif defined(__GNUC__) || defined(__clang__)
// GCC或Clang编译器
#define thread_local __thread
#elif defined(_MSC_VER)
// MSVC编译器
#define thread_local __declspec(thread)
#else
// 其他编译器，可能不支持线程局部存储
#warning "thread_local is not supported by this compiler"
#define thread_local
#endif

// 定义C11线程类型
typedef pthread_t thrd_t;
typedef pthread_mutex_t mtx_t;
typedef pthread_cond_t cnd_t;
typedef pthread_key_t tss_t;
typedef pthread_once_t once_flag;

// 线程函数返回值
enum {
  thrd_success = 0,
  thrd_timedout = 1,
  thrd_busy = 2,
  thrd_error = 3,
  thrd_nomem = 4
};

// 互斥锁类型
enum { mtx_plain = 0, mtx_recursive = 1, mtx_timed = 2 };

// 初始化once_flag的宏
#define ONCE_FLAG_INIT PTHREAD_ONCE_INIT

// 线程函数类型
typedef int (*thrd_start_t)(void*);

// 线程特定数据析构函数类型
typedef void (*tss_dtor_t)(void*);

// 线程函数
static inline int thrd_create(thrd_t* thr, thrd_start_t func, void* arg) {
  return pthread_create(thr, NULL, (void* (*)(void*))func, arg) == 0
             ? thrd_success
             : thrd_error;
}

static inline int thrd_join(thrd_t thr, int* res) {
  void* thread_result;
  int ret = pthread_join(thr, &thread_result);
  if (res != NULL) {
    *res = (int)(intptr_t)thread_result;
  }
  return ret == 0 ? thrd_success : thrd_error;
}

static inline int thrd_detach(thrd_t thr) {
  return pthread_detach(thr) == 0 ? thrd_success : thrd_error;
}

static inline void thrd_exit(int res) {
  pthread_exit((void*)(intptr_t)res);
}

static inline thrd_t thrd_current(void) {
  return pthread_self();
}

static inline int thrd_equal(thrd_t thr1, thrd_t thr2) {
  return pthread_equal(thr1, thr2);
}

static inline int thrd_sleep(const struct timespec* duration,
                             struct timespec* remaining) {
  int ret = nanosleep(duration, remaining);
  if (ret == 0)
    return 0;
  return errno == EINTR ? -1 : -2;
}

static inline void thrd_yield(void) {
  sched_yield();
}

// 互斥锁函数
static inline int mtx_init(mtx_t* mutex, int type) {
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);

  if (type & mtx_recursive) {
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  }

  int ret = pthread_mutex_init(mutex, &attr);
  pthread_mutexattr_destroy(&attr);

  return ret == 0 ? thrd_success : thrd_error;
}

static inline int mtx_lock(mtx_t* mutex) {
  return pthread_mutex_lock(mutex) == 0 ? thrd_success : thrd_error;
}

static inline int mtx_timedlock(mtx_t* mutex,
                                const struct timespec* time_point) {
  struct timespec ts = {.tv_sec = time_point->tv_sec,
                        .tv_nsec = time_point->tv_nsec};
  int ret = pthread_mutex_timedlock(mutex, &ts);
  if (ret == 0)
    return thrd_success;
  return ret == ETIMEDOUT ? thrd_timedout : thrd_error;
}

static inline int mtx_trylock(mtx_t* mutex) {
  return pthread_mutex_trylock(mutex) == 0 ? thrd_success : thrd_busy;
}

static inline int mtx_unlock(mtx_t* mutex) {
  return pthread_mutex_unlock(mutex) == 0 ? thrd_success : thrd_error;
}

static inline void mtx_destroy(mtx_t* mutex) {
  pthread_mutex_destroy(mutex);
}

// 条件变量函数
static inline int cnd_init(cnd_t* cond) {
  return pthread_cond_init(cond, NULL) == 0 ? thrd_success : thrd_error;
}

static inline int cnd_signal(cnd_t* cond) {
  return pthread_cond_signal(cond) == 0 ? thrd_success : thrd_error;
}

static inline int cnd_broadcast(cnd_t* cond) {
  return pthread_cond_broadcast(cond) == 0 ? thrd_success : thrd_error;
}

static inline int cnd_wait(cnd_t* cond, mtx_t* mutex) {
  return pthread_cond_wait(cond, mutex) == 0 ? thrd_success : thrd_error;
}

static inline int cnd_timedwait(cnd_t* cond,
                                mtx_t* mutex,
                                const struct timespec* time_point) {
  struct timespec ts = {.tv_sec = time_point->tv_sec,
                        .tv_nsec = time_point->tv_nsec};
  int ret = pthread_cond_timedwait(cond, mutex, &ts);
  if (ret == 0)
    return thrd_success;
  return ret == ETIMEDOUT ? thrd_timedout : thrd_error;
}

static inline void cnd_destroy(cnd_t* cond) {
  pthread_cond_destroy(cond);
}

// 线程特定存储函数
static inline int tss_create(tss_t* key, tss_dtor_t dtor) {
  return pthread_key_create(key, dtor) == 0 ? thrd_success : thrd_error;
}

static inline void* tss_get(tss_t key) {
  return pthread_getspecific(key);
}

static inline int tss_set(tss_t key, void* val) {
  return pthread_setspecific(key, val) == 0 ? thrd_success : thrd_error;
}

static inline void tss_delete(tss_t key) {
  pthread_key_delete(key);
}

// 一次性初始化函数
static inline void call_once(once_flag* flag, void (*func)(void)) {
  pthread_once(flag, func);
}

#endif

#endif  // _C11_THREADS_H_