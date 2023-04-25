#include <onix/interrupt.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/syscall.h>

#define LOGK(fmt,args...) DEBUGK(fmt, ##args)

//支持 最大64个系统调用
#define SYSCALL_SIZE 64

handler_t syscall_table[SYSCALL_SIZE];



void syscall_check(u32 nr)
{
    if( nr >= SYSCALL_SIZE)
    {
        panic("syscall nr error!!!");
    }
}



static u32 sys_default()
{
    panic("syscall not implemented!!!");
}

static u32 sys_test()
{
    LOGK("syscall test...\n");
    return 255;
}

extern void task_yield();

void syscall_init()
{
    for (size_t i = 0; i < SYSCALL_SIZE; i++)
    {
        syscall_table[i] = sys_default;
    }
    // 0 号系统调用为 test
    syscall_table[SYS_NR_TEST] = sys_test;
    // 1 号系统调用为 yield
    syscall_table[SYS_NR_YIELD] = task_yield;
}

