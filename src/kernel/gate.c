/**
 * @File Name: gate.c
 * @brief  系统调用
 * @Author : Timer email:330070781@qq.com
 * @Version : 1.0
 * @Creat Date : 2023-05-15
 * 
 */
#include <onix/interrupt.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/syscall.h>
#include <onix/task.h>
#include <onix/console.h>
#define LOGK(fmt,args...) DEBUGK(fmt, ##args)

//支持 最大64个系统调用
#define SYSCALL_SIZE 64

// 系统调用函数数组
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


task_t *task = NULL;
static u32 sys_test()
{
    //LOGK("syscall test...\n");
    if(!task)
    {
        task = running_task();

        task_block(task, NULL, TASK_BLOCKED);
    }
    else
    {
        task_unblock(task);

        task = NULL;
    }
    return 255;
}

int32 sys_write(fd_t fd, char *buf, u32 len)
{
    if (fd == stdout || fd == stderr)
    {
        return console_write(buf, len);
    }
    // todo
    panic("write!!!!");
    return 0;
}


void syscall_init()
{
    // 初始化所有系统调用，支持 64 个系统调用
    for (size_t i = 0; i < SYSCALL_SIZE; i++)
    {
        syscall_table[i] = sys_default;
    }
    
    syscall_table[SYS_NR_TEST] = sys_test;

    syscall_table[SYS_NR_SLEEP] = task_sleep;
    
    syscall_table[SYS_NR_YIELD] = task_yield;

    syscall_table[SYS_NR_WRITE] = sys_write;
}

