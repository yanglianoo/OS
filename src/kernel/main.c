#include <onix/onix.h>
#include <onix/types.h>
#include <onix/io.h>
#define CRT_ADDR_REG 0x3d4   //CRT 地址寄存器端口
#define CRT_DATA_REG 0x3d5   //CRT 数据寄存器端口

#define CRT_CURSOR_H 0xe    //CRT光标位置高位
#define CRT_CURSOR_L 0xf    //CRT光标位置地位

void kernel_init()
{
    outb(CRT_ADDR_REG,CRT_CURSOR_H);
    u16 pos = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG,CRT_CURSOR_L);
    pos |= inb(CRT_DATA_REG);

    outb(CRT_ADDR_REG,CRT_CURSOR_H);
    outb(CRT_DATA_REG,0);
    outb(CRT_ADDR_REG,CRT_CURSOR_L);
    outb(CRT_DATA_REG,123);
    return ;
    
}