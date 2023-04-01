#include <onix/onix.h>
#include <onix/types.h>
#include <onix/io.h>
#include <onix/string.h>
#include <onix/console.h>
#include <onix/stdarg.h>
#include <onix/printk.h>
#include <onix/debug.h>
#include <onix/global.h>
#include <onix/task.h>
#include <onix/interrupt.h>
#include <onix/stdlib.h>
#include <onix/time.h>
#include <onix/rtc.h>
void kernel_init()
{

    memory_map_init();
    mapping_init();
    //中断初始化
    interrupt_init();
    // //时钟初始化
    // clock_init();
    // //时间戳初始化
    // time_init();
    // //实时时钟初始化
    // rtc_init();
    // memory_test();
    

    BMB;

    char *ptr = (char *)(0x100000 * 20);

    ptr[0] = 'a';

    // asm volatile("sti");
    hang();

    return ;

}