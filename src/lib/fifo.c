#include <onix/fifo.h>
#include <onix/assert.h>

/* 循环队列初始化 */
void fifo_init(fifo_t *fifo, char *buf, u32 length)
{
    fifo->buf = buf;
    fifo->length = length;
    fifo->head = 0;
    fifo->tail = 0;
}

static _inline u32 fifo_next(fifo_t *fifo, u32 pos)
{
    return (pos + 1) % fifo->length;
}

bool fifo_full(fifo_t *fifo)
{
    bool full = (fifo_next(fifo, fifo->head) == fifo->tail);
    return full;
}


bool fifo_empty(fifo_t *fifo)
{
    return (fifo->head == fifo->tail);
}


char fifo_get(fifo_t *fifo)
{
    assert(!fifo_empty(fifo));
    /* 从 tail 位置获取一个字节*/
    char byte = fifo->buf[fifo->tail];
    /* tail 加一 */
    fifo->tail = fifo_next(fifo, fifo->tail);
    /* 返回读取到的字节 */
    return byte;
}


void fifo_put(fifo_t *fifo, char byte)
{
    /* 当缓冲区满时，调用 fifo_get 函数*/
    while (fifo_full(fifo))
    {
        /* 当缓冲区满时，重置 tail */
        fifo_get(fifo);
    }
    /* 将byte放入fifo->head位*/
    fifo->buf[fifo->head] = byte;
    /* fifo->head 加一*/
    fifo->head = fifo_next(fifo, fifo->head);
   // printk("head:%d tail:%d\n",fifo->head,fifo->tail);
}