#include "idt.h"

extern void (*isr_stub_table[256])(void);

static struct idt_entry idt[256];
static struct idt_ptr idtp;

static inline uint16_t read_cs(void) {
    uint16_t cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
    return cs;
}

static void set_idt_entry(int vec, void *isr, uint16_t sel, uint8_t flags) {
    uint64_t addr = (uint64_t)isr;
    idt[vec].offset_low = addr & 0xFFFF;
    idt[vec].selector = sel;
    idt[vec].ist = 0;
    idt[vec].type_attr = flags;
    idt[vec].offset_mid = (addr >> 16) & 0xFFFF;
    idt[vec].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vec].zero = 0;
}

void idt_init(void) {
    uint16_t cs = read_cs();

    for (int i = 0; i < 256; i++) {
        set_idt_entry(i, (void*)isr_stub_table[i], cs, 0x8E);
    }

    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint64_t)&idt;

    __asm__ volatile ("lidt %0" : : "m"(idtp));
}
