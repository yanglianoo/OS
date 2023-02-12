[bits 32]

section .text ;代码段

global inb ;将 inb 导出
inb:
    push  ebp
    mov   ebp, esp ;保存栈帧,栈帧为栈的入口地址

    xor eax,eax ;将eax清空
    mov edx, [ebp + 8]   ;port
    in al, dx;       将端口号 dx 的 8bit 输入到 ax

    jmp $+2 ;
    jmp $+2 ;
    jmp $+2 ;

    leave ;恢复栈帧
    ret

global outb 
outb:
    push  ebp
    mov   ebp, esp ;保存栈帧,栈帧为栈的入口地址

    xor eax,eax ;将eax清空
    mov edx, [ebp + 8]   ;port
    mov eax, [ebp + 12]  ;value
    out dx, al;       将al中的8bit输出到端口号 dx

    jmp $+2 ;
    jmp $+2 ;
    jmp $+2 ;

    leave ;恢复栈帧
    ret

global inw ;将 inb 导出
inw:
    push  ebp
    mov   ebp, esp ;保存栈帧,栈帧为栈的入口地址

    xor eax,eax ;将eax清空
    mov edx, [ebp + 8]   ;port
    in ax, dx;       将端口号 dx 的 8bit 输入到 ax

    jmp $+2 ;
    jmp $+2 ;
    jmp $+2 ;

    leave ;恢复栈帧
    ret

global outw 
outw:
    push  ebp
    mov   ebp, esp ;保存栈帧,栈帧为栈的入口地址

    xor eax,eax ;将eax清空
    mov edx, [ebp + 8]   ;port
    mov eax, [ebp + 12]  ;value
    out dx, ax;       将ax中的8bit输出到端口号 dx

    jmp $+2 ;
    jmp $+2 ;
    jmp $+2 ;

    leave ;恢复栈帧
    ret
