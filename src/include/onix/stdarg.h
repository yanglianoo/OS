#ifndef ONIX_STDARG_H
#define ONIX_STDARG_H

//将可变参数全部入栈
typedef char* va_list;
//在32位系统栈帧分配的单元大小是4字节(一个参数占4字节)
#define va_start(ap,v) (ap = (va_list)&v )
#define va_arg(ap,t) (*(t*)(ap += sizeof(t *)))
#define va_end(ap) (ap = (va_list)0)   //直接将va_list 置为空指针

#endif