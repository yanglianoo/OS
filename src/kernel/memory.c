#include <onix/memory.h>
#include <onix/types.h>
#include <onix/debug.h>
#include <onix/assert.h>
#include <onix/onix.h>
#include <onix/stdlib.h>
#include <onix/string.h>
#include <onix/bitmap.h>
#include <onix/printk.h>
#include <onix/multiboot2.h>
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

//type = 1 代表可用  type = 2 代表不可用
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

#define DIDX(addr) (((u32)addr >> 22) & 0x3ff)  //获取 addr的页目录索引

#define TIDX(addr) (((u32)addr >> 12) & 0x3ff)  //获取 addr 的页表索引


/**
 * @brief  在宏函数的定义中，& 运算符用于将地址 addr 的低 12 位与二进制数 0xfff 进行按位与操作，
 *         得到低 12 位的值，如果这个值不等于 0，就说明该地址不是一个物理页的起始地址
 */
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

#define KERNEL_MAP_BITS 0x4000 //位图存放位置


//内核映射内存大小 8M
#define KERNEL_MEMORY_SIZE (0x100000 * sizeof(KERNEL_PAGE_TABLE))

bitmap_t kernel_map;
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

    // 如果是 onix loader 进入的内核
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
    else if(magic == MULTIBOOT2_MAGIC )
    {
        u32 size = *(unsigned int *)addr;
        multi_tag_t *tag = (multi_tag_t *)(addr + 8);

        LOGK("Announced mbi size 0x%x\n", size);
        while (tag->type != MULTIBOOT_TAG_TYPE_END)
        {
            if (tag->type == MULTIBOOT_TAG_TYPE_MMAP)
                break;
            // 下一个 tag 对齐到了 8 字节
            tag = (multi_tag_t *)((u32)tag + ((tag->size + 7) & ~7));
        }

        multi_tag_mmap_t *mtag = (multi_tag_mmap_t *)tag;
        multi_mmap_entry_t *entry = mtag->entries;
        while ((u32)entry < (u32)tag + tag->size)
        {
            LOGK("Memory base 0x%p size 0x%p type %d\n",
                 (u32)entry->addr, (u32)entry->len, (u32)entry->type);
            count++;
            if (entry->type == ZONE_VALID && entry->len > memory_size)
            {
                memory_base = (u32)entry->addr;
                memory_size = (u32)entry->len;
            }
            entry = (multi_mmap_entry_t *)((u32)entry + mtag->entry_size);
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


    // memory_base =  0x100000
    // Total pages = 8160
    // Free pages = 7904

    total_pages = IDX(memory_size) + IDX(MEMORY_BASE);
    free_pages = IDX(memory_size);

    LOGK("Total pages %d\n",total_pages);
    LOGK("Free pages %d\n",free_pages);

    if(memory_size < KERNEL_MEMORY_SIZE)
    {
        panic("System memory is %dM too small, at least %dM needed\n",
              memory_size / MEMORY_BASE, KERNEL_MEMORY_SIZE / MEMORY_BASE);
    }
}


static u32 start_page = 0;    //可分配物理内存起始位置
static u8 *memory_map;        //物理内存数组
static u32 memory_map_pages;  //物理内存数组占用的页数

/**
 * @brief  初始化内存映射表，标记已经被占用的内存页
 * 
 */
void memory_map_init()
{
    //初始化物理内存数组,  memory_map = 0x100000
    memory_map = (u8*)memory_base;

    // 计算物理内存数组占用的页数 memory_map_pages = 2页
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

    LOGK("Total pages:%d free pages:%d Start pages:%d Start pages address:0x%p\n",total_pages,free_pages,start_page,start_page*PAGE_SIZE);

    //初始化内核虚拟内存位图，需要8位对齐,内核内存需要 8M ,前1M地址不可用 length = 224位 
    u32 length = (IDX(KERNEL_MEMORY_SIZE) - IDX(MEMORY_BASE)) / 8;
    LOGK("kernel_map length:%d\n",length);
    //初始化位图，将位图 存放在 0x4000处
    bitmap_init(&kernel_map, (u8 *)KERNEL_MAP_BITS,length,IDX(MEMORY_BASE));
    
    bitmap_scan(&kernel_map, memory_map_pages);

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

// 得到 cr3 寄存器
u32 get_cr3()
{
    // 直接将 mov eax, cr3，返回值在 eax 中
    asm volatile("movl %cr3, %eax\n");
}

// 设置 cr3 寄存器，参数是页目录的地址
void set_cr3(u32 pde)
{
    ASSERT_PAGE(pde);
    asm volatile("movl %%eax, %%cr3\n" ::"a"(pde));
}

//将 cr0 寄存器最高位 PG 置为1，启用分页
static _inline void enable_page()
{
    asm volatile(
        "movl %cr0, %eax\n"
        "orl $0x80000000, %eax\n"
        "movl %eax, %cr0\n"
    );
}

/**
 * @brief  初始化页表项 和 页目录项 都可用这个函数
 * @param  entry: 页表项 或者 页目录项的指针
 * @param  index: 页表物理页地址 || 物理页地址
 */
static void entry_init(page_entry_t *entry, u32 index)
{
    *(u32 *)entry = 0;
    entry->present = 1;
    entry->write = 1;
    entry->user = 1;
    entry->index = index;
}

/**
 * @brief  初始化内存映射，将页目录放在0x1000处
 * @brief  初始化了两个页表，0x2000 , 0x3000
 */

void mapping_init()
{
    //地址映射关系计算：
    //1. 0x1000 + 虚拟地址高10位 -> 找到页表地址  这里只映射了前两位，假设现在 我找到了 0x2000
    //2. 0x2000 + 虚拟地址中10位 -> 找到实际物理页地址 
    //3. 实际物理页地址 + 虚拟地址低12位 = 实际物理地址
    // PDE:页目录项
    // PTE:页表项

    //初始化页目录项，起始页目录项指向内核页地址：0x1000
    page_entry_t *pde = (page_entry_t *)KERNEL_PAGE_DIR;
    //把页目录项全部初始化为0
    memset(pde,0,PAGE_SIZE);

#if 1

    idx_t index = 0;

    // kernel占两页,分配两个页表  0x2000 0x3000
    for (idx_t didx = 0; didx < (sizeof(KERNEL_PAGE_TABLE) / 4); didx++)
    {

        //得到一个物理页地址 KERNEL_PAGE_TABLE[didx]  0x2000 0x3000
        page_entry_t *pte = (page_entry_t *)KERNEL_PAGE_TABLE[didx];
        //将一个物理页置为0
        memset(pte, 0, PAGE_SIZE);

        //得到第一个页目录项地址，对 pde[didx] 取地址,页目录的起始地址为 0x1000
        page_entry_t *dentry = &pde[didx];
        //初始化页目录项
        entry_init(dentry, IDX((u32)pte));
        dentry->user = 0; // 只能被内核访问

        for (idx_t tidx = 0; tidx < 1024; tidx++, index++)
        {
            // 第 0 页不映射，为造成空指针访问，缺页异常，便于排错
            if (index == 0)
                continue;

            page_entry_t *tentry = &pte[tidx];
            //初始化页表项，每个页表有1024各页表项，为0x2000 和 0x3000开始的两个页表进行初始化
            entry_init(tentry, index);
            //这里初始化 index = 0~1024 没太懂
            tentry->user = 0;      // 只能被内核访问
            memory_map[index] = 1; // 设置物理内存数组，该页被占用
        }
    }

    // 将最后一个页表指向页目录自己，方便修改
    page_entry_t *entry = &pde[1023];
    entry_init(entry, IDX(KERNEL_PAGE_DIR));

#else  // 内存映射测试代码
    //页目录项第一项的值设为0 , 将页表物理页地址设为 KERNEL_PAGE_ENTRY
    #define KERNEL_PAGE_ENTRY 0x201000
    entry_init(&pde[0],IDX(KERNEL_PAGE_ENTRY));

    //初始化页表,页表的起始地址为 KERNEL_PAGE_ENTRY
    page_entry_t *pte = (page_entry_t *)KERNEL_PAGE_ENTRY;
    //清空页表
    memset(pte,0,PAGE_SIZE);

    page_entry_t *entry;
    for (size_t tidx = 0; tidx < 1024; tidx++)
    {
        //设置该页表
        entry = &pte[tidx];
        //初始化页表项,物理偏移地址设为 tidx ,即 index = tidx
        entry_init(entry, tidx);
        //标记该页被占用,一页为 4K, 1024个页 ,占用 4M
        memory_map[tidx] = 1;
    }
#endif

    //设置cr3寄存器，将pde写入cr3寄存器
    set_cr3((u32)pde);
    //启用分页机制
    enable_page();  
}
/**
这两个数值在操作系统中通常用于虚拟内存管理，
其中 0xfffff000 是内核空间的最高地址，用于存放页目录；
0xffc00000 是内核空间中的固定地址，用于存放页表。在 x86 架构的系统中，一个页表可以映射 4MB 的虚拟地址空间。
因此，对于一个给定的虚拟地址，要找到它所在的页表，需要将虚拟地址的高10位（即目录项索引）左移12位，得到页表的物理地址，然后加上 0xffc00000。
 */
//获取页目录 pde ，将目录地址定位到 0xfffff000 通过虚拟地址映射 刚好能映射回 0x1000的物理地址
static page_entry_t *get_pde()
{
    // 1111 1111 11 11 1111 1111 0000 0000 0000
    // 0x1000 + 1111 1111 11 = 0x1FFC -> 0x1007
    // 0x1000 + 1111 1111 11 = 0x1FFC -> 0x1007
    // 0x1000 + 0000 0000 0000 = 0x1000 刚好映射回去

    return (page_entry_t *)(0xfffff000);
}
//获取页表 pte
static page_entry_t *get_pte(u32 vaddr)
{

    return (page_entry_t *)(0xffc00000 | (DIDX(vaddr) << 12 ));
}

/**
处理器中的 TLB 是一种高速缓存，它存储了虚拟地址空间到物理内存之间的映射关系，以提高地址转换的速度。
在使用页表进行地址映射时，处理器会先在 TLB 中查找虚拟地址对应的物理地址，
如果在 TLB 中找到了对应的映射关系，就可以直接进行地址转换，从而提高系统的运行效率。
当操作系统修改了某个虚拟页面的映射关系时，需要刷新该虚拟页面所在的页表项在 TLB 中的缓存。
否则，当处理器访问该虚拟页面时，会使用已经过期的页表项进行地址转换，导致地址转换错误或者访问了错误的物理页面
 */
// 刷新虚拟地址 vaddr 的 块表 TLB
void flush_tlb(u32 vaddr)
{
    asm volatile("invlpg (%0)" ::"r"(vaddr)
                 : "memory");
}

// 从位图中扫描 count 个连续的页
static u32 scan_page(bitmap_t *map, u32 count)
{
    assert(count > 0);

    int32 index = bitmap_scan(map, count);

    if(index == EOF)
    {
        panic("Scan page fail!!!");
    }
    u32 addr = PAGE(index);
    LOGK("Scan page 0x%p count %d\n",addr,count);
    return addr;

}

// 与 scan_page 相对，重置相应的页
static void reset_page(bitmap_t *map, u32 addr, u32 count)
{
    ASSERT_PAGE(addr);
    assert(count > 0);
    u32 index = IDX(addr);

    for (size_t i = 0; i < count; i++)
    {
        assert(bitmap_test(map, index + i));
        bitmap_set(map, index + i, 0);
    }
}

// 分配 count 个连续的内核页
u32 alloc_kpage(u32 count)
{
    assert(count > 0);
    u32 vaddr = scan_page(&kernel_map, count);
    LOGK("ALLOC kernel pages 0x%p count %d\n", vaddr, count);
    return vaddr;
}

// 释放 count 个连续的内核页
void free_kpage(u32 vaddr, u32 count)
{
    ASSERT_PAGE(vaddr);
    assert(count > 0);
    reset_page(&kernel_map, vaddr, count);
    LOGK("FREE  kernel pages 0x%p count %d\n", vaddr, count);
}



#if 0
void memory_test()
{
    u32 *pages = (u32 *)(0x200000);
    u32 count = 0x6fe;
    for(size_t i=0; i < count; i++)
    {
        
        pages[i] = alloc_kpage(1);
        LOGK("0x%x\n",i);
    }

    for (size_t i = 0; i < count; i++)
    {
        free_kpage(pages[i],1);
    }
}
#else
void memory_test()
{


    // 将 20 M 0x1400000 内存映射到 64M 0x4000000 的位置

    // 我们还需要一个页表，0x900000

    
    u32 vaddr = 0x4000000; // 线性地址几乎可以是任意的，虚拟地址
    u32 paddr = 0x1400000; // 物理地址必须要确定存在
    u32 table = 0x900000;  // 页表也必须是物理地址

    page_entry_t *pde = get_pde();
    printk("pde:0x%p\n",pde);
    //获取到vaddr对应的页目录项的地址，指向了对应的页表，虚拟地址高10位为页目录表的偏移地址
    page_entry_t *dentry = &pde[DIDX(vaddr)];
    printk("dentry:0x%p\n",dentry);
    //初始化vaddr页目录的页目录项，0x1000+0x10= 0x1010 这一项指向的页表物理地址为 0x900000 IDX(0x900000) = 0x900 第0x900页
    entry_init(dentry, IDX(table));
    printk("*dentry:0x%p\n",*dentry);
    //获取到页表所在物理地址 0x900 ，页表存在 0x900000处
    page_entry_t *pte = get_pte(vaddr); 
    printk("pte:0x%p\n",pte);
    printk("*pte:0x%p\n",*pte);
    //获取到实际物理页地址 TIDX(vaddr) = 0 ，将 tentry->pte[0]
    page_entry_t *tentry = &pte[TIDX(vaddr)];
    printk("TIDX(vaddr):0x%p\n",TIDX(vaddr));
    printk("tentry:0x%p\n",tentry);
    //设置 pte[0] = IDX(paddr) = IDX(0x1400000) = 0x1400 页
    entry_init(tentry, IDX(paddr));
    printk("*tentry:0x%p\n",*tentry);


    // 0x4000000 = 0000 0100 0000 0000 0000 0000 0000 0000
    // 高10位 = 0x10 页目录项地址为 ： 0x1000 + 0x10 = 0x1010 -> 0x900
    // 中10位 = 0x0  页表项地址为  ： 0x900 + 0 = 0x900 -> 0x1400 
    // 低10位 = 0x0  最后的物理地址为 ： 0x1400 + 0 = 0x1400 页
    char *ptr = (char *)(0x4000000);
    ptr[0] = 'a';
    entry_init(tentry, IDX(0x1500000));

    //刷新页表
    flush_tlb(vaddr);
    ptr[2] = 'b';

}

// void memory_test()
// {
//     u32 *pages = (u32 *)(0x200000);
//     u32 count = 0x6fe;
//     for (size_t i = 0; i < count; i++)
//     {
//         pages[i] = alloc_kpage(1);
//         LOGK("0x%x\n",i);
//     }

//     for (size_t i = 0; i < count; i++)
//     {
//         free_kpage(pages[i],1);
//     }
    
// }
#endif
