#include <onix/global.h>
#include <onix/string.h>
#include <onix/debug.h>


descriptor_t gdt[GDT_SIZE]; //内核全局描述符表
pointer_t gdt_ptr;          //内核全局描述符表指针

//初始化内核全局描述符表
//GDT用于管理内存分段
/*
1.sgdt指令用于将GDT（全局描述符表）的基地址和限制值存储在指定的内存位置中。
  具体来说，它会将GDT寄存器的值（即GDTR）加载到指定的内存位置中。
2.lgdt指令与sgdt指令相反，它的作用是将指定内存地址中的数据加载到GDT寄存器中。
*/
void gdt_init()
{
    DEBUGK("init gdt!!!!\n");
    //将GDTR寄存器的值读入到 gdt_ptr中
    asm volatile("sgdt gdt_ptr");
    //拷贝loader中加载的GDT到内核定义的gdt中
    memcpy(&gdt, (void *)gdt_ptr.base, gdt_ptr.limit + 1);
    //设置新的gdtr的值，更改起始地址和长度界限
    gdt_ptr.base = (u32)&gdt;
    gdt_ptr.limit = sizeof(gdt) - 1;
    //加载将gdt_ptr内存中的值加载到GDTR寄存器中
    asm volatile("lgdt gdt_ptr\n");
}