用于C语言的协程实现

可用于Windows 32位/64位、Linux/macOS 32位/64位的跨平台协程库

有一个开关`USE_NATIVE_CO`宏，默认为关闭，其含义如下：

1. 如果未定义`USE_NATIVE_CO`宏，则使用汇编和C实现的一套API，可以跨平台（Windows、Linux、MacOS），使用共享栈。
2. 如果定义了`USE_NATIVE_CO`宏，将使用各平台的API来实现协程：

- Windows使用`纤程(Fiber)`，不能使用共享栈，内存占用会比较大
- Linux使用`ucontext`，可以使用共享栈
- MacOS由于不再支持`ucontext`，不可用

注意：在Windows下使用MinGW/GDB调试时，可能会出现一些由于竞态导致的`Segmentation fault`，比如在协程中调用`printf`函数时会经常出现段错误；而使用MinGW/LLDB调试时，则不会出现。主要是由于：

1. **GDB 的线程监视与协程上下文切换发生竞态** — GDB 在后台轮询线程、验证栈、访问 TLS（线程本地存储），而协程正在用汇编直接切换 RSP/RBP/寄存器
2. **碰撞时机** ——当 `printf` 深入 MSVC/MinGW 运行时（调用 `_setjmpex` 做 SEH 相关操作），栈指针/TLS 已经被破坏，导致段错误
3. **非确定性的原因** ——竞态：取决于 GDB 的轮询与协程切换是否同时发生（40% 崩溃率）
4. **为什么 LLDB 不崩溃** ——LLDB 的线程监视/SEH 处理与 GDB 不同，对失效的栈指针容错更好
