#ifndef PRINTK_H
#define PRINTK_H

#include <stdarg.h>

#define info(msg) printk("{BOLD}[{BRIGHT_CYAN}INFO{WHITE}]{RESET} " msg "\n")
#define err(msg) printk("{BOLD}[{BRIGHT_RED}ERR{WHITE}]{RESET} " msg "\n")
#define warn(msg) printk("{BOLD}[{BRIGHT_YELLOW}WARN{WHITE}]{RESET} " msg "\n")

#define infof(fmt, ...) \
    printk("{BOLD}[{BRIGHT_CYAN}INFO{WHITE}]{RESET} " fmt "\n", ##__VA_ARGS__)
#define warnf(fmt, ...) \
    printk("{BOLD}[{BRIGHT_YELLOW}WARN{WHITE}]{RESET} " fmt "\n", ##__VA_ARGS__)
#define errf(fmt, ...) \
    printk("{BOLD}[{BRIGHT_RED}ERR{WHITE}]{RESET} " fmt "\n", ##__VA_ARGS__)

/* Core printk function */
void printk(const char *fmt, ...);
void vprintk(const char *fmt, va_list args);

#endif /* PRINTK_H */
