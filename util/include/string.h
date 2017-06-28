#ifndef __STRING_H__
#define __STRING_H__

#include <define.h>

int atoi(char *str);
char c2x(char ch); 
int a2d(char ch);
char a2i(char ch, char **src, int base, int *nump);
void ui2a(unsigned int num, unsigned int base, char *bf);
void i2a(int num, char *bf);
char toupper(char c);
int strcmp(char *str1, char *str2, size_t num);
int strlen(char *str);
void Memcpy(void *destination, const void *source, size_t num);
void *memset(void *s, int c, unsigned int n);
int is_digit(char c);
int is_alpha(char c);
#endif // __STRING_H__
