#include "memmap.h"
#include "global.h"
#include "printk.h"

void memmap_init(struct limine_memmap_response *response) {
    g_memmap = response;

    if (!g_memmap) {
        printk("No memory map provided by bootloader!\n");
        for (;;);
    }
}

struct limine_memmap_entry *memmap_find_biggest_region(void) {
    struct limine_memmap_entry *best = NULL;

    for (uint64_t i = 0; i < g_memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = g_memmap->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE) continue;

        if (!best || entry->length > best->length) {
            best = entry;
        }
    }

    return best;
}

void memmap_dump(void) {
    printk("{CYAN}--- Memory Map ---{RESET}\n");
    for (uint64_t i = 0; i < g_memmap->entry_count; i++) {
        struct limine_memmap_entry *e = g_memmap->entries[i];
        const char *type;
        switch (e->type) {
            case LIMINE_MEMMAP_USABLE:                 type = "USABLE"; break;
            case LIMINE_MEMMAP_RESERVED:               type = "RESERVED"; break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:       type = "ACPI_RECLAIMABLE"; break;
            case LIMINE_MEMMAP_ACPI_NVS:               type = "ACPI_NVS"; break;
            case LIMINE_MEMMAP_BAD_MEMORY:             type = "BAD"; break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: type = "BOOTLOADER"; break;
            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES: type = "KERNEL/MODULES"; break;
            case LIMINE_MEMMAP_FRAMEBUFFER:            type = "FRAMEBUFFER"; break;
            default:                                   type = "UNKNOWN"; break;
        }

        printk("[%02zu] %s: [0x%016lx - 0x%016lx] (%lu bytes)\n",
               i, type,
               e->base, e->base + e->length - 1,
               e->length);
    }
}
