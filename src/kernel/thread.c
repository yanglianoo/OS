/**
 * @File Name: thread.c
 * @brief  创建的进程函数
 * @Author : Timer email:330070781@qq.com
 * @Version : 1.0
 * @Creat Date : 2023-05-15
 * 
 */
#include <onix/interrupt.h>
#include <onix/syscall.h>
#include <onix/debug.h>
#include <onix/thread.h>
#include <onix/mutex.h>
#include <onix/stdio.h>
#include <onix/arena.h>
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

/**
 * @brief  空闲进程
 */
void idle_thread()
{
    set_interrupt_state(true);
    u32 counter = 0;
    while (true)
    {
        // LOGK("idle task.... %d\n", counter++);
        asm volatile(
            "sti\n" // 开中断
            "hlt\n" // 关闭 CPU，进入暂停状态，等待外中断的到来
        );
        yield(); // 放弃执行权，调度执行其他任务
    }
}

void test_recursion()
{
    char tmp[0x400];
    test_recursion();
}
static void user_init_thread()
{
    u32 counter = 0;

    char ch;
    while (true)
    {
        // test();
        printf("task is in user mode %d\n", counter++);
        BMB;
        test_recursion();
        sleep(1000);
    }
}

void init_thread()
{
    // set_interrupt_state(true);
    char temp[100]; // 为栈顶有足够的空间
    task_to_user_mode(user_init_thread);
}

/**
 * @brief  测试进程
 */
void test_thread()
{
    set_interrupt_state(true);
    u32 counter = 0;

    while (true)
    {
        
        void *ptr = kmalloc(1200);
        LOGK("kmalloc 0x%p...\n",ptr);
        kfree(ptr);

        ptr = kmalloc(1024);
        LOGK("kmalloc 0x%p...\n",ptr);
        kfree(ptr);


        ptr = kmalloc(54);
        LOGK("kmalloc 0x%p...\n",ptr);
        kfree(ptr);

        sleep(5000);
    }
}