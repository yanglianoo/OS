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
#include <onix/thread.h>
#include <onix/global.h>
#define NR_TASKS 64

extern u32 volatile jiffies;
extern u32 jiffy;

extern bitmap_t kernel_map;
extern tss_t tss;

extern void task_switch(task_t *next); 


static task_t *task_table[NR_TASKS]; //所有任务的数组

static list_t block_list;            //任务默认阻塞链表  
static list_t sleep_list;            //睡眠任务链表

static task_t *idle_task;            //空闲任务指针

/**
 * @brief  从task_table里获得一个空闲的任务
 * @return task_t*: 空闲任务的指针
 */
static task_t *get_free_task()
{
    //当创建一个任务时，会对task_table[i] 分配内存
    for (size_t i = 0; i < NR_TASKS; i++)
    {
        if (task_table[i] == NULL)
        {
            //分配一页内存，一页内存表示一个任务，顺序分配
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
        //判断任务状态，如果不等书传进来的状态，则继续搜寻
        if (ptr->state != state)
            continue;
        //搜寻到当前任务，跳过
        if (current == ptr)
            continue;
        // 比较任务剩余时间片 和 全局时间片
        if (task == NULL || task->ticks < ptr->ticks || ptr->jiffies < task->jiffies)
            task = ptr;
    }

    //如果找不到可运行的进程，则运行空闲进程
    if(task == NULL && state == TASK_READY)
    {
        task = idle_task;
    }
    return task;
}


// 激活任务,维护全局唯一的tss段，如果下一个切换的任务的是用户态任务，则将下一个任务的栈顶指针保存到esp0中
void task_activate(task_t *task)
{
    assert(task->magic == ONIX_MAGIC);

    if (task->uid != KERNEL_USER)
    {
        tss.esp0 = (u32)task + PAGE_SIZE;
    }
}

/**
 * @brief  得到当前的TASK:将esp三位抹掉，得到页开始的位置
 * @return task_t*: 当前页的起始位置放置在 eax 中
 */
task_t *running_task()
{
    asm volatile(
        "movl %esp, %eax\n"             //将当前栈顶指针保存到eax中
        "andl $0xfffff000, %eax\n");    //按位与运算将 eax 中的值的最后 12 位（即页内偏移）清零，得到其所在页的起始位置
}


/**
 * @brief  任务调度函数，调度到下一个任务，重置当前任务属性
 */
void schedule()
{
    assert(!get_interrupt_state());  //不可中断

    //获取当前进程的指针
    task_t *current = running_task();

    //得到下一个准备执行进程的指针
    task_t *next = task_search(TASK_READY);

    assert(next != NULL);
    assert(next->magic == ONIX_MAGIC);

    //如果当前任务状态是执行，则将此任务状态置为就绪
    if(current->state == TASK_RUNNING)
    {
        
        current->state = TASK_READY;
    }

    //如果当前任务的时间片为零了，则重新赋值该时间片
    if (!current->ticks)
    {
        current->ticks = current->priority;
    }
    
    //将下一个任务状态置为 TASK_RUNNING
    next->state = TASK_RUNNING;
    if(next == current)
        return;
    /* 有问题 我觉得应该是保存当前内核态的栈指针 */
    task_activate(next);
    //切换到下一个任务，此函数用汇编实现，遵循x86 的ABI规定
    task_switch(next);
}
 

/**
 * @brief  创建任务
 * @param  target: 
 * @param  name: 任务名称
 * @param  priority: 任务优先级
 * @param  uid: 任务id
 * @return task_t*: 任务指针
 */
static task_t *task_create(target_t target, const char *name, u32 priority, u32 uid)
{
    //task为内核栈，为当前创建的任务分配内核栈内存，内存大小为1页
    task_t *task = get_free_task();
    //初始化内核栈中的值全部为 0
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



// 任务阻塞
void task_block(task_t *task, list_t *blist, task_state_t state)
{
    assert(!get_interrupt_state());
    
    assert(task->node.next == NULL);
    assert(task->node.prev == NULL);

    if (blist == NULL)
    {
        blist = &block_list;
    }

    list_push(blist, &task->node);

    assert(state != TASK_READY && state != TASK_RUNNING);

    task->state = state;

    task_t *current = running_task();
    if (current == task)
    {
        schedule();
    }
}

// 解除任务阻塞
void task_unblock(task_t *task)
{
    assert(!get_interrupt_state());

    list_remove(&task->node);

    assert(task->node.next == NULL);
    assert(task->node.prev == NULL);

    task->state = TASK_READY;
}




/**
 * @brief  睡眠系统调用函数
 * @param  ms: 睡眠时间参数
 */
void task_sleep(u32 ms)
{
    assert(!get_interrupt_state());  //不可中断

    
    u32 ticks = ms / jiffy;  //计算睡眠时间需要的时间片
    ticks = ticks > 0 ? ticks : 1;

    task_t *current = running_task();
    current->ticks = jiffies + ticks;


    list_t *list = &sleep_list;
    list_node_t *anchor = &list->tail;

    for (list_node_t *ptr = list->head.next; ptr != &list->tail; ptr = ptr->next)
    {
        task_t *task = element_entry(task_t, node, ptr);

        if (task->ticks > current->ticks)
        {
            anchor = ptr;
            break;
        }
    }

    assert(current->node.next == NULL);
    assert(current->node.prev == NULL);

    list_insert_before(anchor, &current->node);

    current->state = TASK_SLEEPING;

    //
    schedule();
}

/**
 * @brief  唤醒任务
 */
void task_wakeup()
{
    assert(!get_interrupt_state()); // 不可中断

    // 从睡眠链表中找到 ticks 小于等于 jiffies 的任务，恢复执行
    list_t *list = &sleep_list;
    for (list_node_t *ptr = list->head.next; ptr != &list->tail;)
    {
        task_t *task = element_entry(task_t, node, ptr);
        if (task->ticks > jiffies)
        {
            break;
        }

        // unblock 会将指针清空
        ptr = ptr->next;

        task->ticks = 0;
        task_unblock(task);
    }
}

/**
 * @brief  主动放弃 CPU 执行权
 */
void task_yield()
{
    schedule();
}

// 调用该函数的地方不能有任何局部变量
// 调用前栈顶需要准备足够的空间
void task_to_user_mode(target_t target)
{
    task_t *task = running_task();

    u32 addr = (u32)task + PAGE_SIZE;

    addr -= sizeof(intr_frame_t);
    intr_frame_t *iframe = (intr_frame_t *)(addr);

    iframe->vector = 0x20;
    iframe->edi = 1;
    iframe->esi = 2;
    iframe->ebp = 3;
    iframe->esp_dummy = 4;
    iframe->ebx = 5;
    iframe->edx = 6;
    iframe->ecx = 7;
    iframe->eax = 8;

    iframe->gs = 0;
    iframe->ds = USER_DATA_SELECTOR;
    iframe->es = USER_DATA_SELECTOR;
    iframe->fs = USER_DATA_SELECTOR;
    iframe->ss = USER_DATA_SELECTOR;
    iframe->cs = USER_CODE_SELECTOR;

    iframe->error = ONIX_MAGIC;

    u32 stack3 = alloc_kpage(1); // todo replace to user stack

    iframe->eip = (u32)target;
    iframe->eflags = (0 << 12 | 0b10 | 1 << 9);
    iframe->esp = stack3 + PAGE_SIZE;

    asm volatile(
        "movl %0, %%esp\n"
        "jmp interrupt_exit\n" ::"m"(iframe));
}

/**
 * @brief  初始化
 */
static void task_setup()
{
    task_t *task = running_task();
    task->magic = ONIX_MAGIC;
    task->ticks = 1;

    memset(task_table, 0, sizeof(task_table));
}

void task_init()
{
    //初始化阻塞任务链表 和睡眠任务链表
    list_init(&block_list);
    list_init(&sleep_list);

    //初始化任务
    task_setup();
    
    //分别占用5个时间片，时间片代表优先级，所占用时间片越多，优先级越高
    
    idle_task = task_create(idle_thread, "idle", 1, KERNEL_USER); //内核态进程
    task_create(init_thread, "init", 5, NORMAL_USER);   //用户态进程
    task_create(test_thread, "test", 5, KERNEL_USER);   //内核态进程
}

/**
 * @brief  bug出现的原因
 * 在c语言中 字符常量 和 字符串常量是不一样的
 * 在C语言中，字符常量使用单引号（' '）括起来，而字符串常量使用双引号（" "）括起来。
 * 'a' 是 int 类型，而不是指针类型
 * 字符常量是一个单个的字符，而字符串常量是一个以空字符('\0')结尾的字符数组
 */