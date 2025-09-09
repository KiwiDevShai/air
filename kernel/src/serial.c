#include "io.h"

#define COM1 0x3F8

static inline int serial_ready(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_init(void) {
    outb(COM1 + 1, 0x00); // Disable interrupts
    outb(COM1 + 3, 0x80); // Enable DLAB
    outb(COM1 + 0, 0x03); // Divisor = 3 → 38400 baud
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03); // 8n1
    outb(COM1 + 2, 0xC7); // FIFO, 14-byte threshold
    outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void serial_putchar(char c) {
    while (!serial_ready());
    outb(COM1, c);
}

void serial_write(const char *s) {
    while (*s) {
        if (*s == '\n') serial_putchar('\r'); // CRLF
        serial_putchar(*s++);
    }
}

static inline int serial_received(void) {
    // Bit 0 of line status register indicates data is ready
    return inb(COM1 + 5) & 0x01;
}

char serial_getchar(void) {
    while (!serial_received());
    return inb(COM1);
}
