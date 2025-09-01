#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <flanterm.h>
#include <flanterm_backends/fb.h>

#include "version.h"
#include "string.h"
#include "global.h"
#include "printk.h"
#include "idt/idt.h"
#include "mmu/memmap.h"
#include "mmu/pmm.h"
#include "mmu/vmm.h"
#include "heap/kheap.h"

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

struct human_size {
    uint64_t whole;
    uint64_t fraction; // fraction in hundredths (e.g., .00 to .99)
    const char *unit;
};

struct human_size human_readable_size(size_t bytes) {
    struct human_size result = {0};

    if (bytes >= (1ULL << 30)) {
        result.whole = bytes >> 30;
        result.fraction = ((bytes & ((1ULL << 30) - 1)) * 100) >> 30;
        result.unit = "GiB";
    } else if (bytes >= (1ULL << 20)) {
        result.whole = bytes >> 20;
        result.fraction = ((bytes & ((1ULL << 20) - 1)) * 100) >> 20;
        result.unit = "MiB";
    } else if (bytes >= (1ULL << 10)) {
        result.whole = bytes >> 10;
        result.fraction = ((bytes & ((1ULL << 10) - 1)) * 100) >> 10;
        result.unit = "KiB";
    } else {
        result.whole = bytes;
        result.fraction = 0;
        result.unit = "B";
    }

    return result;
}

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
        info("No HHDM response from Limine!");
        hcf();
    }
    if (rsdp_request.response == NULL || rsdp_request.response->address == 0) {
        err("No RSDP from Limine!");
        hcf();
    }
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    g_ft_ctx = flanterm_fb_init(
        NULL, NULL,
        fb->address, fb->width, fb->height, fb->pitch,
        fb->red_mask_size, fb->red_mask_shift,
        fb->green_mask_size, fb->green_mask_shift,
        fb->blue_mask_size, fb->blue_mask_shift,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, 0, 0, 1, 0, 0, 0
    );
    g_rsdp = (uint64_t*)rsdp_request.response->address;

    // Show kernel version
    printk("%s\n", KERNEL_VERSION_STRING);

    // Initialize IDT
    idt_init();

    // Init memmap + save HHDM
    memmap_init(memmap_request.response);

    // Init PMM
    pmm_init();
    info("Physical Memory Manager initialized");

    // Init VMM
    vmm_init();
    info("Virtual Memory Manager initialized");

    kheap_init_auto();
    info("Heap initialized");

    // Hang forever for now
    hcf();
}
