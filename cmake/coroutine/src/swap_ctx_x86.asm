[bits 32]

%ifdef __linux__
; 添加以下内容来明确标记栈不可执行
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

section .text

; 函数实现 void swap_ctx(co_ctx* from, co_ctx* to)
; Windows X86下需要有一个下划线，而Unix下不需要

%ifdef _WIN32
global _swap_ctx  ; 导出swap_ctx
_swap_ctx:
%else
global swap_ctx   ; 导出swap_ctx
swap_ctx:
%endif

    ; 此时栈的值由高到低依次为：to, from, ret
    ;
    ; 低地址
    ;   | ret  | <--- ESP
    ;   | from | <--- ESP+4
    ;   | to   | <--- ESP+8
    ; 高地址

    mov eax, [esp+4]  ; 取第一个参数from
    mov [eax+4],  ebx ; 保存EBX
    mov [eax+8],  ecx ; 保存ECX
    mov [eax+12], edx ; 保存EDX
    mov [eax+16], edi ; 保存EDI
    mov [eax+20], esi ; 保存ESI
    mov [eax+24], ebp ; 保存EBP
    lea ebx, [esp+4]  ; 获取调用本函数之前的栈指针ESP，以保持栈平衡，中间有一个4字节的返回值
    mov [eax+28], ebx ; 保存ESP
    mov ebx, [esp]    ; 获取返回地址
    mov [eax], ebx;   ; 保存返回地址

    mov eax, [esp+8]  ; 取第二个参数to
    mov ebx, [eax+4]  ; 恢复EBX
    mov ecx, [eax+8]  ; 恢复ECX
    mov edx, [eax+12] ; 恢复EDX
    mov edi, [eax+16] ; 恢复EDI
    mov esi, [eax+20] ; 恢复ESI
    mov ebp, [eax+24] ; 恢复EBP
    mov esp, [eax+28] ; 恢复ESP
    push dword [eax]  ; PUSH返回地址，后面的ret才能正常返回
    ret