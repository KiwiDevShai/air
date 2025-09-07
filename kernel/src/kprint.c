#include "kprint.h"
#include "printk.h"
#include "ansi.h"
#include "string.h"  // for strcpy, strlen

static char prefix_buf[64];

static const char *log_prefix_tag(const char *tag, const char *color) {
    size_t i = 0;

    // Copy bold white [
    strcpy(&prefix_buf[i], ANSI_BOLD);
    i += strlen(ANSI_BOLD);

    strcpy(&prefix_buf[i], ANSI_BRIGHT_WHITE);
    i += strlen(ANSI_BRIGHT_WHITE);

    prefix_buf[i++] = '[';

    // Add tag color
    strcpy(&prefix_buf[i], color);
    i += strlen(color);

    // Add tag itself
    strcpy(&prefix_buf[i], tag);
    i += strlen(tag);

    // Close bracket
    strcpy(&prefix_buf[i], ANSI_BRIGHT_WHITE);
    i += strlen(ANSI_BRIGHT_WHITE);

    prefix_buf[i++] = ']';
    prefix_buf[i++] = ' ';

    // Reset
    strcpy(&prefix_buf[i], ANSI_RESET);
    return prefix_buf;
}

static const char *log_prefix(log_level_t level) {
    switch (level) {
        case LOG_INFO:
            return log_prefix_tag("INFO", ANSI_BRIGHT_CYAN);
        case LOG_WARN:
            return log_prefix_tag("WARN", ANSI_BRIGHT_YELLOW);
        case LOG_ERR:
            return log_prefix_tag("ERR", ANSI_BRIGHT_RED);
        case LOG_DEBUG:
            return log_prefix_tag("DEBUG", ANSI_BRIGHT_WHITE);
        default:
            return log_prefix_tag("???", ANSI_BRIGHT_MAGENTA);
    }
}

void kprint(log_level_t level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vkprint(level, fmt, args);
    va_end(args);
}

void vkprint(log_level_t level, const char *fmt, va_list args) {
    printk("%s", log_prefix(level));
    vprintk(fmt, args);
}
