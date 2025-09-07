#ifndef PRINTK_H
#define PRINTK_H

#include <stdarg.h>
#include "ansi.h"

/* Core printk function */
void printk(const char *fmt, ...);
void vprintk(const char *fmt, va_list args);

#endif /* PRINTK_H */
