#include "panic.h"
#include "framebuffer.h"
#include "log.h"
#include <stdint.h>

static const struct framebuffer *panic_fb;

static void vga_panic(const char *msg) {
    volatile uint16_t *vga = (uint16_t *)0xB8000;
    const uint8_t color = 0x4F;
    const char *prefix = "PANIC: ";
    uint32_t i = 0;
    while (prefix[i] != '\0') {
        vga[i] = (uint16_t)prefix[i] | ((uint16_t)color << 8);
        i++;
    }
    if (msg) {
        uint32_t j = 0;
        while (msg[j] != '\0') {
            vga[i + j] = (uint16_t)msg[j] | ((uint16_t)color << 8);
            j++;
        }
    }
}

void panic_set_framebuffer(const struct framebuffer *fb) {
    panic_fb = fb;
}

static void fb_panic(const char *msg) {
    if (!panic_fb || !panic_fb->base || panic_fb->bpp != 32) {
        return;
    }
    struct rect banner = { 0, 0, (int)panic_fb->width, 48 };
    fb_fill_rect(panic_fb, banner, rgb(120, 0, 0));
    fb_draw_string(panic_fb, 8, 16, "PANIC:", rgb(255, 255, 255), rgb(120, 0, 0));
    if (msg) {
        fb_draw_string(panic_fb, 72, 16, msg, rgb(255, 255, 255), rgb(120, 0, 0));
    }
}

void panic(const char *msg) {
    log_puts("PANIC: ");
    if (msg) {
        log_puts(msg);
    }
    log_puts("\n");
    vga_panic(msg);
    fb_panic(msg);
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}
