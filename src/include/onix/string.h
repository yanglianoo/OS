#ifndef ONIX_STRING_H
#define ONIX_STRING_H

#include "types.h"
char* strcpy(char *dest, const char *src);

char* strcat(char *dest, const char *src);
//计算字符串的长度
size_t strlen(const char *str);

int strcmp(const char *lhs, const char *rhs);

char* strchr(const char *str, int ch);

char* strrchr(const char *str, int ch);

int memcmp(const void *lhs, const void *rhs, size_t count);
void* memset(void *dest, int ch, size_t count);
void* memcpy(void *dest, const void *src, size_t count);
void* memchr(const void *str, int ch, size_t count); 

#endif