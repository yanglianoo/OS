[org 0x7c00] ;0x7c00为魔法数，boot的起始地址

;设置屏幕模式为文本模式
mov  ax,3
int 0x10

;初始化段寄存器
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00

mov si, booting
call print



mov edi, 0x1000;读取的目标内存
mov ecx, 2;起始扇区
mov bl, 4;扇区数量
call reda_disk

cmp word [0x1000], 0x55aa
jnz error
jmp 0x1002

jmp error
; 阻塞
jmp $

reda_disk:
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

booting:
    db "Booting Onix...", 10, 13, 0; 10代表 \n\r

error:
    mov si, .msg
    call print
    hlt;让cpu停止
    jmp $
    .msg db "Booting Error!!!", 10,13,0


; 填充 0
times 510 - ($ - $$) db 0

;主引导扇区的最后两个字节必须是 0x55,0xaa
db 0x55,0xaa