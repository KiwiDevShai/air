#include "vmm.h"
#include "kprint.h"
#include "pmm.h"
#include "global.h"
#include "printk.h"

#include <string.h>
#include <stdint.h>
#include <stddef.h>

#define ENTRIES_PER_TABLE 512
#define PAGE_MASK 0x000FFFFFFFFFF000ULL

#define PML4_INDEX(x) (((x) >> 39) & 0x1FF)
#define PDPT_INDEX(x) (((x) >> 30) & 0x1FF)
#define PD_INDEX(x) (((x) >> 21) & 0x1FF)
#define PT_INDEX(x) (((x) >> 12) & 0x1FF)

static uintptr_t current_pml4 = 0;

static inline void *p2v(uintptr_t phys) {
    return (void *)(g_hhdm_offset + phys);
}

static inline uintptr_t read_cr3(void) {
    uintptr_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static inline void invlpg(uintptr_t addr) {
    asm volatile("invlpg (%0)" :: "r"(addr) : "memory");
}

static uintptr_t alloc_table(void) {
    uintptr_t phys = pmm_alloc_page();
    if (!phys) {
        kprint(LOG_ERR, "VMM: out of memory for page tables!\n");
        for (;;) asm("hlt");
    }
    memset(p2v(phys), 0, PAGE_SIZE);
    return phys;
}

static uint64_t *walk(uintptr_t virt, int create) {
    uint64_t *pml4 = (uint64_t *)p2v(current_pml4);

    uint64_t *pml4e = &pml4[PML4_INDEX(virt)];
    if (!(*pml4e & VMM_PRESENT)) {
        if (!create) return NULL;
        uintptr_t new = alloc_table();
        *pml4e = (new & PAGE_MASK) | VMM_PRESENT | VMM_WRITE;
    }
    uint64_t *pdpt = (uint64_t *)p2v((*pml4e) & PAGE_MASK);

    uint64_t *pdpte = &pdpt[PDPT_INDEX(virt)];
    if (!(*pdpte & VMM_PRESENT)) {
        if (!create) return NULL;
        uintptr_t new = alloc_table();
        *pdpte = (new & PAGE_MASK) | VMM_PRESENT | VMM_WRITE;
    }
    uint64_t *pd = (uint64_t *)p2v((*pdpte) & PAGE_MASK);

    uint64_t *pde = &pd[PD_INDEX(virt)];
    if (!(*pde & VMM_PRESENT)) {
        if (!create) return NULL;
        uintptr_t new = alloc_table();
        *pde = (new & PAGE_MASK) | VMM_PRESENT | VMM_WRITE;
    }
    uint64_t *pt = (uint64_t *)p2v((*pde) & PAGE_MASK);

    return &pt[PT_INDEX(virt)];
}

void vmm_map(uintptr_t virt, uintptr_t phys, uint64_t flags) {
    uint64_t *pte = walk(virt, 1);
    *pte = (phys & PAGE_MASK) | flags | VMM_PRESENT;
    invlpg(virt);
}

void vmm_unmap(uintptr_t virt) {
    uint64_t *pte = walk(virt, 0);
    if (pte && (*pte & VMM_PRESENT)) {
        *pte = 0;
        invlpg(virt);
    }
}

uintptr_t vmm_resolve(uintptr_t virt) {
    uint64_t *pte = walk(virt, 0);
    if (!pte || !(*pte & VMM_PRESENT)) return 0;
    return ((*pte) & PAGE_MASK) | (virt & (PAGE_SIZE - 1));
}

void vmm_load_cr3(uintptr_t phys_addr) {
    current_pml4 = phys_addr & PAGE_MASK;
    asm volatile("mov %0, %%cr3" :: "r"(current_pml4) : "memory");
}

void vmm_init(void) {
    const uintptr_t old_cr3_phys = read_cr3() & PAGE_MASK;
    uint64_t *old_pml4 = (uint64_t *)p2v(old_cr3_phys);

    current_pml4 = alloc_table();
    uint64_t *new_pml4 = (uint64_t *)p2v(current_pml4);

    for (size_t i = 256; i < ENTRIES_PER_TABLE; ++i) {
        new_pml4[i] = old_pml4[i];
    }

    for (uint64_t i = 0; i < g_memmap->entry_count; i++) {
        struct limine_memmap_entry *e = g_memmap->entries[i];
        if (e->type != LIMINE_MEMMAP_USABLE) continue;

        uintptr_t end = e->base + e->length;
        for (uintptr_t addr = e->base; addr < end; addr += PAGE_SIZE) {
            vmm_map(addr, addr, VMM_WRITE);
        }
    }

    extern uint8_t *pmm_bitmap;
    extern size_t pmm_bitmap_size;

    if (pmm_bitmap && pmm_bitmap_size) {
        uintptr_t bitmap_phys = virt_to_phys(pmm_bitmap);
        for (size_t i = 0; i < pmm_bitmap_size; i += PAGE_SIZE) {
            vmm_map(bitmap_phys + i, bitmap_phys + i, VMM_WRITE);
        }
    }

    vmm_load_cr3(current_pml4);
    if (debug) kprint(LOG_DEBUG, "Virtual Memory Manager initialized\n");
}
