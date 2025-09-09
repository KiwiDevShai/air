#include "memmap.h"
#include "global.h"
#include "kprint.h"

void memmap_init(struct limine_memmap_response *response) {
    g_memmap = response;

    if (!g_memmap) {
        kprint(LOG_ERR, "No memory map provided by bootloader!\n");
        for (;;); // eternal punishment for forgetting the memory map
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
