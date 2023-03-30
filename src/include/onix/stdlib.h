#ifndef  ONIX_STDLIB_H
#define  ONIX_STDLIB_H
#include <onix/types.h>

#define MAX(a,b) (a < b ? b : a )
#define MIN(a,b) (a > b ? b : a )


u32 div_round_up(u32 num, u32 size);
// 延迟
void delay(u32 count);
// 阻塞
void hang();

u8 bcd_to_bin(u8 value);
u8 bin_to_bcd(u8 value);
#endif