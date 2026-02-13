#include <stdint.h>

#include "framebuffer.h"
#include "input.h"
#include "ata.h"
#include "log.h"
#include "mb2.h"
#include "memory.h"
#include "mbr.h"
#include "panic.h"
#include "scheduler.h"
#include "usb.h"
#include "vfs.h"
#include "ui.h"

static void text_fallback(void) {
    volatile uint16_t *vga = (uint16_t *)0xB8000;
    const char *msg = "ExonOS: framebuffer unavailable";
    const uint8_t color = 0x1F;
    for (uint32_t i = 0; msg[i] != '\0'; ++i) {
        vga[i] = (uint16_t)msg[i] | ((uint16_t)color << 8);
    }
    log_puts("Framebuffer unavailable\n");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    uint32_t eax = 0;
    uint32_t ebx = 0;
    uint32_t ecx = 0;
    uint32_t edx = 0;
    __asm__ volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(leaf), "c"(subleaf));
    if (a) {
        *a = eax;
    }
    if (b) {
        *b = ebx;
    }
    if (c) {
        *c = ecx;
    }
    if (d) {
        *d = edx;
    }
}

static void str_copy(char *dst, const char *src, uint32_t max_len) {
    if (!dst || !src || max_len == 0) {
        return;
    }
    uint32_t i = 0;
    while (i + 1 < max_len && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static void str_trim_right(char *s) {
    if (!s) {
        return;
    }
    uint32_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    while (len > 0 && s[len - 1] == ' ') {
        s[len - 1] = '\0';
        len--;
    }
}

static void fill_cpu_string(char *out, uint32_t max_len) {
    if (!out || max_len == 0) {
        return;
    }
    out[0] = '\0';

    uint32_t max_basic = 0;
    uint32_t ebx = 0;
    uint32_t ecx = 0;
    uint32_t edx = 0;
    cpuid(0, 0, &max_basic, &ebx, &ecx, &edx);

    uint32_t max_ext = 0;
    cpuid(0x80000000u, 0, &max_ext, 0, 0, 0);

    if (max_ext >= 0x80000004u) {
        char brand[48];
        uint32_t *brand_u32 = (uint32_t *)brand;
        cpuid(0x80000002u, 0, &brand_u32[0], &brand_u32[1], &brand_u32[2], &brand_u32[3]);
        cpuid(0x80000003u, 0, &brand_u32[4], &brand_u32[5], &brand_u32[6], &brand_u32[7]);
        cpuid(0x80000004u, 0, &brand_u32[8], &brand_u32[9], &brand_u32[10], &brand_u32[11]);
        char brand_buf[49];
        for (uint32_t i = 0; i < 48; ++i) {
            brand_buf[i] = brand[i];
        }
        brand_buf[48] = '\0';
        str_trim_right(brand_buf);
        str_copy(out, brand_buf, max_len);
        return;
    }

    char vendor[13];
    ((uint32_t *)vendor)[0] = ebx;
    ((uint32_t *)vendor)[1] = edx;
    ((uint32_t *)vendor)[2] = ecx;
    vendor[12] = '\0';
    str_copy(out, vendor, max_len);
}

struct ui_task_ctx {
    struct framebuffer fb;
    struct framebuffer draw_fb;
    struct ui_state state;
};

static void usb_task(void *ctx) {
    (void)ctx;
    static uint32_t div;
    div++;
    if ((div & 0x7) == 0) {
        usb_poll();
    }
}

static void ui_task(void *ctx) {
    struct ui_task_ctx *ui = (struct ui_task_ctx *)ctx;
    static uint32_t frame_div;
    ui_update(&ui->state, &ui->draw_fb);
    frame_div++;
    if ((frame_div & 0x1) == 0) {
        ui_render(&ui->draw_fb, &ui->state);
        fb_blit(&ui->fb, &ui->draw_fb);
    }
}

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info_addr) {
    log_init();
    log_puts("ExonOS booting...\n");
    if (multiboot_magic != MULTIBOOT2_MAGIC) {
        panic("Bad Multiboot2 magic");
    }

    struct framebuffer fb = { 0 };
    if (!mb2_find_framebuffer(multiboot_info_addr, &fb)) {
        panic("No framebuffer tag");
    }
    if (fb.base == 0 || fb.bpp != 32) {
        text_fallback();
    }

    panic_set_framebuffer(&fb);
    fb_draw_string(&fb, 8, 8, "Booting...", rgb(255, 255, 255), rgb(0, 0, 0));
    fb_draw_string(&fb, 8, 24, "Step 1", rgb(255, 255, 255), rgb(0, 0, 0));

    memory_init(multiboot_info_addr);
    fb_draw_string(&fb, 8, 40, "Step 2", rgb(255, 255, 255), rgb(0, 0, 0));
    log_puts("Memory init done\n");

    vfs_init();
    if (ata_init()) {
        struct mbr mbr;
        if (mbr_read(&mbr)) {
            for (uint32_t i = 0; i < MBR_PARTITION_COUNT; ++i) {
                if (mbr.partitions[i].type == 0 || mbr.partitions[i].sectors == 0) {
                    continue;
                }
                log_puts("MBR: P");
                log_dec32(i);
                log_puts(" type=");
                log_hex32(mbr.partitions[i].type);
                log_puts(" lba=");
                log_dec32(mbr.partitions[i].lba_start);
                log_puts(" sectors=");
                log_dec32(mbr.partitions[i].sectors);
                log_puts("\n");

                if (vfs_mount_fat(mbr.partitions[i].lba_start)) {
                    log_puts("FAT root:\n");
                    vfs_list_root();
                    break;
                }
            }
        } else {
            log_puts("MBR read failed\n");
        }
    } else {
        log_puts("No ATA disk\n");
    }
    fb_draw_string(&fb, 8, 56, "Step 3", rgb(255, 255, 255), rgb(0, 0, 0));
    fb_draw_string(&fb, 8, 72, "USB scan...", rgb(255, 255, 255), rgb(0, 0, 0));

    usb_init();
    fb_draw_string(&fb, 8, 88, "Step 4", rgb(255, 255, 255), rgb(0, 0, 0));

    enum { BACKBUFFER_W = 1024, BACKBUFFER_H = 768 };
    static uint32_t backbuffer[BACKBUFFER_W * BACKBUFFER_H];
    if (fb.width > BACKBUFFER_W || fb.height > BACKBUFFER_H) {
        panic("Framebuffer too large");
    }
    struct framebuffer draw_fb = fb;
    draw_fb.base = (uint8_t *)backbuffer;
    draw_fb.pitch = fb.width * 4;

    init_ps2_mouse();

    struct system_info info;
    str_copy(info.version, "Exon OS 0.0.8", sizeof(info.version));
    str_copy(info.kernel, "Exon kernel", sizeof(info.kernel));
    fill_cpu_string(info.cpu, sizeof(info.cpu));
    info.width = fb.width;
    info.height = fb.height;
    info.bpp = fb.bpp;
    uint32_t mem_lower = 0;
    uint32_t mem_upper = 0;
    if (mb2_find_basic_meminfo(multiboot_info_addr, &mem_lower, &mem_upper)) {
        info.ram_kb = mem_lower + mem_upper;
    } else {
        info.ram_kb = 0;
    }
    log_puts("Init UI\n");

    struct ui_task_ctx ui_ctx;
    ui_ctx.fb = fb;
    ui_ctx.draw_fb = draw_fb;
    ui_init(&ui_ctx.state, &draw_fb, &info);
    fb_draw_string(&fb, 8, 104, "Step 5", rgb(255, 255, 255), rgb(0, 0, 0));

    scheduler_init();
    scheduler_add(ui_task, &ui_ctx);
    scheduler_add(usb_task, 0);
    log_puts("Scheduler start\n");
    fb_draw_string(&fb, 8, 120, "Step 6", rgb(255, 255, 255), rgb(0, 0, 0));

    for (;;) {
        scheduler_tick();
    }
}
