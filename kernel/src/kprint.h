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

__attribute__((deprecated("Use kprint(LOG_INFO, ...) instead")))
static inline void info(const char *msg) {
    kprint(LOG_INFO, "%s\n", msg);
}

__attribute__((deprecated("Use kprint(LOG_WARN, ...) instead")))
static inline void warn(const char *msg) {
    kprint(LOG_WARN, "%s\n", msg);
}

__attribute__((deprecated("Use kprint(LOG_ERR, ...) instead")))
static inline void err(const char *msg) {
    kprint(LOG_ERR, "%s\n", msg);
}

__attribute__((deprecated("Use kprint(LOG_INFO, ...) instead")))
static inline void infof(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vkprint(LOG_INFO, fmt, args);
    va_end(args);
    printk("\n");
}

__attribute__((deprecated("Use kprint(LOG_WARN, ...) instead")))
static inline void warnf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vkprint(LOG_WARN, fmt, args);
    va_end(args);
    printk("\n");
}

__attribute__((deprecated("Use kprint(LOG_ERR, ...) instead")))
static inline void errf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vkprint(LOG_ERR, fmt, args);
    va_end(args);
    printk("\n");
}

#endif /* KPRINT_H */
