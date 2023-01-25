; 内核代码
[org 0x1000];内核代码起始地址为 0x1000

dw 0x55aa;魔数，用于判断错误


;打印字符串
mov si, loading
call print
xchg bx, bx
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

    mov si, detecting
    call print
    xchg bx, bx

    ;结构体数量
    mov cx, [ards_count]
    ;结构体指针
    mov si, 0
.show:
    mov eax, [si + ards_buffer]
    mov ebx, [si + ards_buffer + 8]
    mov edx, [si + ards_buffer + 16]
    add si, 20
    xchg bx, bx
    loop .show
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

ards_count:
    dw 0
ards_buffer:
