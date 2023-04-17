#include <onix/string.h>


//把 src 所指向的字符串复制到 dest。

char *strcpy(char *dest, const char *src)
{
    char *ptr = dest;
    while (true)
    {
        *ptr++ = *src;
        if (*src++ == EOS)
            return dest;
    }
}

//把 src 所指向的字符串追加到 dest 所指向的字符串的结尾。
char* strcat(char *dest, const char *src)
{
    char *ptr = dest;
    while (*ptr != EOS)
    {
        ptr++;
    }
    while (true)
    {
        *ptr++ = *src;
        if(*src++ == EOS)
        {
            return dest;
        }
    }
}

//计算字符串的长度 
size_t strlen(const char *str)
{
    char *ptr = (char *)str;
    while (*ptr != EOS)
    {
        ptr++;
    }
    return ptr - str;
}

//把 lhs1 所指向的字符串和 rhs2 所指向的字符串进行比较
int strcmp(const char *lhs, const char *rhs)
{
    while (*lhs == *rhs && *lhs!= EOS && *rhs != EOS)
    {
        lhs++;
        rhs++;
    }
    return *lhs < *rhs ? -1 : *lhs > *rhs;
}

//在参数 str 所指向的字符串中搜索第一次出现字符 c（一个无符号字符）的位置
char* strchr(const char *str, int ch)
{
    char *ptr = (char *)str;
    while (true)
    {
        if(*ptr == ch)
        {
            return ptr;
        }
        if(*ptr++ == EOS)
        {
            return NULL;
        }
    } 
}

//在参数 str 所指向的字符串中搜索最后一次出现字符 c（一个无符号字符）的位置。
char* strrchr(const char *str, int ch)
{
    char *last = NULL;
    char *ptr = (char *)str;
    while (true)
    {
        if(*ptr == ch)
        {
            last = ptr;
        }
        if(*ptr++ == EOS)
        {
            return last;
        }
    }
}

//把存储区 str1 和存储区 str2 的前 n 个字节进行比较。
int memcmp(const void *lhs, const void *rhs, size_t count)
{
    char *lptr = (char *)lhs;
    char *rptr = (char *)rhs;
    while (*lptr == *rptr && count-- > 0)
    {
        lptr++;
        rptr++;
    }
    return *lptr < *rptr ? -1 : *lptr > *rptr;
}

//复制字符 ch（一个无符号字符）到参数 dest 所指向的字符串的前 n 个字符。
void* memset(void *dest, int ch, size_t count)
{
    char *ptr = dest;
    while (count--)
    {
        *ptr++ = ch;
    }
    return dest;
}

// 从存储区 src 复制 n 个字节到存储区 dest。
void* memcpy(void *dest, const void *src, size_t count)
{
    char *ptr = dest;
    while (count--)
    {
        *ptr++ = *((char *)(src++));
    }
    return dest;
}

//在参数 str 所指向的字符串的前 n 个字节中搜索第一次出现字符 c（一个无符号字符）的位置。
void* memchr(const void *str, int ch, size_t count)
{
    char *ptr = (char *)str;
    while (count--)
    {
        if(*ptr == ch)
        {
            return (void *)ptr;
        }
        ptr++;
    }
}

