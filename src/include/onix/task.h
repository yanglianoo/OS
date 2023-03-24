#ifndef ONIX_TASK_H
#define ONIX_TASK_H

#include <onix/types.h>

//定义了一个名为target_t的新类型。这个新类型是一个函数指针类型，指向一个无参数、返回值为u32类型的函数。
typedef u32 target_t(); 

//进程栈顶所在位置，最重要的信息:栈
typedef struct task_t
{
    u32 *stack; //内核栈
}task_t;

//ABI 的寄存器，进程切换之前保存，进程切换之后恢复
typedef struct task_frame_t
{
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    void (*eip)(void);
} task_frame_t;

void task_init();
void schedule();

#endif