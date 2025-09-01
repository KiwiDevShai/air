#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>

/* Page size */
#define PAGE_SIZE 0x1000

/* Paging flags */
#define VMM_PRESENT (1ull << 0)
#define VMM_WRITE (1ull << 1)
#define VMM_USER (1ull << 2)
#define VMM_WRITE_THR (1ull << 3)
#define VMM_CACHE_DIS (1ull << 4)
#define VMM_ACCESSED (1ull << 5)
#define VMM_DIRTY (1ull << 6)
#define VMM_HUGE (1ull << 7)
#define VMM_GLOBAL (1ull << 8)
#define VMM_NX (1ull << 63)

/* Init new page tables and switch to them */
void vmm_init(void);

/* Map a single 4 KiB page */
void vmm_map(uintptr_t virt, uintptr_t phys, uint64_t flags);

/* Unmap a single 4 KiB page */
void vmm_unmap(uintptr_t virt);

/* Resolve virtual -> physical (0 if not mapped) */
uintptr_t vmm_resolve(uintptr_t virt);

/* Switch CR3 to a new PML4 (phys address) */
void vmm_load_cr3(uintptr_t phys_addr);

#define MAP_PHYS_PAGE_TO(virt, phys) \
    vmm_map((virt), (phys) & ~0xFFFUL, VMM_PRESENT | VMM_WRITE); \
    asm volatile("invlpg (%0)" :: "r" ((void*)(virt)) : "memory")
#define MAP_ONE_STRUCT(type, virt, phys) \
    ({ MAP_PHYS_PAGE_TO(virt, phys); (type*)((virt) + ((phys) & 0xFFFUL)); })
#define UNMAP_ONE_STRUCT(virt) \
    vmm_unmap((uintptr_t)(virt) & ~0xFFFUL); \
    asm volatile("invlpg (%0)" :: "r" ((void*)((uintptr_t)(virt) & ~0xFFFUL)) : "memory")

#endif // VMM_H
