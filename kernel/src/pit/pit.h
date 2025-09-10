#ifndef PIT_H
#define PIT_H

#include <stdint.h>

/**
 * Initializes the PIT to the specified frequency (Hz).
 * This sets up the timer to trigger IRQ0 periodically.
 */
void pit_init(uint32_t frequency);

/**
 * Returns the number of ticks since the PIT was initialized.
 */
uint64_t pit_get_ticks(void);

/**
 * Blocking sleep (busy-wait) for the given number of milliseconds.
 */
void pit_sleep(uint64_t ms);

#endif // PIT_H
