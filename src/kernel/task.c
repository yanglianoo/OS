/**
 * @File Name: task.c
 * @brief  
 * @Author : Timer email:330070781@qq.com
 * @Version : 1.0
 * @Creat Date : 2023-04-11
 * 
 */
#include <onix/task.h>
#include <onix/printk.h>
#include <onix/debug.h>
#include <onix/bitmap.h>
#include <onix/memory.h>
#include <onix/interrupt.h>
#include <onix/assert.h>
#include <onix/onix.h>
#include <onix/string.h>



extern bitmap_t kernel_map;
extern void task_switch(task_t *next); 

#define NR_TASKS 64
static task_t *task_table[NR_TASKS]; //所有任务的数组

/**
 * @brief  从task_table里获得一个空闲的任务
 * @return task_t*: 
 */
static task_t *get_free_task()
{
    for (size_t i = 0; i < NR_TASKS; i++)
    {
        if (task_table[i] == NULL)
        {
            //分配一页内存，一页内存表示一个任务
            task_table[i] = (task_t *)alloc_kpage(1);
            return task_table[i];
        }
    }
    panic("No more tasks");
}

/**
 * @brief  从任务数组中查找某种状态的任务
 * @param  state: 任务状态
 * @return task_t*: 
 */
static task_t *task_search(task_state_t state)
{
    assert(!get_interrupt_state());
    task_t *task = NULL;
    task_t *current = running_task();

    for (size_t i = 0; i < NR_TASKS; i++)
    {
        task_t *ptr = task_table[i];
        if (ptr == NULL)
            continue;

        if (ptr->state != state)
            continue;
        if (current == ptr)
            continue;
        if (task == NULL || task->ticks < ptr->ticks || ptr->jiffies < task->jiffies)
            task = ptr;
    }

    return task;
}


//得到当前的TASK:将esp三位抹掉，得到页开始的位置
task_t *running_task()
{
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n");
}


/**
 * @brief  任务调度函数
 */
void schedule()
{
    task_t *current = running_task();

    task_t *next = task_search(TASK_READY);

    assert(next != NULL);
    assert(next->magic == ONIX_MAGIC);

    if(current->state == TASK_RUNNING)
    {
        //如果当前任务状态是执行，则将此任务状态置为就绪
        current->state = TASK_READY;
    }

    next->state = TASK_RUNNING;
    if(next == current)
        return;
    
    task_switch(next);
}


/**
 * @brief  创建任务
 * @param  target: 
 * @param  name: 任务名称
 * @param  priority: 任务优先级
 * @param  uid: 任务id
 * @return task_t*: 
 */
static task_t *task_create(target_t target, const char *name, u32 priority, u32 uid)
{
    //task为内核栈
    task_t *task = get_free_task();
    //初始化内核栈
    memset(task , 0, PAGE_SIZE);


    //栈顶 = 栈起始 + 一页 
    u32 stack = (u32)task + PAGE_SIZE; 

    //减去 task_frame_t 的大小
    stack -= sizeof(task_frame_t);

    task_frame_t *frame = (task_frame_t *)stack;
    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = (void *)target;

    strcpy((char *)task->name, name);

    
    task->stack = (u32 *)stack; //设置任务栈顶地址

    task->priority = priority; //设置任务优先级
    task->ticks = task->priority;
    task->jiffies = 0;
    task->state = TASK_READY;
    task->uid = uid;
    task->vmap = &kernel_map;
    task->pde = KERNEL_PAGE_DIR;
    task->magic = ONIX_MAGIC;

    return task;
}
// //创建任务，为任务分配栈空间
// static void task_create(task_t *task, target_t target)
// {
//     u32 stack = (u32)task + PAGE_SIZE; //栈顶 = 栈起始 + 一页 

//     stack -= sizeof(task_frame_t);     //减去 task_frame_t 的大小
//     task_frame_t *frame = (task_frame_t *)stack;
//     //保存寄存器的值

//     frame->ebx = 0x11111111;
//     frame->esi = 0x22222222;
//     frame->edi = 0x33333333;
//     frame->ebp = 0x44444444;

//     //函数return 后会跳转到这里去执行，存储了下一条将要执行的指令在内存中的地址
//     frame->eip = (void *)target; //指向要创建的任务

//     //当前task的栈区位置
//     task->stack = (u32 *)stack;
// }


// void task_init()
// {
//     task_create(a, thread_a); //创建task a的栈
//     task_create(b, thread_b); //创建task b的栈
//     schedule();
// }

static void task_setup()
{
    task_t *task = running_task();
    task->magic = ONIX_MAGIC;
    task->ticks = 1;

    memset(task_table, 0, sizeof(task_table));
}



u32  thread_a()
{
    set_interrupt_state(true);
    while (true)
    {
        printk("A");
    }
    
}

u32  thread_b()
{
    set_interrupt_state(true);
    while (true)
    {
        printk("B");
    }
    
}

u32  thread_c()
{
    set_interrupt_state(true);
    while (true)
    {
        printk("C");
    }
}

void task_init()
{
    task_setup();
    
    task_create(thread_a,"a",5, KERNEL_USER);
    task_create(thread_b,"b",5, KERNEL_USER);
    task_create(thread_c,"c",5, KERNEL_USER);
}

