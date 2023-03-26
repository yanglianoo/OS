#include <onix/io.h>
#include <onix/interrupt.h>
#include <onix/assert.h>
#include <onix/debug.h>

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

void clock_handler(int vector)
{
    assert(vector == 0x20);
    send_eoi(vector);

    jiffies++;
    DEBUGK("clock jiffies %d ...\n", jiffies);
}

void pit_init()
{   

    outb(PIT_CTRL_REG, 0b00110100);
    //输出计数器的值
    outb(PIT_CHAN0_REG,CLOCK_COUNTER & 0xff); //低字节
    outb(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff); //高字节
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