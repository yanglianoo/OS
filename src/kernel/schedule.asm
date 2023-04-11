[bits 32]

section .text
; 切换任务的函数
global task_switch
task_switch:
    ; 保存栈帧，将sp的值存到bp寄存器中
    push ebp
    mov ebp, esp
    ;保存ABI需要的寄存器的值
    push ebx
    push esi
    push edi
    ;当前这个任务的信息
    mov eax, esp;
    and eax, 0xfffff000; current

    ;保存当前线程的栈顶，当进程被切换后，下一次执行此进程时能恢复栈顶
    mov [eax], esp
    
    ;压入了两个局部变量：current，next 通过偏移获得变量值，next为下一个函数栈的入口地址
    mov eax, [ebp + 8]

    ;将sp指针切换到next线程的栈顶
    mov esp, [eax]  
    ; 恢复寄存器的值
    pop edi
    pop esi
    pop ebx
    pop ebp

    ;return时会跳到ip指针指向的位置继续执行程序
    ret  