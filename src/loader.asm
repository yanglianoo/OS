; 内核代码
[org 0x1000];内核代码起始地址为 0x1000

dw 0x55aa;魔数，用于判断错误


;打印字符串
mov si, loading
call print
xchg bx, bx
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
    xchg bx, bx
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

    ; mov si, detecting
    ; call print
    ; xchg bx, bx

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
;阻塞
jmp $

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
prepare_protect_mode:
    cli ;关闭中断
    ;打开A20线
    in al, 0x92
    or al, 0b10
    out 0x92, al

    lgdt [gdt_ptr] ;加载GDT

    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp word code_selector:protect_enable

    ud2;出错
[bits 32]
protect_enable:
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax;初始化数据段

    mov esp, 0x10000

    mov byte [0xb8000], 'P'

    xchg bx, bx

    mov byte [0x200000], 'P'

    xchg bx, bx


base equ 0
limit equ 0xfffff

code_selector equ (1 << 3)
data_selector equ (2 << 3)
gdt_ptr:
    dw (gdt_end - gdt_base -1)
    dd gdt_base
gdt_base: 
    dd 0,0
gdt_code:
    dw limit & 0xffff;
    dw base  & 0xffff;
    db (base >> 16) & 0xff;
    db 0b1110 | 0b0001_0000
    db 0b1100_0000 | (limit >> 16)
    db (base >> 24) & 0xff
gdt_data:
    dw limit & 0xffff;
    dw base  & 0xffff;
    db (base >> 16) & 0xff;
    db 0b0010 | 0b0001_0000
    db 0b1100_0000 | (limit >> 16)
    db (base >> 24) & 0xff
gdt_end:

;检测到的可使用的内存块的个数
ards_count: 
    dw 0
;内存检测缓存结构体
ards_buffer:

