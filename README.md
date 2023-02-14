# OS开发
- 开发环境：
    - `ubuntu20.04.5 WSL`
    - `sudo apt install nasm`:安装汇编编译器 nasm
    - `sudo apt install bochs-x`:安装虚拟机 bochs
    - `sudo apt-get install qemu-system`   #下载安装可以模拟全部硬件的qemu
    - `sudo apt install gdb` 安装gdb调试器
    - `sudo apt-get install gcc-multilib`&&`sudo apt-get install g++-multilib` 安装在64位的机器上产生32位的程序

- 参考书籍:
    - 操作系统真相还原
    - 30天自制操作系统
    - Orange'S:一个操作系统的实现

**常用寄存器：**

![image-20230128173803324](image/image-20230128173803324.png)

| 16位寄存器 |      功能      | 高8位 | 低8位 |
| :--------: | :------------: | ----- | ------ |
|     AX     |   累加寄存器   | AH    | AL     |
|     CX     |   计数寄存器   | CH    | CL     |
|     DX     |   数据寄存器   | DH    | DL     |
|     BX     |   基址寄存器   | BH    | BL     |
|     SP     |  栈指针寄存器  |       |        |
|     BP     |  基指针寄存器  |       |        |
|     SI     |  源变址寄存器  |       |        |
|     DI     | 目的变址寄存器 |       |        |

- `EAX，ECX，EDX，EBX，ESP，EBP，ESI，EDI `为32位寄存器，寓意为`Extend`，在16位寄存器前加`E`。

| 段寄存器 |     功能     |
| :------: | :----------: |
|    ES    | 附件段寄存器 |
|    CS    | 代码段寄存器 |
|    SS    |  栈段寄存器  |
|    DS    | 数据段寄存器 |
|    FS    |   没有名称   |
|    GS    |   没有名称   |
## 常用汇编指令
 - `equ`: equ是nasm提供的伪指令，意为equal，即等于，指令格式为：符号名称 equ 表达式
 - `dw`：word，双字节，写入双字节即16个bit
 - `db`：bite，单字节，写入单字节即8个bit
 - `dword`: double word,四个字节，32bit
 - `org`: 伪指令，规定程序的起始地址，若不指定则程序默认从`0000h`开始
## 杂项
> 可重定位文件和可执行文件：在linux下`.o`和`.bin`文件都是`elf`格式。
> 各个符号的地址都是0，而且有的符号还是未定义的。地址为0是因为，在没有链接之前，各个可重定位文件是独立的，他们无法加载到内存中去执行，各个符号还没有进行重定位。而有的符号未定义是因为该文件中引用了外部文件的代码或者数据。