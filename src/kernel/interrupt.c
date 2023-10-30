/**
 * @File Name: interrupt.c
 * @brief  
 * @Author : Timer email:330070781@qq.com
 * @Version : 1.0
 * @Creat Date : 2023-03-26
 * 
 */

#include <onix/interrupt.h>
#include <onix/global.h>
#include <onix/debug.h>
#include <onix/printk.h>
#include <onix/stdlib.h>
#include <onix/io.h>
#include <onix/task.h>
#include <onix/assert.h>
#define LOGK(fmt,args...) DEBUGK(fmt,##args)

#define ENTRY_SIZE 0x30

#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1
#define PIC_EOI 0x20

gate_t idt[IDT_SIZE];
pointer_t idt_ptr;
//中断向量表，里面存储的是每一个中断函数的入口地址
handler_t handler_table[IDT_SIZE];
//中断描述符表
extern handler_t handler_entry_table[ENTRY_SIZE];
//系统调用中断函数
extern void syscall_handler();
                     
extern void page_fault();

static char *messages[] = {
    "#DE Divide Error\0",
    "#DB RESERVED\0",
    "--  NMI Interrupt\0",
    "#BP Breakpoint\0",
    "#OF Overflow\0",
    "#BR BOUND Range Exceeded\0",
    "#UD Invalid Opcode (Undefined Opcode)\0",
    "#NM Device Not Available (No Math Coprocessor)\0",
    "#DF Double Fault\0",
    "    Coprocessor Segment Overrun (reserved)\0",
    "#TS Invalid TSS\0",
    "#NP Segment Not Present\0",
    "#SS Stack-Segment Fault\0",
    "#GP General Protection\0",
    "#PF Page Fault\0",
    "--  (Intel reserved. Do not use.)\0",
    "#MF x87 FPU Floating-Point Error (Math Fault)\0",
    "#AC Alignment Check\0",
    "#MC Machine Check\0",
    "#XF SIMD Floating-Point Exception\0",
    "#VE Virtualization Exception\0",
    "#CP Control Protection Exception\0",
};
void exception_handler(    int vector,
    u32 edi, u32 esi, u32 ebp, u32 esp,
    u32 ebx, u32 edx, u32 ecx, u32 eax,
    u32 gs, u32 fs, u32 es, u32 ds,
    u32 vector0, u32 error, u32 eip, u32 cs, u32 eflags)
{
    
    char *message = NULL;
    if(vector < 22)
    {
        message = messages[vector];
    }
    else
    {
        message = messages[15];
    }
    printk("\nEXCEPTION : %s \n", messages[vector]);
    printk("   VECTOR : 0x%02X\n", vector);
    printk("    ERROR : 0x%08X\n", error);
    printk("   EFLAGS : 0x%08X\n", eflags);
    printk("       CS : 0x%02X\n", cs);
    printk("      EIP : 0x%08X\n", eip);
    printk("      ESP : 0x%08X\n", esp);
    // 阻塞
    hang();
}


// 通知中断控制器，中断处理结束
void send_eoi(int vector)
{
    if (vector >= 0x20 && vector < 0x28)
    {
        outb(PIC_M_CTRL, PIC_EOI);
    }
    if (vector >= 0x28 && vector < 0x30)
    {
        outb(PIC_M_CTRL, PIC_EOI);
        outb(PIC_S_CTRL, PIC_EOI);
    }
}




void set_interrupt_handler(u32 irq, handler_t handler)
{
     //使用 assert 函数确保中断号在正确的范围内
    assert(irq >= 0 && irq < 16);
    handler_table[IRQ_MASTER_NR + irq] = handler;
}


/**
 * @brief  实现了中断屏蔽的功能,如果 enable 为 true，则将对应中断的屏蔽位置 0，否则将对应中断的屏蔽位置 1
 * @param  irq: 中断号
 * @param  enable: 布尔值 enable
 */
void set_interrupt_mask(u32 irq, bool enable)
{
    //使用 assert 函数确保中断号在正确的范围内
    assert(irq >= 0 && irq < 16);


    u16 port;
    //判断中断号为主片还是重片
    if (irq < 8)
    {
        port = PIC_M_DATA;
    }
    else
    {
        port = PIC_S_DATA;
        irq -= 8;
    }
    if (enable)
    {
        outb(port, inb(port) & ~(1 << irq)); //1左移 irq位，然后按位与
    }
    else
    {
        outb(port, inb(port) | (1 << irq)); //1左移 irq位，然后按位或
    }
}

void default_handler(int vector)
{
    send_eoi(vector);
   // schedule();
    LOGK(" [%d] default interrupt called %d...\n",vector);
}

// 初始化外中断控制器
void pic_init()
{
    outb(PIC_M_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_M_DATA, 0x20);       // ICW2: 起始中断向量号 0x20
    outb(PIC_M_DATA, 0b00000100); // ICW3: IR2接从片.
    outb(PIC_M_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_S_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_S_DATA, 0x28);       // ICW2: 起始中断向量号 0x28
    outb(PIC_S_DATA, 2);          // ICW3: 设置从片连接到主片的 IR2 引脚
    outb(PIC_S_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_M_DATA, 0b11111101); // 关闭所有中断
    outb(PIC_S_DATA, 0b11111111); // 关闭所有中断
}
void idt_init()
{
    //初始化中断描述符表
    for (size_t i = 0; i < IDT_SIZE; i++)
    {
        gate_t *gate = &idt[i];
        handler_t handler = handler_entry_table[i];
        gate->offset0 = (u32)handler & 0xffff;         //段内偏移 0 ~ 15位
        gate->offset1 = ((u32)handler >> 16) & 0xffff; //段内偏移 16 ~ 31位
        gate->selector = 1 << 3;    //代码段选择子
        gate->reserved = 0;         //保留不用
        gate->type = 0b1110;        
        gate->DPL = 0;              // 使用 int 指令访问的最低权限
        gate->present = 1;          // 是否有效
    }

    for(size_t i = 0; i < 0x20; i++)
    {
        handler_table[i] = exception_handler;
    }

    handler_table[0xe] = page_fault;
    
    for (size_t i = 0x20; i < ENTRY_SIZE; i++)
    {
        handler_table[i] = default_handler;
    }



    // 初始化系统调用，从0x80开始为系统调用中断
    gate_t *gate = &idt[0x80];
    gate->offset0 = (u32)syscall_handler & 0xffff;
    gate->offset1 = ((u32)syscall_handler >> 16) & 0xffff;
    gate->selector = 1 << 3; // 代码段
    gate->reserved = 0;      // 保留不用
    gate->type = 0b1110;     // 中断门
    gate->segment = 0;       // 系统段
    gate->DPL = 3;           // 用户态
    gate->present = 1;       // 有效
    //加载中断描述符表
    idt_ptr.base = (u32)idt;
    idt_ptr.limit = sizeof(idt) - 1;

    asm volatile("lidt idt_ptr\n");
}

//清除 IF 位，返回设置之前的值
bool interrupt_disable()  
{
    asm volatile(
        "pushfl\n"
        "cli\n"
        "popl %eax\n"
        "shrl $9, %eax\n"
        "andl $1, %eax\n"
    );
}

//获得IF位
bool get_interrupt_state()
{
    asm volatile(
        "pushfl\n"
        "popl %eax\n"
        "shrl $9, %eax\n"
        "andl $1, %eax\n"
    );
}

//设置IF位
void set_interrupt_state(bool state)
{
    if(state)
        asm volatile("sti\n");
    else
        asm volatile("cli\n");
}

void interrupt_init()
{
    pic_init();
    idt_init();
}



