#ifndef ONIX_TASK_H
#define ONIX_TASK_H

#include <onix/types.h>
#include <onix/bitmap.h>

#define KERNEL_USER 0
#define NORMAL_USER 1

#define TASK_NAME_LEN 16

//定义了一个名为target_t的新类型。这个新类型是一个函数指针类型，指向一个无参数、返回值为u32类型的函数。
typedef u32 target_t(); 

//进程状态枚举
typedef enum task_state_t
{
    TASK_INIT,      //初始化
    TASK_RUNNING,   //执行
    TASK_READY,     //就绪
    TASK_BLOCKED,   //阻塞
    TASK_SLEEPING,  //睡眠
    TASK_WAITING,   //等待
    TASK_DIED,      //死亡
} task_state_t;

//进程栈的属性结构体
typedef struct task_t
{
    u32 *stack;             // 内核栈
    task_state_t state;     // 任务状态
    u32 priority;           // 任务优先级
    u32 ticks;              // 剩余时间片
    u32 jiffies;            // 上次执行时全局时间片
    char name[TASK_NAME_LEN];// 任务名
    u32 uid;                // 用户 id
    u32 pde;                // 页目录物理地址
    struct bitmap_t *vmap;  // 进程虚拟内存位图
    u32 magic;              // 内核魔数，用于检测栈溢出
} task_t;

//ABI 的寄存器，进程切换之前保存，进程切换之后恢复
typedef struct task_frame_t
{
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    void (*eip)(void);
} task_frame_t;

task_t *running_task();
void schedule();
void task_init();

void task_yield();
#endif