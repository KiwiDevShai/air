#include "kprint.h"
#include "printk.h"
#include "ansi.h"
#include "string.h"
#include "pit/pit.h"

static char tag_buf[64];
static char time_buf[64];

static const char *log_prefix_tag_buf(char *buf, const char *tag, const char *color) {
    size_t i = 0;

    strcpy(&buf[i], ANSI_BOLD);          i += strlen(ANSI_BOLD);
    strcpy(&buf[i], ANSI_BRIGHT_WHITE);  i += strlen(ANSI_BRIGHT_WHITE);
    buf[i++] = '[';

    strcpy(&buf[i], color);              i += strlen(color);
    strcpy(&buf[i], tag);                i += strlen(tag);

    strcpy(&buf[i], ANSI_BRIGHT_WHITE);  i += strlen(ANSI_BRIGHT_WHITE);
    buf[i++] = ']';
    buf[i++] = ' ';
    strcpy(&buf[i], ANSI_RESET);
    return buf;
}

static const char *log_time_prefix(void) {
    uint64_t ticks = pit_get_ticks();
    uint64_t sec = ticks / 1000;
    uint64_t ms = ticks % 1000;

    char tmp[16], *tp = tmp;

    ulltoa(sec, tp, 10);
    tp += strlen(tp);

    *tp++ = '.';

    if (ms < 100) *tp++ = '0';
    if (ms < 10)  *tp++ = '0';

    ulltoa(ms, tp, 10);
    tp += strlen(tp);

    *tp = '\0';

    return log_prefix_tag_buf(time_buf, tmp, ANSI_BRIGHT_GREEN);
}

static const char *log_prefix(log_level_t level) {
    const char *color =
        (level == LOG_INFO)  ? ANSI_BRIGHT_CYAN :
        (level == LOG_WARN)  ? ANSI_BRIGHT_YELLOW :
        (level == LOG_ERR)   ? ANSI_BRIGHT_RED :
        (level == LOG_DEBUG) ? ANSI_BRIGHT_BLUE :
                                ANSI_BRIGHT_MAGENTA;

    const char *tag =
        (level == LOG_INFO)  ? "INFO" :
        (level == LOG_WARN)  ? "WARN" :
        (level == LOG_ERR)   ? "ERR " :
        (level == LOG_DEBUG) ? "DBG " : "???";

    return log_prefix_tag_buf(tag_buf, tag, color);
}

void kprint(log_level_t level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vkprint(level, fmt, args);
    va_end(args);
}

void vkprint(log_level_t level, const char *fmt, va_list args) {
    const char *ts = log_time_prefix();
    const char *tag = log_prefix(level);

    printk("%s%s", ts, tag);
    vprintk(fmt, args);
}
