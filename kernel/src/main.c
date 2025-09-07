#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <flanterm.h>

#include "kprint.h"
#include "version.h"
#include "serial.h"
#include "global.h"
#include "printk.h"
#include "idt/idt.h"
#include "mmu/memmap.h"
#include "mmu/pmm.h"
#include "mmu/vmm.h"
#include "heap/kheap.h"
#include "video/fonts.h"
#include "ansi.h"
#include "video/video.h"

// Limine Base Revision
__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// Framebuffer request
__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Memory map request
__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

// HHDM request
__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

// RSDP request
__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

// Kernel entry point
void kmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED
        || framebuffer_request.response == NULL
        || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Init serial first so we get logs early
    serial_init();
    // Setup framebuffer + flanterm
    if (hhdm_request.response) {
        g_hhdm_offset = hhdm_request.response->offset;
    } else {
        kprint(LOG_INFO, "No HHDM response from Limine!\n");
        hcf();
    }
    if (rsdp_request.response == NULL || rsdp_request.response->address == 0) {
        kprint(LOG_ERR, "No RSDP from Limine!\n");
        hcf();
    }
    // struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    g_fb = framebuffer_request.response->framebuffers[0];
    ft_init(NULL, 8, 16, 2, 2);
    g_rsdp = (uint64_t*)rsdp_request.response->address;

    // Show kernel version
    kprint(LOG_INFO, "%s!!\n", KERNEL_VERSION_STRING);

    // Initialize IDT
    idt_init();

    // Init memmap + save HHDM
    memmap_init(memmap_request.response);

    // Init PMM
    pmm_init();
    kprint(LOG_INFO, "Physical Memory Manager initialized\n");

    // Init VMM
    vmm_init();
    kprint(LOG_INFO, "Virtual Memory Manager initialized\n");

    kheap_init_auto();
    kprint(LOG_INFO, "Heap initialized\n");

    ft_set_font((void*)fontosaurus_fnt, 8, 16);
    kprint(LOG_INFO, "New font initialized\n");

    for (int i = 0x20; i <= 0x7E; i++) {
        const char *color =
            (i >= '0' && i <= '9') ? ANSI_BRIGHT_CYAN :
            (i >= 'A' && i <= 'Z') ? ANSI_BRIGHT_YELLOW :
            (i >= 'a' && i <= 'z') ? ANSI_BRIGHT_GREEN :
                                    ANSI_BRIGHT_MAGENTA;

        printk(ANSI_BOLD "%s0x", color);
        if (i < 0x10)
            printk("0");
        printk("%x: '%c'" ANSI_RESET "%s", i, (char)i, ((i - 0x1F) % 5 == 0) ? "\n" : " ");
    }

    // Hang forever for now
    hcf();
}
