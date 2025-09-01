#include "printk.h"
#include "string.h"
#include "global.h"
#include <flanterm.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include "heap/kheap.h"

#define ANSI_BUF_LEN     16
#define PRINTK_BUF_SIZE  1024

static const char *lookup_ansi(const char *tag) {
    // Standard foreground
    if (strcmp(tag, "BLACK") == 0) return "\033[30m";
    if (strcmp(tag, "RED") == 0) return "\033[31m";
    if (strcmp(tag, "GREEN") == 0) return "\033[32m";
    if (strcmp(tag, "YELLOW") == 0) return "\033[33m";
    if (strcmp(tag, "BLUE") == 0) return "\033[34m";
    if (strcmp(tag, "MAGENTA") == 0) return "\033[35m";
    if (strcmp(tag, "CYAN") == 0) return "\033[36m";
    if (strcmp(tag, "WHITE") == 0) return "\033[37m";

    // Bright foreground
    if (strcmp(tag, "BRIGHT_BLACK") == 0) return "\033[90m";
    if (strcmp(tag, "BRIGHT_RED") == 0) return "\033[91m";
    if (strcmp(tag, "BRIGHT_GREEN") == 0) return "\033[92m";
    if (strcmp(tag, "BRIGHT_YELLOW") == 0) return "\033[93m";
    if (strcmp(tag, "BRIGHT_BLUE") == 0) return "\033[94m";
    if (strcmp(tag, "BRIGHT_MAGENTA") == 0) return "\033[95m";
    if (strcmp(tag, "BRIGHT_CYAN") == 0) return "\033[96m";
    if (strcmp(tag, "BRIGHT_WHITE") == 0) return "\033[97m";

    // Bold, reset, etc.
    if (strcmp(tag, "BOLD") == 0) return "\033[1m";
    if (strcmp(tag, "RESET") == 0) return "\033[0m";

    return NULL;
}

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#ifndef ANSI_BUF_LEN
#define ANSI_BUF_LEN 32
#endif

/* extern: you provide this */
extern const char *lookup_ansi(const char *tag);

/* extern: you provide these (from string.c) */
void lltoa(long long value, char *str, int base);
void ulltoa(unsigned long long value, char *str, int base);

