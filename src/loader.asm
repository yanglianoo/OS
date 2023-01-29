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
    inc word [ards_count]

    cmp ebx, 0
    jnz .next

    mov si, detecting
    call print
    xchg bx, bx

    jmp prepare_protect_mode
;     ;结构体数量
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
    xchg bx, bx
    cli ;关闭中断

    ;打开A20线
    in al, 0x92
    or al, 0b10
    out 0x92, al

    lgdt [gdt_ptr] ;加载GDT
 
    ;启动保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
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



[bits 32]
protect_mode:
    xchg bx, bx
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax;初始化段寄存器

    mov esp, 0x10000;修改栈顶
    mov byte [0xb8000], 'P'
    mov byte [0x200000], 'P'
;阻塞
jmp $

;内存开始的位置：基地址 从0 开始
memory_base equ 0 
;内存界限 4G/4K -1
memory_limit equ ((1024 * 1024 * 1024 * 4) /(1024 * 4)) -1

code_selector equ (1 << 3)
data_selector equ (2 << 3)
gdt_ptr:
    dw (gdt_end - gdt_base ) - 1
    dd gdt_base
gdt_base: 
    dd 0,0; NULL描述符
gdt_code:
    dw memory_limit & 0xffff;段界限的 0 ~ 15位
    dw memory_base  & 0xffff;基地址的0 ~ 15位
    db (memory_base >> 16) & 0xff; 基地址的 16 ~ 23 位
    ; 存在 - dlp 0 - S _ 代码 - 非依从 - 可读 - 没有被访问过
    db 0b1_00_1_1_0_1_0
    ; 4k - 32 位 - 不是 64 位 - 段界限 16 ~ 19
    db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xf;
    db (memory_base >> 24) & 0xff ; 基地址 24 ~ 31 位
gdt_data:
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
    dw 0
;内存检测缓存结构体
ards_buffer:

