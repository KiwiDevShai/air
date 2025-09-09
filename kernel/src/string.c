#include "string.h"
#include "io.h"

/* ----------------- String functions ----------------- */
size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

size_t strnlen(const char *s, size_t max) {
    size_t len = 0;
    while (len < max && s[len]) len++;
    return len;
}

void strcpy(char *dest, const char *src) {
    while ((*dest++ = *src++));
}

void strncpy(char *dest, const char *src, size_t n) {
    while (n && (*src)) {
        *dest++ = *src++;
        n--;
    }
    while (n--) *dest++ = '\0';
}

void strcat(char *dest, const char *src) {
    while (*dest) dest++;
    while ((*dest++ = *src++));
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n && *a && (*a == *b)) {
        a++; b++; n--;
    }
    return n ? (unsigned char)*a - (unsigned char)*b : 0;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return (char *)s;
        s++;
    }
    return NULL;
}

char *strrchr(const char *s, int c) {
    const char *last = NULL;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    return (char *)last;
}

char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;

    for (; *haystack; haystack++) {
        const char *h = haystack;
        const char *n = needle;

        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }

        if (!*n) return (char *)haystack;
    }

    return NULL;
}

/* ----------------- Memory functions ----------------- */
void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

int memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *pa = a, *pb = b;
    while (n--) {
        if (*pa != *pb) return *pa - *pb;
        pa++; pb++;
    }
    return 0;
}

/* ---------------------- itoa helpers ---------------------- */
static void reverse_str(char *str, char *end) {
    while (str < end) {
        char tmp = *str;
        *str++ = *end;
        *end-- = tmp;
    }
}

void itoa(int value, char *str, int base) {
    char *ptr = str;
    int is_negative = 0;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    while (value) {
        int digit = value % base;
        *ptr++ = (digit < 10) ? digit + '0' : digit - 10 + 'A';
        value /= base;
    }

    if (is_negative) *ptr++ = '-';
    *ptr = '\0';
    reverse_str(str, ptr - 1);
}

void utoa(unsigned long long value, char *str, int base) {
    char *ptr = str;
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }
    while (value) {
        int digit = value % base;
        *ptr++ = (digit < 10) ? digit + '0' : digit - 10 + 'A';
        value /= base;
    }
    *ptr = '\0';
    reverse_str(str, ptr - 1);
}

void lltoa(long long value, char *str, int base) {
    char *ptr = str;
    int is_negative = 0;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    while (value) {
        int digit = value % base;
        *ptr++ = (digit < 10) ? digit + '0' : digit - 10 + 'A';
        value /= base;
    }

    if (is_negative) *ptr++ = '-';
    *ptr = '\0';
    reverse_str(str, ptr - 1);
}

void ulltoa(unsigned long long value, char *str, int base) {
    char *ptr = str;
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }
    while (value) {
        int digit = value % base;
        *ptr++ = (digit < 10) ? digit + '0' : digit - 10 + 'A';
        value /= base;
    }
    *ptr = '\0';
    reverse_str(str, ptr - 1);
}
