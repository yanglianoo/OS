# OS开发

- 参考书籍:
    - 操作系统真相还原
    - 30天自制操作系统

**常用寄存器：**

![image-20230128173803324](README.assets/image-20230128173803324.png)

| 16位寄存器 |      功能      |
| :--------: | :------------: |
|     AX     |   累加寄存器   |
|     CX     |   计数寄存器   |
|     DX     |   数据寄存器   |
|     BX     |   基址寄存器   |
|     SP     |  栈指针寄存器  |
|     BP     |  基指针寄存器  |
|     SI     |  源变址寄存器  |
|     DI     | 目的变址寄存器 |

| 8位寄存器 |      功能       |
| :-------: | :-------------: |
|    AL     | 累加寄存器低8位 |
|    CL     | 计数寄存器低8位 |
|    DL     | 数据寄存器低8位 |
|    BL     | 基址寄存器低8位 |
|    AH     | 累加寄存器高8位 |
|    CH     | 计数寄存器高8位 |
|    DH     | 数据寄存器高8位 |
|    BH     | 基址寄存器高8位 |

- `EAX，ECX，EDX，EBX，ESP，EBP，ESI，EDI `为32位寄存器，寓意为`Extend`，在16位寄存器前加`E`。

| 段寄存器 |     功能     |
| :------: | :----------: |
|    ES    | 附件段寄存器 |
|    CS    | 代码段寄存器 |
|    SS    |  栈段寄存器  |
|    DS    | 数据段寄存器 |
|    FS    |   没有名称   |
|    GS    |   没有名称   |
