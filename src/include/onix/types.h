#ifndef ONIX_TYPES_H
#define ONIX_TYPES_H

#define EOF -1  
#define NULL ((void *)0)
#define EOS '\0'

#ifndef __cplusplus
#define bool _Bool
#define true 1
#define false 0
#endif

//用于定义特殊的结构体，声明结构体不需要字节对齐
#define _packed __attribute__((packed))

//表示函数在编译时使用“省略框架指针”(omit-frame-pointer)优化选项
#define _ofp __attribute__((optimize("omit-frame-pointer")))

//它使用了 __attribute__((always_inline)) 属性，意味着在编译时将总是强制内联该函数。
#define _inline __attribute__((always_inline)) inline

// 定义无符号整型
typedef unsigned int size_t;
// 定义有符号的整型
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

// 定义无符号的整型
typedef unsigned char u8;       // 8bit
typedef unsigned short u16;     // 16bit
typedef unsigned int u32;       // 32bit
typedef unsigned long long u64; // 64bit

typedef u32 time_t;
typedef u32 idx_t;

/* 文件描述符 */
typedef int32 fd_t;
typedef enum std_fd_t
{
    stdin,  
    stdout,
    stderr,
} std_fd_t;

#endif