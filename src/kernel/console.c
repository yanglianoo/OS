#include <onix/console.h>
#include <onix/io.h>
#include <onix/string.h>
#include <onix/interrupt.h>
/*
屏幕上所有字符以0为起始，默认为80*25模式，每行80个字符共25行
屏幕上可以容纳2000个字符，坐标值范围是0~1999
一个字符占用2个字节，所以光标乘以2后才是字符在显存中的位置
*/
#define CRT_ADDR_REG 0x3D4 // CRT(6845)地址寄存器 默认地址为 0x3D4
#define CRT_DATA_REG 0x3D5 // CRT(6845)数据寄存器 默认地址为 0x3D5

#define CRT_START_ADDR_H 0xC // 显示内存起始位置 - 高位
#define CRT_START_ADDR_L 0xD // 显示内存起始位置 - 低位
#define CRT_CURSOR_H 0xE     // 光标位置 - 储存光标的高8位
#define CRT_CURSOR_L 0xF     // 光标位置 - 储存光标的低8位

#define MEM_BASE 0xB8000              // 显卡内存起始位置
#define MEM_SIZE 0x4000               // 显卡内存大小
#define MEM_END (MEM_BASE + MEM_SIZE) // 显卡内存结束位置
#define WIDTH 80                      // 屏幕文本列数
#define HEIGHT 25                     // 屏幕文本行数
#define ROW_SIZE (WIDTH * 2)          // 每行字节数
#define SCR_SIZE (ROW_SIZE * HEIGHT)  // 屏幕字节数

#define ASCII_NUL 0x00
#define ASCII_ENQ 0x05
#define ASCII_BEL 0x07 // \a
#define ASCII_BS 0x08  // \b
#define ASCII_HT 0x09  // \t
#define ASCII_LF 0x0A  // \n
#define ASCII_VT 0x0B  // \v
#define ASCII_FF 0x0C  // \f
#define ASCII_CR 0x0D  // \r
#define ASCII_DEL 0x7F

static u32 screen; // 当前显示器开始的内存位置

static u32 pos; // 记录当前光标的内存位置

static u32 x, y; // 记录当前光标的坐标
/*
颜色属性由两个十六进制数字指定（颜色常量）
第一个对应于背景色，第二个对应于前景色。
每个数字可以为以下任何值:

    0 = 黑色       8 = 灰色
    1 = 蓝色       9 = 淡蓝色
    2 = 绿色       A = 淡绿色
    3 = 浅绿色     B = 淡浅绿色
    4 = 红色       C = 淡红色
    5 = 紫色       D = 淡紫色
    6 = 黄色       E = 淡黄色
    7 = 白色       F = 亮白色
————————————————
*/
static u8 attr = 7;        // 字符颜色样式，7代表为白色

static u16 erase = 0x0720; // 空格


//获得当前显示器的开始位置
static void get_screen()
{
    outb(CRT_ADDR_REG,CRT_START_ADDR_H);  //开始位置高地址
    screen = inb(CRT_DATA_REG) << 8;      //得到高8位
    outb(CRT_ADDR_REG,CRT_START_ADDR_L);  //开始位置低地址
    screen |= inb(CRT_DATA_REG);          //得到低8位

    screen <<=1;                         // screen *= 2
    screen += MEM_BASE;                  //加上显存初始位置
}
//设置当前显示器的开始位置
static void set_screen()
{
    outb(CRT_ADDR_REG,CRT_START_ADDR_H);  
    outb(CRT_DATA_REG,((screen-MEM_BASE)>>9) & 0xff);   //设置screen高8位
    outb(CRT_ADDR_REG,CRT_START_ADDR_L);
    outb(CRT_DATA_REG,((screen-MEM_BASE)>>1) & 0xff);   //设置screen低8位
}

//获得当前光标位置
static void get_cursor()
{
    //先往端口地址为0x3D4的Address Register寄存器中写入寄存器的索引
    //再从端口地址为0x3D5的Data Register读写数据
    outb(CRT_ADDR_REG,CRT_CURSOR_H);
    pos = inb(CRT_DATA_REG) << 8;    //获取光标高8位
    outb(CRT_ADDR_REG,CRT_CURSOR_L);
    pos |= inb(CRT_DATA_REG);       

    get_screen();

    pos <<= 1;
    pos += MEM_BASE;

    u32 delta = (pos- screen) >> 1;
    //获得光标绝对位置
    x = delta % WIDTH;
    y = delta / WIDTH;
}

//设置当前光标位置
static void set_cursor()
{
    outb(CRT_ADDR_REG,CRT_CURSOR_H);
    outb(CRT_DATA_REG,((pos - MEM_BASE)>>9) & 0xff); //设置光标高8位
    outb(CRT_ADDR_REG,CRT_CURSOR_L);
    outb(CRT_DATA_REG,((pos - MEM_BASE)>>1) & 0xff); //设置光标低8位
}
//清空屏幕
void console_clear()
{
    screen = MEM_BASE;
    pos = MEM_BASE;
    x = y = 0;
    set_cursor();
    set_screen();

    //将内存里的值全部改为空格
    u16 *ptr = (u16*) MEM_BASE;
    while (ptr < MEM_END)
    {
        *ptr++ = erase;
    }
    
}
//向左移动一个字符
static void command_bs()
{
    if (x)
    {
        x--;
        pos -=2;
        *(u16*)pos = erase;
    }
}
//删除当前的字符
static void command_del()
{

    *(u16*)pos = erase;

}
//回到开头的位置
static void command_cr()
{
    pos -= (x << 1);
    x = 0;

}
//滚屏操作
static void scroll_up()
{
    if(screen + SCR_SIZE + ROW_SIZE >= MEM_END)
    {
        memcpy((void*)MEM_BASE,(void*)screen,SCR_SIZE);
        pos -= (screen -MEM_BASE);
        screen = MEM_BASE;
    }

        u32 *ptr = (u32 *)(screen + SCR_SIZE);
        for (size_t i = 0; i < WIDTH; i++)
        {
            *ptr++ = erase;
        }
        screen += ROW_SIZE;
        pos += ROW_SIZE;
        set_screen();
}
//换行符 \n
static void command_lf()
{

   if(y + 1 <HEIGHT)
   {
    y ++;
    pos += ROW_SIZE;
    return;
   }
   scroll_up();

}

extern void start_beep();
//在显示器上写入字符串
void console_write(char *buf,u32 count)
{
    //禁止中断
    bool intr = interrupt_disable();
    
    char ch;
    char *ptr = (char *)pos;
    while (count--)
    {
        ch = *buf++;
        switch (ch)
        {
        case ASCII_NUL:
            break;
        case ASCII_ENQ:
            break;
        case ASCII_BEL:
            start_beep();
            break;
        case ASCII_BS:
            command_bs();
            break;
        case ASCII_HT:
            break;
        case ASCII_LF:
            command_lf();
            command_cr();
            break;
        case ASCII_VT:
            break;
        case ASCII_FF:
            command_lf();
            break;
        case ASCII_CR:
            command_cr();
            break;
        case ASCII_DEL:
            command_del();
            break;        
        default:
            if(x>=WIDTH)
            {
                x -= WIDTH;
                pos -= ROW_SIZE;
                command_lf();
            }
            //设置字符值，占一个字节
            *ptr++ = ch;
            //设置字符样式，占一个字节
            *ptr++ = attr;
            //光标往后移动一个字符，即两个字节
            pos += 2;
            x++;
            break;
        }
    }
    set_cursor(); 

    //恢复中断
    set_interrupt_state(intr);
}

void console_init()
{
    console_clear();
}