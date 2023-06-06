; 内核代码
[org 0x1000];内核代码起始地址为 0x1000

dw 0x55aa;魔数，用于判断错误


;打印字符串
mov si, loading
call print

;检测内存代码
detect_memory:
    ;将ebx置为0
    xor ebx, ebx
    ;es:di 结构体的缓存位置
    mov ax, 0
    mov es, ax
    mov edi, ards_buffer
    ;固定签名
    mov edx, 0x534d4150
.next:
    ;子功能号
    mov eax, 0xe820
    ; ards 结构的大小 20个字节
    mov ecx, 20
    ;调用 0x15 系统调用 
    int 0x15
    ;如果cf置位就表示出错
    jc error
    ;将缓存指针指向下一个结构体
    add di, cx
    ;将结构体数量加一
    inc dword [ards_count]

    cmp ebx, 0
    jnz .next

    mov si, detecting
    call print

    jmp prepare_protect_mode
    ; 结构体数量
;     mov cx, [ards_count]
;     ;结构体指针
;     mov si, 0
; .show:
;     mov eax, [si + ards_buffer]
;     mov ebx, [si + ards_buffer + 8]
;     mov edx, [si + ards_buffer + 16]
;     add si, 20
;     xchg bx, bx
;     loop .show
prepare_protect_mode:
    cli ;关闭中断

    ;打开A20线 
    in al, 0x92
    or al, 0b10
    out 0x92, al

    lgdt [gdt_ptr] ;加载GDT
 
    ;启动保护模式，cr0寄存器第零位置1
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ;刷新流水线
    jmp dword code_selector:protect_mode



print:
    mov ah, 0x0e
.next:
    mov al, [si]
    cmp al, 0
    jz .done
    int 0x10
    inc si
    jmp .next
.done:
    ret

loading:
    db "Loading Onix...", 10, 13, 0; 10代表 \n\r
detecting:
    db "Detecting Memory Success...", 10, 13, 0
    
error:
    mov si, .msg
    call print
    hlt;让cpu停止
    jmp $
    .msg db "Loading Error!!!", 10,13,0


;告诉编译器将如下的代码编译成32位的
[bits 32]
protect_mode:
    ;xchg bx, bx
    
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax;用选择子初始化各段寄存器

    mov esp, 0x10000;修改栈顶

    mov edi, 0x10000;读取的目标内存
    mov ecx, 10;起始扇区
    mov bl, 200;扇区数量
    call read_disk

    mov eax,0x20220205; 内核魔数
    mov ebx,ards_count; ards 数量指针

    jmp dword code_selector:0x10040

    ud2; 

;阻塞
jmp $

read_disk:
    ;设置读写的扇区的数量
    mov dx, 0x1f2
    mov al, bl ; al=bl=1 
    out dx, al ; 将al输出到dx，配置0x1f2的值为1，设置读写的扇区数量为1

    inc dx; 0x1f3,inc 为加1的指令
    mov al, cl ;起始扇区的前8位
    out dx, al

    inc dx; 0x1f4,inc 为加1的指令
    shr ecx, 8 ;将ecx右移8位，即将中8位移动到低8位的地方
    mov al, cl ;起始扇区的中8位
    out dx, al

    inc dx; 0x1f5,inc 为加1的指令
    shr ecx, 8 ;再将ecx右移8位，将最开始的高8位移动到低8位的地方
    mov al, cl ;起始扇区的前高8位
    out dx, al

    inc dx; 0x1f6
    shr ecx, 8
    and cl, 0b1111;将cl寄存器的高四位置为0 

    mov al, 0b1110_0000;
    or  al, cl               ;00000000 or 11100000 = 11100000
    out dx, al; 主盘 - LBA模式

    inc dx; 0x1f7
    mov al, 0x20; 读硬盘
    out dx, al

    xor ecx, ecx;异或，清空ecx
    mov cl, bl;得到读写扇区的数量 ,此时为1个

    .read:
        push cx;保存cx
        call .waits; 等待数据准备完毕
        call .reads; 读取一个扇区
        pop cx;恢复cx 
        loop .read
    ret

    .waits:
        mov dx, 0x1f7
        .check:
            in al,dx;将0x1f7地址中的数据读入al
            jmp $+2;直接跳转到下一行，相当于一点点延迟
            jmp $+2
            jmp $+2
            and al, 0b1000_1000;只保留al的第7位与第3位
            cmp al, 0b0000_1000;如果数据准备完毕，硬盘不繁忙，即第3位为1，第7位为0
            jnz .check
        ret
    .reads:
        mov dx, 0x1f0
        mov cx, 256; 一个扇区是256个字
        .readw:
            in ax, dx
            jmp $+2;直接跳转到下一行，相当于一点点延迟
            jmp $+2
            jmp $+2
            mov [edi], ax
            add edi, 2
            loop .readw
        ret

