#ifndef C_COROUTINE_H_INCLUDE
#define C_COROUTINE_H_INCLUDE

#include <stdbool.h>
#include <stdint.h>

typedef struct co_env co_env;

// 协程ID，非负数为有效ID，否则为无效ID
typedef int co_id_t;

// 协程函数原型
typedef void (*co_func)(void* arg);


// @brief 开启本线程的协程环境
// @param cap 协程初始容量，若为0，则默认为15，否则会自动与`2的N次方-1`对齐，当容量不足时会自动扩容为2倍加1
// @param share_stack_size 共享栈大小，会按8字节对齐，如果为0，则默认为1M
// @return 成功返回true，否则返回false
bool co_open(uint16_t cap, size_t share_stack_size);

// @brief 关闭本线程的协程环境
void co_close();

// @brief 在本线程创建一个协程
// @param co_func     协程函数指针
// @param arg         传递给协程函数的参数
// @param stack_size  协程的栈大小，仅Windows使用纤程时有效，0为线程默认大小
// @return            成功返回一个协程ID；失败返回-1
co_id_t co_new(co_func, void* arg, size_t stack_size);

// @brief 继续执行一个本线程的协程
// @param id          协程ID
// @return            如果协程还可以调用co_resume，返回true，否则返回false
bool co_resume(co_id_t id);

// @brief 切换本线程正在运行的协程，这里不取名co_yield以免与C++ 20中的关键字重名
void co_swap();

// @brief 获取本线程协程环境变量正在运行的协程ID
// @return 非负数则为当前正在运行的协程ID，否则没有协程正在运行
co_id_t co_id();

#endif
