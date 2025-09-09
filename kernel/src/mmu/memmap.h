#ifndef MEMMAP_H
#define MEMMAP_H

#include <limine.h>

/* Init memory map and assign g_memmap */
void memmap_init(struct limine_memmap_response *response);

/* Find the largest usable memory region */
struct limine_memmap_entry *memmap_find_biggest_region(void);

#endif /* MEMMAP_H */