;内存开始的位置：基地址 从0 开始
memory_base equ 0 
;内存界限 4G/4K -1
memory_limit equ ((1024 * 1024 * 1024 * 4) /(1024 * 4)) -1  ;1024 * 1024 = 1M  1M * 1024 = 1G

code_selector equ (1 << 3)  ;代码段选择子，1左移三位： 1000 ，最后三位为0，代表选择第一个GDT
data_selector equ (2 << 3)  ;数据段选择子，2左移三位： 10000，最后三位为0，代表选择第二个GDT

;GDTR寄存器，通过 lgdt[gdt_ptr] 加载到GDTR寄存器里去
gdt_ptr:
    dw (gdt_end - gdt_base ) - 1 ;界限
    dd gdt_base   ;GDT内存起始地址

;GDT表，8字节一个，默认第0个为NULL
; typedef struct descriptor /* 共 8 个字节 */
; {
;     unsigned short limit_low;      // 段界限 0 ~ 15 位
;     unsigned int base_low : 24;    // 基地址 0 ~ 23 位 16M
;     unsigned char type : 4;        // 段类型
;     unsigned char segment : 1;     // 1 表示代码段或数据段，0 表示系统段
;     unsigned char DPL : 2;         // Descriptor Privilege Level 描述符特权等级 0 ~ 3
;     unsigned char present : 1;     // 存在位，1 在内存中，0 在磁盘上
;     unsigned char limit_high : 4;  // 段界限 16 ~ 19;
;     unsigned char available : 1;   // 该安排的都安排了，送给操作系统吧
;     unsigned char long_mode : 1;   // 64 位扩展标志
;     unsigned char big : 1;         // 32 位 还是 16 位;
;     unsigned char granularity : 1; // 粒度 4KB 或 1B
;     unsigned char base_high;       // 基地址 24 ~ 31 位
; } __attribute__((packed)) descriptor;
gdt_base: 
    dd 0,0; NULL描述符
gdt_code:;代码段定义
    dw memory_limit & 0xffff;段界限的 0 ~ 15位
    dw memory_base  & 0xffff;基地址的0 ~ 15位
    db (memory_base >> 16) & 0xff; 基地址的 16 ~ 23 位
    ; 存在 - dlp 0 - S _ 代码 - 非依从 - 可读 - 没有被访问过
    db 0b1_00_1_1_0_1_0
    ; 4k - 32 位 - 不是 64 位 - 段界限 16 ~ 19
    db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xf;
    db (memory_base >> 24) & 0xff ; 基地址 24 ~ 31 位
gdt_data:;数据段定义
    dw memory_limit & 0xffff;段界限的 0 ~ 15位
    dw memory_base  & 0xffff;基地址的0 ~ 15位
    db (memory_base >> 16) & 0xff; 基地址的 16 ~ 23 位
    ; 存在 - dlp 0 - S _ 数据 - 向上 - 可写 - 没有被访问过
    db 0b1_00_1_0_0_1_0
    ; 4k - 32 位 - 不是 64 位 - 段界限 16 ~ 19
    db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xf;
    db (memory_base >> 24) & 0xff ; 基地址 24 ~ 31 位
gdt_end:

;检测到的可使用的内存块的个数
ards_count: 
    dd 0
;内存检测缓存结构体
ards_buffer:

