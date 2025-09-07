#include "printk.h"
#include "serial.h"
#include "heap/kheap.h"
#include "global.h"       // for g_ft_ctx, g_kheap_ready
#include "string.h"       // for all your homegrown libc knockoffs

#include <flanterm.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define PRINTK_BUF_SIZE 1024

static void append_number(char **buf, size_t *left, unsigned long long value,
                          int base, int width, int pad_zero, int is_signed) {
    char tmp[64];
    size_t len;

    if (is_signed)
        lltoa((long long)value, tmp, base);
    else
        ulltoa(value, tmp, base);

    len = strlen(tmp);

    if ((size_t)width > len) {
        size_t pad = width - len;
        char pad_char = pad_zero ? '0' : ' ';
        for (size_t i = 0; i < pad && *left > 0; i++) {
            *(*buf)++ = pad_char;
            (*left)--;
        }
    }

    for (size_t i = 0; i < len && *left > 0; i++) {
        *(*buf)++ = tmp[i];
        (*left)--;
    }
}

static size_t vsnprintf(char *buf, size_t max, const char *fmt, va_list args) {
    char *out = buf;
    size_t left = max - 1;

    while (*fmt && left > 0) {
        if (*fmt != '%') {
            *out++ = *fmt++;
            left--;
            continue;
        }

        fmt++;

        int width = 0;
        int pad_zero = 0;

        if (*fmt == '0') {
            pad_zero = 1;
            fmt++;
        }

        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        switch (*fmt) {
            case 's': {
                const char *s = va_arg(args, const char *);
                if (!s) s = "(null)";
                while (*s && left > 0) {
                    *out++ = *s++;
                    left--;
                }
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                *out++ = c;
                left--;
                break;
            }
            case 'd': {
                int val = va_arg(args, int);
                append_number(&out, &left, val, 10, width, pad_zero, 1);
                break;
            }
            case 'u': {
                unsigned int val = va_arg(args, unsigned int);
                append_number(&out, &left, val, 10, width, pad_zero, 0);
                break;
            }
            case 'x': {
                unsigned int val = va_arg(args, unsigned int);
                append_number(&out, &left, val, 16, width, pad_zero, 0);
                break;
            }
            case 'p': {
                uintptr_t val = (uintptr_t)va_arg(args, void *);
                if (left >= 2) {
                    *out++ = '0';
                    *out++ = 'x';
                    left -= 2;
                }
                append_number(&out, &left, val, 16, 0, 0, 0);
                break;
            }
            case '%': {
                *out++ = '%';
                left--;
                break;
            }
            default: {
                *out++ = '%';
                if (left > 0) {
                    *out++ = *fmt;
                    left--;
                }
                left--;
                break;
            }
        }

        fmt++;
    }

    *out = '\0';
    return (size_t)(out - buf);
}

static void output(const char *str, size_t len) {
    serial_write(str);
    if (g_ft_ctx) {
        flanterm_write(g_ft_ctx, str, len);
    }
}

void vprintk(const char *fmt, va_list args) {
    if (g_kheap_ready) {
        char *buf = kmalloc(PRINTK_BUF_SIZE);
        if (!buf) return;

        va_list args_copy;
        va_copy(args_copy, args);
        size_t len = vsnprintf(buf, PRINTK_BUF_SIZE, fmt, args_copy);
        va_end(args_copy);

        output(buf, len);
        kfree(buf);
    } else {
        char tmpbuf[PRINTK_BUF_SIZE];
        va_list args_copy;
        va_copy(args_copy, args);
        size_t len = vsnprintf(tmpbuf, sizeof(tmpbuf), fmt, args_copy);
        va_end(args_copy);

        output(tmpbuf, len);
    }
}

void printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}
