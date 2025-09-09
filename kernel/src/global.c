#include "global.h"
#include "limine.h"
#include <stdint.h>

struct flanterm_context *g_ft_ctx = NULL;
struct limine_memmap_response *g_memmap = NULL;
uint64_t g_hhdm_offset = 0;
bool g_kheap_ready = false;
bool debug = false;
uint64_t *g_rsdp = NULL;
struct limine_framebuffer *g_fb = NULL;
