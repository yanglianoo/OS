#include <onix/interrupt.h>
#include <onix/debug.h>

#define LOGK(fmt,args...) DEBUGK(fmt,##args)

extern void memory_map_init();
extern void mapping_init();
extern void interrupt_init();
extern void clock_init();
extern void time_init();
extern void rtc_init();
extern void task_init();
extern void syscall_init();
extern void hang();


void kernel_init()
{

    //初始化内存映射表
    memory_map_init();
    //初始化内存映射
    mapping_init();
    //中断初始化
    interrupt_init();
    // //时钟初始化
    clock_init();
    // //时间戳初始化
    // time_init();
    // //实时时钟初始化
    // rtc_init();
    //memory_test();
    
    // bitmap_tests();
    task_init();
    syscall_init();
    set_interrupt_state(true);

    // asm volatile("sti");
    //hang();


}