#include "video.h"
#include "fonts.h"
#include "global.h"
#include "heap/kheap.h"
#include <flanterm_backends/fb.h>

static void* current_font = 0;

// Wrapper functions to satisfy flanterm_fb_set_font signature
static void* font_malloc(size_t size) {
    return kheap_alloc(size);
}

static void font_free(void* ptr, size_t size) {
    (void)size; // kheap_free doesn't use the size
    kheap_free(ptr);
}

void ft_init(void* font, int font_size_x, int font_size_y, int font_scale_x, int font_scale_y) {
    if (!font) {
        font = (void*)default_font;
    }

    current_font = font;

    g_ft_ctx = flanterm_fb_init(
        NULL, NULL,
        g_fb->address, g_fb->width, g_fb->height, g_fb->pitch,
        g_fb->red_mask_size, g_fb->red_mask_shift,
        g_fb->green_mask_size, g_fb->green_mask_shift,
        g_fb->blue_mask_size, g_fb->blue_mask_shift,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        font, font_size_x, font_size_y, 1,
        font_scale_x, font_scale_y, 0
    );
}

void ft_set_font(void* font, int font_size_x, int font_size_y) {
    flanterm_fb_set_font(
        g_ft_ctx,
        font,
        (size_t)font_size_x,
        (size_t)font_size_y,
        1,
        g_kheap_ready ? font_malloc : NULL,
        g_kheap_ready ? font_free  : NULL
    );
}
