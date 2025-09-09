#include "kheap.h"
#include "kprint.h"
#include "mmu/memmap.h"
#include "mmu/pmm.h"
#include "mmu/vmm.h"
#include "global.h"
#include "printk.h"
#include <string.h>
#include <stdint.h>
#include <stddef.h>

static uintptr_t heap_start_virt = 0;
static uintptr_t heap_end_virt   = 0;

extern uint8_t *pmm_bitmap;
extern size_t pmm_bitmap_size;

typedef struct block_header {
    size_t size;
    int free;
    struct block_header *next;
} block_header_t;

static block_header_t *free_list = NULL;

static inline void pmm_mark_frame_used(size_t frame_idx) {
    pmm_bitmap[frame_idx >> 3] |= (uint8_t)(1u << (frame_idx & 7u));
}

static void pmm_reserve_range(uintptr_t phys, size_t length) {
    if (!pmm_bitmap || !pmm_bitmap_size || !length) return;

    const size_t first = (size_t)(phys / PAGE_SIZE);
    const size_t pages = (length + (PAGE_SIZE - 1)) / PAGE_SIZE;
    const size_t max_frames = pmm_bitmap_size * 8u;

    for (size_t i = 0; i < pages; ++i) {
        size_t f = first + i;
        if (f < max_frames) pmm_mark_frame_used(f);
    }
}

size_t kheap_init(void) {
    struct limine_memmap_entry *big = memmap_find_biggest_region();
    if (!big) {
        kprint(LOG_ERR, "KHEAP: no usable region found!\n");
        return 0;
    }

    uintptr_t heap_start_phys = (big->base + 0xFFF) & ~0xFFFULL;
    size_t available_size  = (size_t)(big->length - (heap_start_phys - big->base));

    size_t heap_size = available_size / 2;
    const size_t max_heap = 512ULL * 1024 * 1024;
    if (heap_size > max_heap) heap_size = max_heap;

    uintptr_t heap_end_phys = heap_start_phys + heap_size;

    heap_start_virt = g_hhdm_offset + heap_start_phys;
    heap_end_virt = g_hhdm_offset + heap_end_phys;

    pmm_reserve_range(heap_start_phys, heap_size);

    free_list = (block_header_t *)heap_start_virt;
    free_list->size = heap_size - sizeof(block_header_t);
    free_list->free = 1;
    free_list->next = NULL;

    g_kheap_ready = true;
    if (debug) kprint(LOG_DEBUG, "Heap initialized\n");
    return free_list->size;
}

static void split_block(block_header_t *block, size_t size) {
    if (block->size <= size + sizeof(block_header_t)) return;

    block_header_t *new_block =
        (block_header_t *)((uintptr_t)block + sizeof(block_header_t) + size);

    new_block->size = block->size - size - sizeof(block_header_t);
    new_block->free = 1;
    new_block->next = block->next;

    block->size = size;
    block->next = new_block;
}

void *kheap_alloc(size_t size) {
    if (size == 0) return NULL;
    size = (size + 15) & ~15ULL;

    block_header_t *curr = free_list;
    while (curr) {
        if (curr->free && curr->size >= size) {
            split_block(curr, size);
            curr->free = 0;
            return (void *)((uintptr_t)curr + sizeof(block_header_t));
        }
        curr = curr->next;
    }

    printk("{RED}KHEAP: out of memory!{RESET}\n");
    return NULL;
}

void kheap_free(void *ptr) {
    if (!ptr) return;

    block_header_t *block =
        (block_header_t *)((uintptr_t)ptr - sizeof(block_header_t));
    block->free = 1;

    block_header_t *curr = free_list;
    while (curr) {
        if (curr->free && curr->next && curr->next->free) {
            curr->size += sizeof(block_header_t) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

void *kmalloc(size_t size) {
    return kheap_alloc(size);
}

void *kzalloc(size_t size) {
    void *ptr = kheap_alloc(size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

void *kcalloc(size_t n, size_t size) {
    if (n && size && n > (SIZE_MAX / size)) return NULL;
    size_t total = n * size;
    return kzalloc(total);
}

void kfree(void *ptr) {
    kheap_free(ptr);
}
