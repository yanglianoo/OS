[bits 32]


extern handler_table

section .text
; 这段汇编代码是一个宏定义，用于定义中断处理程序的代码。具体来说：
; %macro INTERRUPT_HANDLER 2：定义一个名为INTERRUPT_HANDLER的宏，该宏有两个参数，%1和%2。
; interrupt_handler_%1：将宏的第一个参数%1作为中断处理程序的名称，并在名称前加上"interrupt_handler_"前缀。
; %ifn %2：如果宏的第二个参数%2不存在，则执行下面的代码。
; push 0x20222202：将0x20222202压入堆栈，作为中断处理程序的返回地址。
; push %1：将宏的第一个参数%1压入堆栈，作为中断处理程序的参数。
; jmp interrupt_entry：跳转到interrupt_entry标签处执行中断处理程序的代码。
; 因此，使用该宏可以快速生成中断处理程序的代码。使用方法为：在代码中调用该宏，并传递中断处理程序的名称和是否需要在堆栈中压入返回地址两个参数。
%macro INTERRUPT_HANDLER 2
interrupt_handler_%1:
%ifn %2
    push 0x20222202
%endif
    ; 压入中断号
    push %1;
    ; 跳转到终断入口
    jmp interrupt_entry
%endmacro

interrupt_entry:

    ; 保存上文的寄存器信息
    push ds
    push es
    push fs
    push gs
    ; 压入通用寄存器
    pusha 
    ; 取出中断号
    mov eax, [esp + 12 * 4]

    ; 向中断处理函数传递函数参数
    push eax
    ; 调用中断处理函数
    call [handler_table + eax * 4]

    ; 对应 push eax,调用结束恢复栈
    add esp, 4

    ; 恢复上下文寄存器信息
    ; 弹出通用寄存器
    popa
    pop gs
    pop fs
    pop es
    pop ds

    ; 对应push %1
    ; 对应error code 或 push magic
    add esp, 8
    iret

global interrupt_exit
interrupt_exit:

    ; 对应 push eax，调用结束恢复栈
    add esp, 4

    ; 恢复下文寄存器信息
    popa
    pop gs
    pop fs
    pop es
    pop ds

    ; 对应 push %1
    ; 对应 error code 或 push magic
    add esp, 8
    iret

INTERRUPT_HANDLER 0x00, 0; divide by zero
INTERRUPT_HANDLER 0x01, 0; debug
INTERRUPT_HANDLER 0x02, 0; non maskable interrupt
INTERRUPT_HANDLER 0x03, 0; breakpoint

INTERRUPT_HANDLER 0x04, 0; overflow
INTERRUPT_HANDLER 0x05, 0; bound range exceeded
INTERRUPT_HANDLER 0x06, 0; invalid opcode
INTERRUPT_HANDLER 0x07, 0; device not avilable

INTERRUPT_HANDLER 0x08, 1; double fault
INTERRUPT_HANDLER 0x09, 0; coprocessor segment overrun
INTERRUPT_HANDLER 0x0a, 1; invalid TSS
INTERRUPT_HANDLER 0x0b, 1; segment not present

INTERRUPT_HANDLER 0x0c, 1; stack segment fault
INTERRUPT_HANDLER 0x0d, 1; general protection fault
INTERRUPT_HANDLER 0x0e, 1; page fault
INTERRUPT_HANDLER 0x0f, 0; reserved

INTERRUPT_HANDLER 0x10, 0; x87 floating point exception
INTERRUPT_HANDLER 0x11, 1; alignment check
INTERRUPT_HANDLER 0x12, 0; machine check
INTERRUPT_HANDLER 0x13, 0; SIMD Floating - Point Exception

INTERRUPT_HANDLER 0x14, 0; Virtualization Exception
INTERRUPT_HANDLER 0x15, 1; Control Protection Exception
INTERRUPT_HANDLER 0x16, 0; reserved
INTERRUPT_HANDLER 0x17, 0; reserved

INTERRUPT_HANDLER 0x18, 0; reserved
INTERRUPT_HANDLER 0x19, 0; reserved
INTERRUPT_HANDLER 0x1a, 0; reserved
INTERRUPT_HANDLER 0x1b, 0; reserved

