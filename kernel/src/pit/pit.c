#include "pit/pit.h"
#include "global.h"
#include "io.h"
#include "idt/isr.h"
#include "kprint.h"

#define PIT_CHANNEL0_PORT 0x40
#define PIT_COMMAND_PORT  0x43
#define PIT_BASE_FREQUENCY 1193182

static volatile uint64_t pit_ticks = 0;

void pit_tick_handler(void) {
    pit_ticks++;
    // printk("[PIT] tick %lu\n", pit_ticks);
}

void pit_init(uint32_t frequency) {
    if (frequency == 0) return;

    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    // set PIT to mode 3 (square wave), channel 0, binary mode
    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF); // low byte
    outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xFF); // high byte

    irq_register_handler(0, pit_tick_handler); // Hook IRQ0
    if (debug) kprint(LOG_DEBUG, "PIT initialized\n");
}

uint64_t pit_get_ticks(void) {
    return pit_ticks;
}

void pit_sleep(uint64_t ms) {
    uint64_t target = pit_ticks + (ms * 1000 / 1000); // sleepy weepy
    while (pit_ticks < target) {
        __asm__ volatile ("pause");
    }
}
