#include <onix/memory.h>
#include <onix/types.h>
#include <onix/debug.h>
#include <onix/assert.h>
#include <onix/onix.h>
#include <onix/stdlib.h>
#include <onix/string.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define ZONE_VALID 1   //ards  可用内存区域
#define ZONE_RESERVED 2 //ards 不可用区域

/**
 * @brief  宏函数 IDX 用于获取给定地址 addr 所在的物理页的索引。
 *         在 x86 架构下，一个物理页的大小通常是 4KB（2^12），
 *         因此 IDX 将给定的地址右移 12 位（相当于除以 4096），
 *         然后强制类型转换为 32 位无符号整数，得到该地址所在的页索引。
 */
#define IDX(addr) ((u32)addr >> 12)  //获取 addr 的页索引

#define PAGE(idx) ((u32)idx << 12)  //获取页索引 idx 对应的页的开始的位置


/**
 * @brief  在宏函数的定义中，& 运算符用于将地址 addr 的低 12 位与二进制数 0xfff 进行按位与操作，
 *         得到低 12 位的值，如果这个值不等于 0，就说明该地址不是一个物理页的起始地址
 */
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

typedef struct ards_t
{
    u64 base; //内存基地址 64位 8字节
    u64 size; //内存长度   64位 8字节
    u32 type; //类型       32位 4字节
} _packed ards_t;


static u32 memory_base = 0;  //可用内存基地址，应该等于 1M
static u32 memory_size = 0;  //可用内存大小
static u32 total_pages = 0;  //所有内存页数
static u32 free_pages = 0;   //空闲内存页数

#define used_pages (total_pages - free_pages) //已用页数


/**
 * @brief  初始化内存地址，将loader检测到的可用内存结构体读出来
 * @param  magic: 内核魔数
 * @param  addr:  实际位addr_count的值。将此地址 +4 即是 addr_buffer 的指针
 */
void memory_init(u32 magic, u32 addr)
{
    u32 count = 0;
    ards_t *ptr;


    if(magic == ONIX_MAGIC)
    {
        count = *(u32*)addr;
        ptr = (ards_t *)(addr + 4);
        for (size_t i = 0; i < count; i++,ptr++)
        {
            LOGK("Memory base 0x%p size 0x%p type 0x%p \n",
            (u32)ptr->base,(u32)ptr->size,(u32)ptr->type);
            if(ptr->type == ZONE_VALID && ptr->size > memory_size)
            {
                memory_base = (u32)ptr->base;
                memory_size = (u32)ptr->size;
            }
        }
        
    }
    else
    {
        panic("Memory init magic unknown 0x%p\n",magic);
    }

    LOGK("ARDS count %d\n",count);
    LOGK("Memory base 0x%p\n",(u32)memory_base);
    LOGK("Memory size 0x%p\n",(u32)memory_size);

    assert(memory_base == MEMORY_BASE);
    assert((memory_size & 0xfff) == 0);

    total_pages = IDX(memory_size) + IDX(MEMORY_BASE);
    free_pages = IDX(memory_size);

    LOGK("Total pages %d\n",total_pages);
    LOGK("Free pages %d\n",free_pages);
}


static u32 start_page = 0;    //可分配物理内存起始位置
static u8 *memory_map;       //物理内存数组
static u32 memory_map_pages;  //物理内存数组占用的页数

/**
 * @brief  初始化内存映射表，标记已经被占用的内存页
 * 
 */
void memory_map_init()
{
    //初始化物理内存数组
    memory_map = (u8*)memory_base;

    // 计算物理内存数组占用的页数
    //总共8160页 每一页大小为4K 因此物理内存数组大小为 8160/4K 
    memory_map_pages = div_round_up(total_pages, PAGE_SIZE);
    LOGK("Memory map page count %d\n", memory_map_pages);

    //空闲页减少
    free_pages -= memory_map_pages;

    // 清空物理内存数组
    memset((void *)memory_map, 0 , memory_map_pages * PAGE_SIZE);

    // 前 1M 的内存位置 以及 物理内存数组已占用的页，已被占用
    start_page = IDX(MEMORY_BASE) + memory_map_pages;
    for (size_t i = 0; i < start_page; i++)
    {
        memory_map[i] = 1;
    }

    LOGK("Total pages %d free pages %d Start pages %d \n",total_pages,free_pages,start_page);
}


//分配一页物理内存
static u32 get_page()
{
    for (size_t i = start_page; i < total_pages; i++)
    {
        if(!memory_map[i])
        {
            //将此页标记为已被占用
            memory_map[i] = 1;
            free_pages--;
            assert(free_pages >= 0);
            //左移12位就是 乘以 4K
            u32 page = ((u32)i) << 12;
            LOGK("GET page 0x%p\n", page);
            return page;
        }
    }
    panic("Out of memory!!!");
}

//释放一页物理内存
static void put_page(u32 addr)
{
    //判断传入地址是否为页起始地址
    ASSERT_PAGE(addr);

    //获取页索引
    u32 idx = IDX(addr);

    assert(idx >= start_page && idx < total_pages);

    assert(memory_map[idx] >= 1);

    memory_map[idx]--;

    if(!memory_map[idx])
    {
        free_pages++;
    }

    assert(free_pages >0 && free_pages < total_pages);
    LOGK("PUT page 0x%p\n",addr);
}


void memory_test()
{
    u32 pages[10];
    for (size_t i = 0; i < 10; i++)
    {
        pages[i] = get_page();
    }

    for (size_t i = 0; i < 10; i++)
    {
        put_page(pages[i]);
    }
    
}