INTERRUPT_HANDLER 0x1c, 0; reserved
INTERRUPT_HANDLER 0x1d, 0; reserved
INTERRUPT_HANDLER 0x1e, 0; reserved
INTERRUPT_HANDLER 0x1f, 0; reserved

INTERRUPT_HANDLER 0x20, 0; clock 时钟中断
INTERRUPT_HANDLER 0x21, 0; keyboard 键盘中断
INTERRUPT_HANDLER 0x22, 0
INTERRUPT_HANDLER 0x23, 0; com2 串口2
INTERRUPT_HANDLER 0x24, 0; com1 串口1
INTERRUPT_HANDLER 0x25, 0
INTERRUPT_HANDLER 0x26, 0
INTERRUPT_HANDLER 0x27, 0
INTERRUPT_HANDLER 0x28, 0; rtc 实时时钟
INTERRUPT_HANDLER 0x29, 0
INTERRUPT_HANDLER 0x2a, 0
INTERRUPT_HANDLER 0x2b, 0
INTERRUPT_HANDLER 0x2c, 0
INTERRUPT_HANDLER 0x2d, 0
INTERRUPT_HANDLER 0x2e, 0; harddisk1 硬盘主通道
INTERRUPT_HANDLER 0x2f, 0; harddisk2 硬盘从通道

; 下面的数组记录了每个中断入口函数的指针
section .data
global handler_entry_table
handler_entry_table:
    dd interrupt_handler_0x00
    dd interrupt_handler_0x01
    dd interrupt_handler_0x02
    dd interrupt_handler_0x03
    dd interrupt_handler_0x04
    dd interrupt_handler_0x05
    dd interrupt_handler_0x06
    dd interrupt_handler_0x07
    dd interrupt_handler_0x08
    dd interrupt_handler_0x09
    dd interrupt_handler_0x0a
    dd interrupt_handler_0x0b
    dd interrupt_handler_0x0c
    dd interrupt_handler_0x0d
    dd interrupt_handler_0x0e
    dd interrupt_handler_0x0f
    dd interrupt_handler_0x10
    dd interrupt_handler_0x11
    dd interrupt_handler_0x12
    dd interrupt_handler_0x13
    dd interrupt_handler_0x14
    dd interrupt_handler_0x15
    dd interrupt_handler_0x16
    dd interrupt_handler_0x17
    dd interrupt_handler_0x18
    dd interrupt_handler_0x19
    dd interrupt_handler_0x1a
    dd interrupt_handler_0x1b
    dd interrupt_handler_0x1c
    dd interrupt_handler_0x1d
    dd interrupt_handler_0x1e
    dd interrupt_handler_0x1f
    dd interrupt_handler_0x20
    dd interrupt_handler_0x21
    dd interrupt_handler_0x22
    dd interrupt_handler_0x23
    dd interrupt_handler_0x24
    dd interrupt_handler_0x25
    dd interrupt_handler_0x26
    dd interrupt_handler_0x27
    dd interrupt_handler_0x28
    dd interrupt_handler_0x29
    dd interrupt_handler_0x2a
    dd interrupt_handler_0x2b
    dd interrupt_handler_0x2c
    dd interrupt_handler_0x2d
    dd interrupt_handler_0x2e
    dd interrupt_handler_0x2f




section .text

extern syscall_check
extern syscall_table
global syscall_handler
syscall_handler:
    xchg bx, bx

    ; 验证系统调用号
    push eax
    call syscall_check
    add esp, 4

    push 0x20222202

    push 0x80

    ; 保存上文寄存器信息
    push ds
    push es
    push fs
    push gs
    pusha

    push 0x80; 向中断处理函数传递参数中断向量 vector
    xchg bx, bx

    push edx; 第三个参数
    push ecx; 第二个参数
    push ebx; 第一个参数

    ; 调用系统调用处理函数，syscall_table 中存储了系统调用处理函数的指针
    call [syscall_table + eax * 4]

    xchg bx, bx
    add esp, 12; 系统调用结束恢复栈

    ; 修改栈中 eax 寄存器，设置系统调用返回值
    mov dword [esp + 8 * 4], eax

    ; 跳转到中断返回
    jmp interrupt_exit