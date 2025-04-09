[bits 64]
extern printf ;声明printf，将调用C语言的printf函数

section .rodata
    msg  db "这是汇编中的输出",0xa,0

section .text
global asmFoo

%ifdef WIN32
asmFoo:
    push	rbp
    mov		rbp, rsp
	sub		rsp, 0x20
    lea		rax, [rel msg]
	mov		rcx, rax
    call	printf
	add		rsp, 0x20
    pop		rbp
    ret
%else
asmFoo:
    push	rbp
    mov		rbp, rsp
	sub		rsp, 0x20
    lea 	rdi, [rel msg]
    call	printf wrt ..plt ;也可以使用 call 	[rel printf wrt ..got]
	add		rsp, 0x20
    pop		rbp
    ret
%endif
