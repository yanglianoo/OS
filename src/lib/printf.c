/**
 * @File Name: printf.c
 * @brief  
 * @Author : Timer email:330070781@qq.com
 * @Version : 1.0
 * @Creat Date : 2023-06-30
 * 
 */
#include <onix/stdarg.h>
#include <onix/stdio.h>
#include <onix/syscall.h>

static char buf[1024];

int printf(const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);

    i = vsprintf(buf, fmt, args);

    va_end(args);

    write(stdout, buf, i);

    return i;
}