/*#include "vmm.h"
#include "pmm.h"
#include "memmap.h"
#include "global.h"
#include "printk.h"

#include <string.h>

#define ENTRIES_PER_TABLE 512

#define PML4_INDEX(x) (((x) >> 39) & 0x1FF)
#define PDPT_INDEX(x) (((x) >> 30) & 0x1FF)
#define PD_INDEX(x)   (((x) >> 21) & 0x1FF)
#define PT_INDEX(x)   (((x) >> 12) & 0x1FF)

static uintptr_t current_pml4 = 0;

// phys->virt via HHDM
static inline void *p2v(uintptr_t phys) {
    return (void *)(g_hhdm_offset + phys);
}

// allocate + zero a new page table
static uintptr_t alloc_table(void) {
    uintptr_t phys = pmm_alloc_page();
    if (!phys) {
        printk("{RED}VMM: out of memory for page tables!{RESET}\n");
        for (;;) asm("hlt");
    }
    memset(p2v(phys), 0, PAGE_SIZE);
    return phys;
}

// walk page tables, return final PTE pointer (in HHDM)
static uint64_t *walk(uintptr_t virt, int create) {
    uintptr_t pml4_phys = current_pml4;
    uint64_t *pml4 = (uint64_t *)p2v(pml4_phys);

    // PML4
    uint64_t *pml4e = &pml4[PML4_INDEX(virt)];
    if (!(*pml4e & VMM_PRESENT)) {
        if (!create) return NULL;
        uintptr_t new = alloc_table();
        *pml4e = new | VMM_PRESENT | VMM_WRITE;
    }
    uint64_t *pdpt = (uint64_t *)p2v(*pml4e & ~0xFFF);

    // PDPT
    uint64_t *pdpte = &pdpt[PDPT_INDEX(virt)];
    if (!(*pdpte & VMM_PRESENT)) {
        if (!create) return NULL;
        uintptr_t new = alloc_table();
        *pdpte = new | VMM_PRESENT | VMM_WRITE;
    }
    uint64_t *pd = (uint64_t *)p2v(*pdpte & ~0xFFF);

    // PD
    uint64_t *pde = &pd[PD_INDEX(virt)];
    if (!(*pde & VMM_PRESENT)) {
        if (!create) return NULL;
        uintptr_t new = alloc_table();
        *pde = new | VMM_PRESENT | VMM_WRITE;
    }
    uint64_t *pt = (uint64_t *)p2v(*pde & ~0xFFF);

    return &pt[PT_INDEX(virt)];
}

void vmm_map(uintptr_t virt, uintptr_t phys, uint64_t flags) {
    uint64_t *pte = walk(virt, 1);
    *pte = (phys & ~0xFFF) | flags | VMM_PRESENT;
}

void vmm_unmap(uintptr_t virt) {
    uint64_t *pte = walk(virt, 0);
    if (pte) *pte = 0;
}

uintptr_t vmm_resolve(uintptr_t virt) {
    uint64_t *pte = walk(virt, 0);
    if (!pte || !(*pte & VMM_PRESENT)) return 0;
    return *pte & ~0xFFF;
}

void vmm_load_cr3(uintptr_t phys_addr) {
    asm volatile("mov %0, %%cr3" :: "r"(phys_addr) : "memory");
    current_pml4 = phys_addr;
}

void vmm_init(void) {
    printk("VMM: initializing fresh page tables...\n");

    // allocate new PML4
    current_pml4 = alloc_table();

    // 1) Map all usable + kernel regions into HHDM
    for (uint64_t i = 0; i < g_memmap->entry_count; i++) {
        struct limine_memmap_entry *e = g_memmap->entries[i];
        if (e->type != LIMINE_MEMMAP_USABLE &&
            e->type != LIMINE_MEMMAP_EXECUTABLE_AND_MODULES) continue;

        for (uintptr_t addr = e->base;
            addr < e->base + e->length;
            addr += PAGE_SIZE) {
            vmm_map(g_hhdm_offset + addr, addr, VMM_WRITE);
        }
    }

    // 2) Map kernel higher-half
    for (uint64_t i = 0; i < g_memmap->entry_count; i++) {
        struct limine_memmap_entry *e = g_memmap->entries[i];
        if (e->type == LIMINE_MEMMAP_EXECUTABLE_AND_MODULES) {
            for (uintptr_t off = 0; off < e->length; off += PAGE_SIZE) {
                vmm_map(0xFFFFFFFF80000000ULL + off, e->base + off, VMM_WRITE);
            }
        }
    }

    // 3) Identity-map usable memory (for now, until we drop it later)
    for (uint64_t i = 0; i < g_memmap->entry_count; i++) {
        struct limine_memmap_entry *e = g_memmap->entries[i];
        if (e->type != LIMINE_MEMMAP_USABLE) continue;

        for (uintptr_t addr = e->base;
             addr < e->base + e->length;
             addr += PAGE_SIZE) {
            vmm_map(addr, addr, VMM_WRITE);
        }
    }

    // 4) Map bitmap so PMM still works
    extern uint8_t *pmm_bitmap;
    extern size_t pmm_bitmap_size;
    uintptr_t bitmap_phys = virt_to_phys(pmm_bitmap);
    for (size_t i = 0; i < pmm_bitmap_size; i += PAGE_SIZE) {
        vmm_map(bitmap_phys + i, bitmap_phys + i, VMM_WRITE);
    }

    // Load CR3
    vmm_load_cr3(current_pml4);
    printk("VMM: fresh tables loaded and active.\n");
}
*/