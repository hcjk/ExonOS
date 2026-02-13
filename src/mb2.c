#include "mb2.h"

struct mb2_tag {
    uint32_t type;
    uint32_t size;
};

struct mb2_tag_framebuffer {
    uint32_t type;
    uint32_t size;
    uint64_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
};

struct mb2_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

struct mb2_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct mb2_mmap_entry entries[];
};

int mb2_find_framebuffer(uint32_t mb_info_addr, struct framebuffer *out_fb) {
    if (!out_fb) {
        return 0;
    }
    out_fb->base = 0;
    out_fb->width = 0;
    out_fb->height = 0;
    out_fb->pitch = 0;
    out_fb->bpp = 0;

    uint8_t *base = (uint8_t *)(uintptr_t)mb_info_addr;
    uint32_t total_size = *(uint32_t *)base;
    if (total_size < 8) {
        return 0;
    }

    uint8_t *ptr = base + 8;
    uint8_t *end = base + total_size;

    while (ptr + sizeof(struct mb2_tag) <= end) {
        struct mb2_tag *tag = (struct mb2_tag *)ptr;
        if (tag->type == 0) {
            break;
        }
        if (tag->type == 8 && tag->size >= sizeof(struct mb2_tag_framebuffer)) {
            struct mb2_tag_framebuffer *fb_tag = (struct mb2_tag_framebuffer *)tag;
            out_fb->base = (uint8_t *)(uintptr_t)fb_tag->addr;
            out_fb->width = fb_tag->width;
            out_fb->height = fb_tag->height;
            out_fb->pitch = fb_tag->pitch;
            out_fb->bpp = fb_tag->bpp;
            return 1;
        }
        uint32_t next = (tag->size + 7u) & ~7u;
        if (next == 0) {
            break;
        }
        ptr += next;
    }

    return 0;
}

int mb2_find_basic_meminfo(uint32_t mb_info_addr, uint32_t *mem_lower_kb, uint32_t *mem_upper_kb) {
    if (!mem_lower_kb || !mem_upper_kb) {
        return 0;
    }
    *mem_lower_kb = 0;
    *mem_upper_kb = 0;

    uint8_t *base = (uint8_t *)(uintptr_t)mb_info_addr;
    uint32_t total_size = *(uint32_t *)base;
    if (total_size < 8) {
        return 0;
    }

    uint8_t *ptr = base + 8;
    uint8_t *end = base + total_size;

    while (ptr + sizeof(struct mb2_tag) <= end) {
        struct mb2_tag *tag = (struct mb2_tag *)ptr;
        if (tag->type == 0) {
            break;
        }
        if (tag->type == 4 && tag->size >= sizeof(struct mb2_tag_basic_meminfo)) {
            struct mb2_tag_basic_meminfo *mem = (struct mb2_tag_basic_meminfo *)tag;
            *mem_lower_kb = mem->mem_lower;
            *mem_upper_kb = mem->mem_upper;
            return 1;
        }
        uint32_t next = (tag->size + 7u) & ~7u;
        if (next == 0) {
            break;
        }
        ptr += next;
    }

    return 0;
}

int mb2_get_mmap(uint32_t mb_info_addr,
                 const struct mb2_mmap_entry **entries,
                 uint32_t *entry_size,
                 uint32_t *entry_count) {
    if (!entries || !entry_size || !entry_count) {
        return 0;
    }
    *entries = 0;
    *entry_size = 0;
    *entry_count = 0;

    uint8_t *base = (uint8_t *)(uintptr_t)mb_info_addr;
    uint32_t total_size = *(uint32_t *)base;
    if (total_size < 8) {
        return 0;
    }

    uint8_t *ptr = base + 8;
    uint8_t *end = base + total_size;

    while (ptr + sizeof(struct mb2_tag) <= end) {
        struct mb2_tag *tag = (struct mb2_tag *)ptr;
        if (tag->type == 0) {
            break;
        }
        if (tag->type == 6 && tag->size >= sizeof(struct mb2_tag_mmap)) {
            struct mb2_tag_mmap *mmap = (struct mb2_tag_mmap *)tag;
            uint32_t count = 0;
            if (mmap->entry_size != 0) {
                count = (mmap->size - sizeof(struct mb2_tag_mmap)) / mmap->entry_size;
            }
            *entries = mmap->entries;
            *entry_size = mmap->entry_size;
            *entry_count = count;
            return 1;
        }
        uint32_t next = (tag->size + 7u) & ~7u;
        if (next == 0) {
            break;
        }
        ptr += next;
    }

    return 0;
}
