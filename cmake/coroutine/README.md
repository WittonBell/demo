用于C语言的协程实现

可用于Windows 32位/64位、Linux/macOS 32位/64位的跨平台协程库

有一个开关`USE_NATIVE_CO`宏，默认为关闭，其含义如下：
1. 如果未定义`USE_NATIVE_CO`宏，则使用汇编和C实现的一套API，可以跨平台（Windows、Linux、MacOS），使用共享栈。
2. 如果定义了`USE_NATIVE_CO`宏，将使用各平台的API来实现协程：
- Windows使用`纤程(Fiber)`，不能使用共享栈，内存占用会比较大
- Linux使用`ucontext`，可以使用共享栈
- MacOS由于不再支持`ucontext`，不可用
