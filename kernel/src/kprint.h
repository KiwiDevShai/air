#ifndef KPRINT_H
#define KPRINT_H

#include <stdarg.h>
#include "ansi.h"
#include "printk.h"

// Log levels
typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERR,
    LOG_DEBUG,
} log_level_t;

// Core log output
void kprint(log_level_t level, const char *fmt, ...);
void vkprint(log_level_t level, const char *fmt, va_list args);

#endif /* KPRINT_H */
