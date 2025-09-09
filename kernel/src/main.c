#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <flanterm.h>

#include "kprint.h"
#include "version.h"
#include "string.h"
#include "serial.h"
#include "global.h"
#include "idt/idt.h"
#include "mmu/memmap.h"
#include "mmu/pmm.h"
#include "mmu/vmm.h"
#include "heap/kheap.h"
#include "video/fonts.h"
#include "video/video.h"

// Limine Base Revision
__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// Limine requests
__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_executable_cmdline_request cmdline_request = {
    .id = LIMINE_EXECUTABLE_CMDLINE_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

// Kernel stack
__attribute__((aligned(16)))
uint8_t kernel_stack[16 * 1024 * 1024]; // 16 MiB

// Kernel entry point
void kmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED ||
        framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Set stack pointer
    asm volatile (
        "lea %[stack_top], %%rsp\n"
        :
        : [stack_top] "m" (*(uint64_t *)(kernel_stack + sizeof(kernel_stack)))
        : "rsp"
    );

    // Initialize serial output
    serial_init();

    // Check HHDM response
    if (hhdm_request.response) {
        g_hhdm_offset = hhdm_request.response->offset;
    } else {
        kprint(LOG_INFO, "No HHDM response from Limine!\n");
        hcf();
    }

    // Check RSDP response
    if (!rsdp_request.response || rsdp_request.response->address == 0) {
        kprint(LOG_ERR, "No RSDP from Limine!\n");
        hcf();
    }

    // Initialize framebuffer
    g_fb = framebuffer_request.response->framebuffers[0];
    ft_init(NULL, 8, 16, 0, 0);

    g_rsdp = (uint64_t*)rsdp_request.response->address;

    // Command line parsing
    if (!cmdline_request.response || !cmdline_request.response->cmdline) {
        kprint(LOG_ERR, "Command line response or pointer is NULL!\n");
        hcf();
    }

    const char *cmdline = cmdline_request.response->cmdline;
    debug = strstr(cmdline, "debug");

    // Show kernel version
    kprint(LOG_INFO, "%s%s\n", (debug ? "debug-" : ""), KERNEL_VERSION_STRING);

    // Initialize subsystems
    memmap_init(memmap_request.response);
    idt_init();
    pmm_init();
    vmm_init();
    kheap_init();

    // Hang forever (for now)
    hcf();
}
