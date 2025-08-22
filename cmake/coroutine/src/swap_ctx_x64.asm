[bits 64]

%ifdef __linux__
; 添加以下内容来明确标记栈不可执行
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

section .text
; 定义 swap_ctx 函数
global swap_ctx

; 函数实现 void swap_ctx(co_ctx* from, co_ctx* to)
; Win64 前4个整数/指针参数：RCX, RDX, R8, R9
; Linux/MacOS 前6个整数/指针参数：RDI, RSI, RDX, RCX, R8, R9
swap_ctx:

%ifdef _WIN32
%define FROM rcx
%define TO rdx
%else
%define FROM rdi
%define TO rsi
%endif
    ; Win64下有影子内存：此时栈的值由高到低依次应为：to, from, ret
    ; 编译器生成代码会使用如下指令回设参数到栈中
    ; mov [rsp+16], rdx ; 回设参数to
    ; mov [rsp+8], rcx  ; 回设参数from
    ; 由于本函数没未如编译器生成代码一般回设参数到栈中，所以不能使用栈取参数
    ;
    ; 低地址
    ;   | ret  | <--- RSP
    ;   | from | <--- 需要被调用函数主动设置RCX的值到RSP+8，才能在栈中获取
    ;   | to   | <--- 需要被调用函数主动设置RDX的值到RSP+16，才能在栈中获取
    ; 高地址

    ; Linux/MacOS下没有影子内存：此时栈的值为：
    ; 低地址
    ;   | ret  | <--- RSP
    ; 高地址

    ; 保存当前上下文到 rcx 指向的结构，rcx即：from
    mov [FROM + 0x08], rbx  ; 保存RBX
    mov [FROM + 0x10], rcx  ; 保存RCX
    mov [FROM + 0x18], rdx  ; 保存RDX
    mov [FROM + 0x20], r8   ; 保存R8
    mov [FROM + 0x28], r9   ; 保存R9
    mov [FROM + 0x30], r12  ; 保存R12
    mov [FROM + 0x38], r13  ; 保存R13
    mov [FROM + 0x40], r14  ; 保存R14
    mov [FROM + 0x48], r15  ; 保存R15
    mov [FROM + 0x50], rdi  ; 保存RDI
    mov [FROM + 0x58], rsi  ; 保存RSI
    mov [FROM + 0x60], rbp  ; 保存RBP
    mov [FROM + 0x68], rsp  ; 保存RSP
    mov rax, [rsp]          ; 获取返回地址
    mov [FROM], rax         ; 保存返回地址
    
    ; 清零 rax
    xor rax, rax
    
    ; 从 rdx/rsi 指向的结构恢复新上下文，rdx/rsi即：to
    mov rbx, [TO + 0x08]  ; 恢复RBX
    mov rcx, [TO + 0x10]  ; 恢复RCX
%ifndef _WIN32
    mov rdx, [TO + 0x18]  ; 恢复RDX
%endif
    mov r8,  [TO + 0x20]  ; 恢复R8
    mov r9,  [TO + 0x28]  ; 恢复R9
    mov r12, [TO + 0x30]  ; 恢复R12
    mov r13, [TO + 0x38]  ; 恢复R13
    mov r14, [TO + 0x40]  ; 恢复R14
    mov r15, [TO + 0x48]  ; 恢复R15
    mov rdi, [TO + 0x50]  ; 恢复RDI
%ifdef _WIN32
    mov rsi, [TO + 0x58]  ; 恢复RSI
%endif
    mov rbp, [TO + 0x60]  ; 恢复RBP
    mov rsp, [TO + 0x68]  ; 恢复RSP
    ; 调整栈指针并压入返回地址
    lea rsp, [rsp + 8]
    push qword [TO] ; 压入返回地址

    ; 最后恢复rdx/rsi寄存器
%ifdef _WIN32
    mov rdx, [TO + 0x18]  ; 恢复RDX
%else
    mov rsi, [TO + 0x58]  ; 恢复RSI
%endif
    
    ; 返回，将跳转到新上下文的返回地址
    ret