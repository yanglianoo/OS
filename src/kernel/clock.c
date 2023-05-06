#include <onix/io.h>
#include <onix/interrupt.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/task.h>
#include <onix/onix.h>
//计数器的端口号
#define PIT_CHAN0_REG 0X40
#define PIT_CHAN2_REG 0X42
//控制字寄存器
#define PIT_CTRL_REG 0X43


#define HZ 100   //中断频率100HZ
#define OSCILLATOR 1193182  //原始振荡器频率
#define CLOCK_COUNTER (OSCILLATOR / HZ) 
#define JIFFY (1000 / HZ)

#define SPEAKER_REG 0x61
#define BEEP_HZ 440
#define BEEP_COUNTER (OSCILLATOR / BEEP_HZ)

u32 volatile jiffies = 0;
u32 jiffy = JIFFY;

u32 volatile beeping = 0;

void start_beep()
{
    if(!beeping)
    {
        outb(SPEAKER_REG,inb(SPEAKER_REG) | 0b11);
    }
    beeping = jiffies + 5;
}

void stop_beep()
{
    if(beeping && jiffies > beeping)
    {
        outb(SPEAKER_REG, inb(SPEAKER_REG) & 0xfc);
        beeping = 0;
    }
}
void clock_handler(int vector)
{
    assert(vector == 0x20);
    send_eoi(vector);
    stop_beep();
    //全局时间片加加
    jiffies++;
    // DEBUGK("jiffiles = %d\n",jiffies);
    
    //在时钟中断中进行任务切换
    task_t *task = running_task();
    assert(task->magic == ONIX_MAGIC);

    task->jiffies = jiffies;
    task->ticks--;
    if(!task->ticks)
    {
        schedule();
    }
}

void pit_init()
{   
    //配置计数器 0 时钟
    outb(PIT_CTRL_REG, 0b00110100);
    //输出计数器的值
    outb(PIT_CHAN0_REG,CLOCK_COUNTER & 0xff); //低字节
    outb(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff); //高字节

    //配置计数器 2 蜂鸣器时钟
    outb(PIT_CTRL_REG, 0b10110110);
    outb(PIT_CHAN2_REG, (u8)BEEP_COUNTER);
    outb(PIT_CHAN2_REG,(u8)(BEEP_COUNTER >> 8));

}

void clock_init()
{
    //初始化计数器
    pit_init();
    //设置中断向量号
    set_interrupt_handler(IRQ_CLOCK, clock_handler);
    //打开中断
    set_interrupt_mask(IRQ_CLOCK, true);
}