#ifndef C_COROUTINE_H_INCLUDE
#define C_COROUTINE_H_INCLUDE

#include <stdbool.h>
#include <stdint.h>

typedef struct co_env co_env;

// 协程ID，非负数为有效ID，否则为无效ID
typedef int co_id_t;

// 协程函数原型
typedef void (*co_func)(co_env*, void* arg);


// @brief 开启协程环境
// @param cap 协程初始容量，若为0，则默认为15，否则会自动与`2的N次方-1`对齐，当容量不足时会自动扩容为2倍加1
// @return 成功返回环境指针，否则返回NULL
co_env* co_open(uint16_t cap);

// @brief 关闭协程环境
// @param env         协程环境变量
void co_close(co_env* env);

// @brief 创建一个协程
// @param env         协程环境变量
// @param co_func     协程函数指针
// @param arg         传递给协程函数的参数
// @param stack_size  协程的栈大小，仅Windows有效，0为线程默认大小
// @return            成功返回一个协程ID；失败返回-1
co_id_t co_new(co_env* env, co_func, void* arg, size_t stack_size);

// @brief 继续执行一个协程
// @param env         协程环境变量
// @param id          协程ID
// @param arg         传递给协程函数的参数
// @param stack_size  协程的栈大小，仅Windows有效，0为线程默认大小
// @return            如果协程还可以调用co_resume，返回true，否则返回false
bool co_resume(co_env* env, co_id_t id);

// @brief 暂停正在运行的协程
// @param env   协程环境变量
void co_yield(co_env* env);

// @brief 获取协程环境变量正在运行的协程ID
// @return 非负数则为当前正在运行的协程ID，否则没有协程正在运行
co_id_t co_id(co_env* env);

#endif
