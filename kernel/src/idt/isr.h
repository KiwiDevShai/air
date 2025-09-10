#ifndef ISR_H
#define ISR_H

// Register an IRQ handler for a given IRQ number (0–15)
void irq_register_handler(int irq, void (*handler)(void));

#endif // ISR_H
