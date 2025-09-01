#ifndef KHEAP_H
#define KHEAP_H

#include <stddef.h>
#include <stdint.h>

// Initialize heap using biggest usable region from memmap
size_t kheap_init_auto(void);

// Core bump allocator
void *kheap_alloc(size_t size);
void  kheap_free(void *ptr);

// Wrappers
void *kmalloc(size_t size);
void *kzalloc(size_t size);
void *kcalloc(size_t n, size_t size);
void  kfree(void *ptr);

#endif // KHEAP_H
