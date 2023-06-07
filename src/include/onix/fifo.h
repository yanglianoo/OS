#ifndef ONIX_FIFO_H
#define ONIX_FIFO_H

#include <onix/types.h>

/* 循环队列 */
typedef struct fifo_t
{
    char *buf;   //用于存储数据的缓冲区数组
    u32 length;  //缓冲区的长度
    u32 head;    // 指向缓冲区中下一个可写入的位置    
    u32 tail;    // 指向缓冲区中下一个可读取的位置
} fifo_t;

void fifo_init(fifo_t *fifo, char *buf, u32 length);
bool fifo_full(fifo_t *fifo);
bool fifo_empty(fifo_t *fifo);
char fifo_get(fifo_t *fifo);
void fifo_put(fifo_t *fifo, char byte);

#endif