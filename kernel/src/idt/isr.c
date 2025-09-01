#include <stdint.h>
#include "printk.h"

struct isr_frame {
    uint64_t rax, rcx, rdx, rbx, rbp, rsi, rdi;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;

    uint64_t int_no;
    uint64_t err_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
};

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
    printk("{YELLOW}  PF err:{RESET} P=%d W/R=%d U/S=%d RSVD=%d I/D=%d PK=%d SS=%d SGX=%d\n",
           (e>>0)&1, (e>>1)&1, (e>>2)&1, (e>>3)&1, (e>>4)&1,
           (e>>5)&1, (e>>6)&1, (e>>15)&1);
}

uint64_t isr_common_frame(struct isr_frame *f) {
    const char *name = exc_name(f->int_no);

    printk("\n{RED}{BOLD}*** EXCEPTION ***{RESET} {MAGENTA}%s{RESET} on CPU %d\n", name, (uint64_t)0); // TODO: real CPU id
    printk("{CYAN}rax{RESET}: 0x%016lx  {CYAN}rbx{RESET}: 0x%016lx  {CYAN}rcx{RESET}: 0x%016lx  {CYAN}rdx{RESET}: 0x%016lx\n",
           f->rax, f->rbx, f->rcx, f->rdx);
    printk("{CYAN}rsi{RESET}: 0x%016lx  {CYAN}rdi{RESET}: 0x%016lx  {CYAN}rbp{RESET}: 0x%016lx  {CYAN}r8 {RESET}: 0x%016lx\n",
           f->rsi, f->rdi, f->rbp, f->r8);
    printk("{CYAN}r9 {RESET}: 0x%016lx  {CYAN}r10{RESET}: 0x%016lx  {CYAN}r11{RESET}: 0x%016lx  {CYAN}r12{RESET}: 0x%016lx\n",
           f->r9, f->r10, f->r11, f->r12);
    printk("{CYAN}r13{RESET}: 0x%016lx  {CYAN}r14{RESET}: 0x%016lx  {CYAN}r15{RESET}: 0x%016lx\n",
           f->r13, f->r14, f->r15);

    printk("{YELLOW}vec{RESET}: %lu  {YELLOW}err{RESET}: %lu\n", f->int_no, f->err_code);
    printk("{GREEN}RIP{RESET}: 0x%016lx  {GREEN}CS{RESET}: 0x%016lx  {GREEN}RFLAGS{RESET}: 0x%016lx\n",
           f->rip, f->cs, f->rflags);

    if (f->int_no == 14) {
        uint64_t cr2 = read_cr2();
        printk("{MAGENTA}CR2{RESET}: 0x%016lx\n", cr2);
        dump_pf_err(f->err_code);
    }

    printk("{BOLD}{BLUE}Stack dump (top 16 qwords):{RESET}\n");
    uint64_t *sp = (uint64_t *)(&f->rflags + 1);
    for (int i = 0; i < 16; i++) {
        printk("  [rsp+%02x] = 0x%016lx\n", i * 8, sp[i]);
    }

    if (f->int_no == 3) {
        printk("{GREEN}#BP Breakpoint caught — continuing execution{RESET}\n");
        return (uint64_t)f;
    }

    for (;;) __asm__ volatile("hlt");
}
