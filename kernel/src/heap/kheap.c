#include "kheap.h"
#include "kprint.h"
#include "mmu/pmm.h"
#include "mmu/vmm.h"
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#define HEAP_START 0xFFFF800010000000
#define HEAP_MAX   0xFFFF800020000000 // 256MB of heap space
#define PAGE_SIZE  0x1000

typedef struct block_header {
    size_t size;
    int free;
    struct block_header *next;
} block_header_t;

static uintptr_t heap_current = HEAP_START;
static block_header_t *free_list = NULL;

static void grow_heap(size_t required_size) {
    while (heap_current + required_size > vmm_resolve(heap_current)) {
        uintptr_t phys = pmm_alloc_page();
        if (!phys) {
            kprint(LOG_ERR, "KHEAP: failed to grow heap\n");
            return;
        }
        vmm_map(heap_current, phys, VMM_PRESENT | VMM_WRITE);
        heap_current += PAGE_SIZE;
    }
}

size_t kheap_init(void) {
    heap_current = HEAP_START;
    // Pre-map 1 page
    uintptr_t phys = pmm_alloc_page();
    if (!phys) {
        kprint(LOG_ERR, "KHEAP: initial page allocation failed\n");
        return 0;
    }
    vmm_map(heap_current, phys, VMM_PRESENT | VMM_WRITE);
    free_list = (block_header_t *)heap_current;
    free_list->size = PAGE_SIZE - sizeof(block_header_t);
    free_list->free = 1;
    free_list->next = NULL;
    kprint(LOG_DEBUG, "KHEAP: initialized dynamic heap at %p\n", (void *)heap_current);
    return free_list->size;
}

static void split_block(block_header_t *block, size_t size) {
    if (block->size <= size + sizeof(block_header_t)) return;

    block_header_t *new_block = (block_header_t *)((uintptr_t)block + sizeof(block_header_t) + size);
    new_block->size = block->size - size - sizeof(block_header_t);
    new_block->free = 1;
    new_block->next = block->next;

    block->size = size;
    block->next = new_block;
}

static void *request_more_memory(size_t size) {
    size_t total_size = size + sizeof(block_header_t);
    total_size = (total_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    uintptr_t alloc_base = heap_current;
    for (size_t off = 0; off < total_size; off += PAGE_SIZE) {
        uintptr_t phys = pmm_alloc_page();
        if (!phys) return NULL;
        vmm_map(alloc_base + off, phys, VMM_PRESENT | VMM_WRITE);
    }

    heap_current += total_size;

    block_header_t *block = (block_header_t *)alloc_base;
    block->size = total_size - sizeof(block_header_t);
    block->free = 0;
    block->next = NULL;
    return (void *)((uintptr_t)block + sizeof(block_header_t));
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
        if (!curr->next) break;
        curr = curr->next;
    }

    void *new_block = request_more_memory(size);
    if (!new_block) {
        kprint(LOG_ERR, "KHEAP: out of memory!\n");
        return NULL;
    }

    curr->next = (block_header_t *)((uintptr_t)new_block - sizeof(block_header_t));
    return new_block;
}

void kheap_free(void *ptr) {
    if (!ptr) return;

    block_header_t *block = (block_header_t *)((uintptr_t)ptr - sizeof(block_header_t));
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
    void *ptr = kmalloc(size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

void *kcalloc(size_t n, size_t size) {
    if (n && size && n > (SIZE_MAX / size)) return NULL;
    return kzalloc(n * size);
}

void kfree(void *ptr) {
    kheap_free(ptr);
}
