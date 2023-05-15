/**
 * @File Name: bitmap.c
 * @brief  ：位图bitmap，用于描述内存页的使用情况
 * @Author : Timer email:330070781@qq.com
 * @Version : 1.0
 * @Creat Date : 2023-04-09
 * 
 */
#include <onix/bitmap.h>
#include <onix/assert.h>
#include <onix/string.h>


// 构造位图
void bitmap_make(bitmap_t *map, char *bits, u32 length, u32 offest)
{
    map->bits = bits;
    map->length = length;
    map->offset = offest;
}


// 初始化位图
void bitmap_init(bitmap_t *map, char *bits, u32 length, u32 offest)
{
    memset(bits, 0 ,length);
    bitmap_make(map, bits, length, offest);
}

// 测试位图的某一位是否为 1
bool bitmap_test(bitmap_t *map, u32 index)
{
    assert(index >= map->offset);

    idx_t idx = index - map->offset;
/*
    如果 idx 是10，那么 bytes 将等于1（10 / 8 = 1），
    bits 将等于2（10 % 8 = 2）。
    这意味着第10个位是位图的第二个字节中的第3个位。
*/
    u32 bytes = idx / 8;

    u8 bits = idx % 8;

    assert(bytes < map->length);

    return (map->bits[bytes] & (1 << bits));
}

// 设置位图某位的值
void bitmap_set(bitmap_t *map, u32 index, bool value)
{
    assert(index >= map->offset);

    idx_t idx = index - map->offset;

    u32 bytes = idx / 8;

    u8 bits = idx % 8;

    if(value)
    {
        map->bits[bytes] |= (1 << bits);
    }
    else
    {
        map->bits[bytes] &= ~(1 << bits);
    }

}

/**
 * @brief  用于在位图（bitmap）中查找指定数量的连续空闲位（值为0的位），并将这些位标记为已使用（值为1）
 * @param  map: 位图指针
 * @param  count: 连续空闲个数
 * @return int: 
 */
int bitmap_scan(bitmap_t *map, u32 count)
{
    int start = EOF;
    u32 bits_left = map->length * 8; //初始化为位图总位数，用于记录还需要扫描的位数。
    u32 next_bit = 0;  //用于记录当前扫描到的位在位图中的索引。
    u32 counter = 0;   //用于记录已经扫描到的连续空闲位的数量

    while (bits_left-- > 0)
    {
        if(!bitmap_test(map,map->offset + next_bit))
        {
            counter++;
        }
        else
        {
            counter = 0;
        }

        next_bit++;

        if(counter == count)
        {
            start = next_bit - count;
            break;
        }
    }

    if(start == EOF)
        return EOF;
    
    bits_left = count;
    next_bit = start;
    //将找到的连续空闲位标记为已使用
    while (bits_left--)
    {
        bitmap_set(map,map->offset + next_bit, true);
        next_bit++;
    }
    
    return start + map->offset;
}

#include <onix/debug.h>

#define LOGK(fmt, args...)  DEBUGK(fmt, ##args)

#define LEN 2 //定义长度为两个字节
u8 buf[LEN];  //位图起始位置
bitmap_t map; //位图结构体

//位图测试代码
void bitmap_tests()
{
    bitmap_init(&map, buf, LEN,0);
    for (size_t i = 0; i < 33; i++)
    {
        idx_t idx = bitmap_scan(&map,1);
        if(idx == EOF)
        {
            LOGK("TEST FINISH\n");
            break;
        }
        LOGK("%d\n",idx);
    }
    
}
