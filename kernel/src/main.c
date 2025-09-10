#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <flanterm.h>

#include "io.h"
#include "kprint.h"
#include "pit/pit.h"
#include "version.h"
#include "string.h"
#include "serial.h"
#include "global.h"
#include "idt/idt.h"
#include "mmu/memmap.h"
#include "mmu/pmm.h"
#include "mmu/vmm.h"
#include "heap/kheap.h"
#include "vfs/file.h"
#include "vfs/fs/ramfs/ramfs.h"
#include "vfs/vfs.h"
#include "video/fonts.h"
#include "video/video.h"

// Limine boot protocol requests

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

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

// Kernel Stack

__attribute__((aligned(16)))
uint8_t kernel_stack[16 * 1024 * 1024]; // 16 MiB

void pic_remap(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Master offset = 32
    outb(0xA1, 0x28); // Slave offset = 40
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0); // Unmask all
    outb(0xA1, 0x0);
}

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

    serial_init();

    if (hhdm_request.response)
        g_hhdm_offset = hhdm_request.response->offset;
    else {
        kprint(LOG_ERR, "No HHDM response from Limine!\n");
        hcf();
    }

    if (!rsdp_request.response || rsdp_request.response->address == 0) {
        kprint(LOG_ERR, "No RSDP from Limine!\n");
        hcf();
    }

    g_fb = framebuffer_request.response->framebuffers[0];
    ft_init(NULL, 8, 16, 0, 0);
    g_rsdp = (uint64_t*)rsdp_request.response->address;

    if (!cmdline_request.response || !cmdline_request.response->cmdline) {
        kprint(LOG_ERR, "No command line from Limine!\n");
        hcf();
    }

    const char *cmdline = cmdline_request.response->cmdline;
    pic_remap();
    debug = strstr(cmdline, "debug");

    kprint(LOG_INFO, "%s%s\n", (debug ? "debug-" : ""), KERNEL_VERSION_STRING);

    // Initialize systems
    memmap_init(memmap_request.response);
    idt_init();
    pit_init(1000);
    __asm__ volatile("sti");
    pmm_init();
    vmm_init();
    kheap_init();
    vfs_init();
    vfs_register_filesystem(&ramfs_fs);
    vfs_mount("ramfs", NULL, "/");

    kprint(LOG_INFO, "Sleeping for 1 second...\n");
    pit_sleep(1000);
    kprint(LOG_WARN, "Halting on 3...\n");
    pit_sleep(1000);
    kprint(LOG_WARN, "Halting on 2...\n");
    pit_sleep(1000);
    kprint(LOG_WARN, "Halting on 1...\n");
    pit_sleep(1000);
    kprint(LOG_WARN, "Halting.\n");
    // Hang
    hcf();
}
