#include "pmm.h"
#include "kprint.h"
#include "memmap.h"
#include "global.h"
#include "printk.h"

#define PAGE_SIZE 4096

uint8_t *pmm_bitmap = NULL;
size_t pmm_bitmap_size = 0;
static uintptr_t managed_base = 0;
static size_t total_pages = 0;

#define BIT_SET(b, i)   ((b)[(i)/8] |=  (1 << ((i) % 8)))
#define BIT_CLEAR(b, i) ((b)[(i)/8] &= ~(1 << ((i) % 8)))
#define BIT_TEST(b, i)  ((b)[(i)/8] &   (1 << ((i) % 8)))

void pmm_init(void) {
    struct limine_memmap_entry *big = memmap_find_biggest_region();
    if (!big) {
        kprint(LOG_ERR, "PMM: No usable region found!\n");
        for (;;) asm("hlt");
    }

    managed_base = big->base;
    total_pages  = big->length / PAGE_SIZE;
    pmm_bitmap_size  = (total_pages + 7) / 8;

    // Place pmm_bitmap at start of that region via HHDM
    pmm_bitmap = (uint8_t *)(g_hhdm_offset + managed_base);

    // Mark all pages as used
    for (size_t i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0xFF;
    }

    // Free all pages after the pmm_bitmap itself
    size_t used_pages = (pmm_bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = used_pages; i < total_pages; i++) {
        BIT_CLEAR(pmm_bitmap, i);
    }
    if (debug) kprint(LOG_DEBUG, "Physical Memory Manager initialized\n");
}

uintptr_t pmm_alloc_page(void) {
    for (size_t i = 0; i < total_pages; i++) {
        if (!BIT_TEST(pmm_bitmap, i)) {
            BIT_SET(pmm_bitmap, i);
            return managed_base + i * PAGE_SIZE; // physical
        }
    }
    return 0; // out of memory
}

void pmm_free_page(uintptr_t phys_addr) {
    if (phys_addr < managed_base) return;
    size_t i = (phys_addr - managed_base) / PAGE_SIZE;
    if (i < total_pages) {
        BIT_CLEAR(pmm_bitmap, i);
    }
}

/* Helper to map phys -> virt */
void *pmm_alloc_page_hhdm(void) {
    uintptr_t phys = pmm_alloc_page();
    if (!phys) return NULL;
    return (void *)(g_hhdm_offset + phys);
}
