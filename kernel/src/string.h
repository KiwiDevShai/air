#ifndef STRING_H
#define STRING_H

#include <stddef.h>

/* ----------------- String functions ----------------- */
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t max);

void strcpy(char *dest, const char *src);
void strncpy(char *dest, const char *src, size_t n);

void strcat(char *dest, const char *src);

int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);

char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);

/* ----------------- Memory functions ----------------- */
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *a, const void *b, size_t n);

/* ----------------- itoa / number helpers ----------------- */
void itoa(int value, char *str, int base);
void utoa(unsigned long long value, char *str, int base);
void lltoa(long long value, char *str, int base);
void ulltoa(unsigned long long value, char *str, int base);

#endif /* STRING_H */
