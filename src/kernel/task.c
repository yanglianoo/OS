#include <onix/task.h>
#include <onix/printk.h>
#include <onix/debug.h>

#define PAGE_SIZE 0x1000  //一页=4Kb
/*
ABI的寄存器，进程切换之前保存，进程切换之后恢复
eax：存的是返回值，肯定会改变
*/
task_t *a = (task_t *)0x1000;
task_t *b = (task_t *)0x2000;

extern void task_switch(task_t *next);

//得到当前的TASK:将esp三位抹掉，得到页开始的位置
task_t *running_task()
{
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n"
    );
}
//任务调度
void schedule()
{
    task_t *current = running_task();
    //交替切换
    task_t *next = current == a ? b : a;
    task_switch(next);
}

u32 thread_a()
{
    while (true)
    {
        printk("A");
        schedule();
    }
    
}

u32 thread_b()
{
    while (true)
    {
        printk("B");
        schedule();
    }
    
}

//创建任务，为任务分配栈空间
static void task_create(task_t *task, target_t target)
{
    u32 stack = (u32)task + PAGE_SIZE; //栈顶 = 栈起始 + 一页 

    stack -= sizeof(task_frame_t);     //减去 task_frame_t 的大小
    task_frame_t *frame = (task_frame_t *)stack;
    //保存寄存器的值

    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    
    //函数return 后会跳转到这里去执行，存储了下一条将要执行的指令在内存中的地址
    frame->eip = (void *)target; //指向要创建的任务

    //当前task的栈区位置
    task->stack = (u32 *)stack;
}


void task_init()
{
    task_create(a, thread_a); //创建task a的栈
    task_create(b, thread_b); //创建task b的栈
    schedule();
}