static size_t format_to_buffer(char *buf, size_t max, const char *fmt, va_list args) {
    char *out = buf;
    size_t left = max - 1; // leave space for null terminator

    while (*fmt && left > 0) {
        /* ---------- {TAG} expansion ---------- */
        if (*fmt == '{') {
            const char *start = fmt + 1;
            const char *end = strchr(start, '}');
            if (end && (end - start) < ANSI_BUF_LEN) {
                char tag[ANSI_BUF_LEN];
                memcpy(tag, start, end - start);
                tag[end - start] = '\0';
                const char *ansi = lookup_ansi(tag);
                if (ansi) {
                    size_t len = strlen(ansi);
                    if (len > left) len = left;
                    memcpy(out, ansi, len);
                    out += len; left -= len;
                    fmt = end + 1;
                    continue;
                }
            }
        }

        /* ---------- literal ---------- */
        if (*fmt != '%') {
            *out++ = *fmt++;
            left--;
            continue;
        }

        /* ---------- start format ---------- */
        fmt++;
        int width = 0;
        int precision = -1;
        int pad_zero = 0;
        int alt = 0;

        if (*fmt == '#') { alt = 1; fmt++; }
        if (*fmt == '0') { pad_zero = 1; fmt++; }

        /* width */
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        /* precision */
        if (*fmt == '.') {
            fmt++;
            precision = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                precision = precision * 10 + (*fmt - '0');
                fmt++;
            }
        }

        /* length modifiers */
        int long_flag = 0;
        int longlong_flag = 0;
        int size_flag = 0;

        if (*fmt == 'l') {
            fmt++;
            if (*fmt == 'l') { longlong_flag = 1; fmt++; }
            else { long_flag = 1; }
        } else if (*fmt == 'z') {
            size_flag = 1;
            fmt++;
        }

        /* ---------- specifier ---------- */
        char tmp[64];
        size_t len = 0;

        switch (*fmt) {
            case 's': {
                const char *s = va_arg(args, const char *);
                if (!s) s = "(null)";
                size_t real_len = strlen(s);
                if (precision >= 0 && (size_t)precision < real_len)
                    real_len = precision;
                if (width > 0 && real_len < (size_t)width) {
                    size_t pad = width - real_len;
                    if (pad > left) pad = left;
                    memset(out, ' ', pad);
                    out += pad; left -= pad;
                }
                if (real_len > left) real_len = left;
                memcpy(out, s, real_len);
                out += real_len; left -= real_len;
                break;
            }
            case 'c': {
                *out++ = (char)va_arg(args, int);
                left--;
                break;
            }
            case 'd': {
                long long val;
                if (longlong_flag) val = va_arg(args, long long);
                else if (long_flag) val = va_arg(args, long);
                else val = va_arg(args, int);
                lltoa(val, tmp, 10);
                len = strlen(tmp);
                if (width > 0 && len < (size_t)width) {
                    size_t pad = width - len;
                    if (pad > left) pad = left;
                    memset(out, pad_zero ? '0' : ' ', pad);
                    out += pad; left -= pad;
                }
                if (len > left) len = left;
                memcpy(out, tmp, len);
                out += len; left -= len;
                break;
            }
            case 'u': {
                unsigned long long val;
                if (longlong_flag) val = va_arg(args, unsigned long long);
                else if (long_flag) val = va_arg(args, unsigned long);
                else if (size_flag) val = va_arg(args, size_t);
                else val = va_arg(args, unsigned int);
                ulltoa(val, tmp, 10);
                len = strlen(tmp);
                if (width > 0 && len < (size_t)width) {
                    size_t pad = width - len;
                    if (pad > left) pad = left;
                    memset(out, pad_zero ? '0' : ' ', pad);
                    out += pad; left -= pad;
                }
                if (len > left) len = left;
                memcpy(out, tmp, len);
                out += len; left -= len;
                break;
            }
            case 'x': {
                unsigned long long val;
                if (longlong_flag) val = va_arg(args, unsigned long long);
                else if (long_flag) val = va_arg(args, unsigned long);
                else if (size_flag) val = va_arg(args, size_t);
                else val = va_arg(args, unsigned int);
                ulltoa(val, tmp, 16);
                len = strlen(tmp);
                if (alt && left >= 2) {
                    *out++ = '0'; *out++ = 'x'; left -= 2;
                }
                if (width > 0 && len < (size_t)width) {
                    size_t pad = width - len;
                    if (pad > left) pad = left;
                    memset(out, pad_zero ? '0' : ' ', pad);
                    out += pad; left -= pad;
                }
                if (len > left) len = left;
                memcpy(out, tmp, len);
                out += len; left -= len;
                break;
            }
            case 'p': {
                uintptr_t val = (uintptr_t)va_arg(args, void *);
                if (left >= 2) { *out++ = '0'; *out++ = 'x'; left -= 2; }
                ulltoa(val, tmp, 16);
                len = strlen(tmp);
                if (len > left) len = left;
                memcpy(out, tmp, len);
                out += len; left -= len;
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
    if (g_ft_ctx) flanterm_write(g_ft_ctx, str, len);
}

void vprintk(const char *fmt, va_list args) {
    if (g_kheap_ready) {
        char *buf = kmalloc(PRINTK_BUF_SIZE);
        if (!buf) return;

        va_list args_copy;
        va_copy(args_copy, args);
        size_t len = format_to_buffer(buf, PRINTK_BUF_SIZE, fmt, args_copy);
        va_end(args_copy);

        output(buf, len);
        kfree(buf);
    } else {
        char tmpbuf[PRINTK_BUF_SIZE];
        va_list args_copy;
        va_copy(args_copy, args);
        size_t len = format_to_buffer(tmpbuf, sizeof(tmpbuf), fmt, args_copy);
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
