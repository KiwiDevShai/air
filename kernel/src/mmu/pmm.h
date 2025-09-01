#ifndef PMM_H
#define PMM_H

#include <stddef.h>
#include <stdint.h>

void pmm_init(void);
uintptr_t pmm_alloc_page(void);
void pmm_free_page(uintptr_t phys_addr);
void *pmm_alloc_page_hhdm(void);

/* expose bitmap info so VMM can map it */
extern uint8_t *pmm_bitmap;
extern size_t pmm_bitmap_size;

#endif // PMM_H

