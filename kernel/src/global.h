#ifndef GLOBAL_H
#define GLOBAL_H

#include <flanterm.h>
#include <limine.h>
#include <stdbool.h>
#include <stdint.h>

/* Global flanterm context */
extern struct flanterm_context *g_ft_ctx;

/* Global memory map response from Limine */
extern struct limine_memmap_response *g_memmap;

/* Global HHDM offset */
extern uint64_t g_hhdm_offset;

/* Global RSDP pointer */
extern uint64_t *g_rsdp;

/* Global Framebuffer pointer */
extern struct limine_framebuffer *g_fb;

/* Kheap ready flag */
extern bool g_kheap_ready;

/* Debug flag */
extern bool debug;

/* Helpers */
static inline void *phys_to_virt(uintptr_t phys) {
    return (void *)(g_hhdm_offset + phys);
}

static inline uintptr_t virt_to_phys(void *virt) {
    return (uintptr_t)virt - g_hhdm_offset;
}

static void hcf(void) {
    for (;;) {
#if defined(__x86_64__)
        asm ("hlt");
#else
        while (1);
#endif
    }
}

/* Some types */
typedef long ssize_t;

#endif /* GLOBAL_H */
