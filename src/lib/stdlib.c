#include <onix/stdlib.h>

// 延迟
void delay(u32 count)
{
    while (count--)
        ;
}

void hang()
{
    while (true)
        ;
    
}


