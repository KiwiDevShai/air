#include <stdint.h>
#include "printk.h"
#include "io.h"

#define MAX_IRQS 16

struct isr_frame {
    uint64_t rax, rcx, rdx, rbx, rbp, rsi, rdi;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t int_no;
    uint64_t err_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
};

// IRQ Support

typedef void (*irq_handler_t)(void);
static irq_handler_t irq_handlers[MAX_IRQS] = { 0 };

void irq_register_handler(int irq, irq_handler_t handler) {
    if (irq < MAX_IRQS) {
        irq_handlers[irq] = handler;
    }
}

// EXCEPTION MESSAGE DECODING

static const char *exc_name(uint64_t v) {
    switch (v) {
        case 0:  return "#DE Divide Error";
        case 1:  return "#DB Debug";
        case 2:  return "NMI";
        case 3:  return "#BP Breakpoint";
        case 4:  return "#OF Overflow";
        case 5:  return "#BR BOUND Range";
        case 6:  return "#UD Invalid Opcode";
        case 7:  return "#NM Device Not Available";
        case 8:  return "#DF Double Fault";
        case 10: return "#TS Invalid TSS";
        case 11: return "#NP Segment Not Present";
        case 12: return "#SS Stack Fault";
        case 13: return "#GP General Protection";
        case 14: return "#PF Page Fault";
        case 16: return "#MF x87 FP";
        case 17: return "#AC Alignment Check";
        case 18: return "#MC Machine Check";
        case 19: return "#XF SIMD FP";
        case 30: return "#SX Security";
        default: return "Unknown";
    }
}

static inline uint64_t read_cr2(void) {
    uint64_t x; __asm__ volatile("mov %%cr2,%0" : "=r"(x));
    return x;
}

static void dump_pf_err(uint64_t e) {
    printk("\x1b[33m  PF err:\x1b[0m P=%d W/R=%d U/S=%d RSVD=%d I/D=%d PK=%d SS=%d SGX=%d\n",
           (e>>0)&1, (e>>1)&1, (e>>2)&1, (e>>3)&1, (e>>4)&1,
           (e>>5)&1, (e>>6)&1, (e>>15)&1);
}

uint64_t isr_common_frame(struct isr_frame *f) {
    // IRQ HANDLING
    if (f->int_no >= 32 && f->int_no < 48) {
        int irq = f->int_no - 32;

        if (irq < MAX_IRQS && irq_handlers[irq]) {
            irq_handlers[irq]();
        }

        // Send End Of Interrupt (EOI)
        if (f->int_no >= 40) {
            outb(0xA0, 0x20); // Slave PIC
        }
        outb(0x20, 0x20); // Master PIC

        return (uint64_t)f;
    }

    // EXCEPTION HANDLING
    const char *name = exc_name(f->int_no);

    printk("\n\x1b[31m\x1b[1m*** EXCEPTION ***\x1b[0m \x1b[35m%s\x1b[0m on CPU %d\n", name, (uint64_t)0);
    printk("\x1b[36mrax\x1b[0m: 0x%016lx  \x1b[36mrbx\x1b[0m: 0x%016lx  \x1b[36mrcx\x1b[0m: 0x%016lx  \x1b[36mrdx\x1b[0m: 0x%016lx\n",
           f->rax, f->rbx, f->rcx, f->rdx);
    printk("\x1b[36mrsi\x1b[0m: 0x%016lx  \x1b[36mrdi\x1b[0m: 0x%016lx  \x1b[36mrbp\x1b[0m: 0x%016lx  \x1b[36mr8 \x1b[0m: 0x%016lx\n",
           f->rsi, f->rdi, f->rbp, f->r8);
    printk("\x1b[36mr9 \x1b[0m: 0x%016lx  \x1b[36mr10\x1b[0m: 0x%016lx  \x1b[36mr11\x1b[0m: 0x%016lx  \x1b[36mr12\x1b[0m: 0x%016lx\n",
           f->r9, f->r10, f->r11, f->r12);
    printk("\x1b[36mr13\x1b[0m: 0x%016lx  \x1b[36mr14\x1b[0m: 0x%016lx  \x1b[36mr15\x1b[0m: 0x%016lx\n",
           f->r13, f->r14, f->r15);

    printk("\x1b[33mvec\x1b[0m: %lu  \x1b[33merr\x1b[0m: %lu\n", f->int_no, f->err_code);
    printk("\x1b[32mRIP\x1b[0m: 0x%016lx  \x1b[32mCS\x1b[0m: 0x%016lx  \x1b[32mRFLAGS\x1b[0m: 0x%016lx\n",
           f->rip, f->cs, f->rflags);

    if (f->int_no == 14) {
        uint64_t cr2 = read_cr2();
        printk("\x1b[35mCR2\x1b[0m: 0x%016lx\n", cr2);
        dump_pf_err(f->err_code);
    }

    printk("\x1b[1m\x1b[34mStack dump (top 16 qwords):\x1b[0m\n");
    uint64_t *sp = (uint64_t *)(&f->rflags + 1);
    for (int i = 0; i < 16; i++) {
        printk("  [rsp+%02x] = 0x%016lx\n", i * 8, sp[i]);
    }

    if (f->int_no == 3) {
        printk("\x1b[32m#BP Breakpoint caught — continuing execution\x1b[0m\n");
        return (uint64_t)f;
    }

    for (;;) __asm__ volatile("hlt");
}
