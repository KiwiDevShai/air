#ifndef SERIAL_H
#define SERIAL_H

void serial_init(void);
void serial_putchar(char c);
void serial_write(const char *s);

char serial_getchar(void);
int serial_received(void);

#endif // SERIAL_H