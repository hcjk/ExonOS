#include "memory.h"
#include "mb2.h"
#include "panic.h"

#define PAGE_SIZE 4096u
#define HEAP_SIZE (4u * 1024u * 1024u)

extern uint8_t _kernel_end;

static uint8_t *heap_curr;
static uint8_t *heap_limit;
static uint8_t *bitmap;
static uint32_t total_pages;
static uint32_t free_pages;

static uint32_t align_up(uint32_t value, uint32_t align) {
    return (value + align - 1u) & ~(align - 1u);
}

static void bitmap_set(uint32_t idx) {
    bitmap[idx / 8] |= (uint8_t)(1u << (idx % 8));
}

static void bitmap_clear(uint32_t idx) {
    bitmap[idx / 8] &= (uint8_t)~(1u << (idx % 8));
}

static int bitmap_test(uint32_t idx) {
    return (bitmap[idx / 8] & (uint8_t)(1u << (idx % 8))) != 0;
}

static void reserve_range(uint32_t start, uint32_t end) {
    uint32_t page_start = start / PAGE_SIZE;
    uint32_t page_end = (end + PAGE_SIZE - 1u) / PAGE_SIZE;
    for (uint32_t i = page_start; i < page_end && i < total_pages; ++i) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            if (free_pages > 0) {
                free_pages--;
            }
        }
    }
}

static void free_range(uint32_t start, uint32_t end) {
    uint32_t page_start = start / PAGE_SIZE;
    uint32_t page_end = (end + PAGE_SIZE - 1u) / PAGE_SIZE;
    for (uint32_t i = page_start; i < page_end && i < total_pages; ++i) {
        if (bitmap_test(i)) {
            bitmap_clear(i);
            free_pages++;
        }
    }
}

void memory_init(uint32_t mb_info_addr) {
    const struct mb2_mmap_entry *entries = 0;
    uint32_t entry_size = 0;
    uint32_t entry_count = 0;
    if (!mb2_get_mmap(mb_info_addr, &entries, &entry_size, &entry_count)) {
        panic("No memory map");
    }

    uint64_t max_addr = 0;
    for (uint32_t i = 0; i < entry_count; ++i) {
        const struct mb2_mmap_entry *entry = (const struct mb2_mmap_entry *)((const uint8_t *)entries + i * entry_size);
        uint64_t end = entry->addr + entry->len;
        if (end > max_addr) {
            max_addr = end;
        }
    }
    if (max_addr == 0) {
        panic("Empty memory map");
    }
    if (max_addr > 0xFFFFFFFFull) {
        max_addr = 0xFFFFFFFFull;
    }

    uint32_t max_addr32 = (uint32_t)max_addr;
    uint64_t pages64 = (max_addr + PAGE_SIZE - 1u) / PAGE_SIZE;
    if (pages64 > 0xFFFFFFFFull) {
        pages64 = 0xFFFFFFFFull;
    }
    total_pages = (uint32_t)pages64;
    if (total_pages == 0) {
        panic("No usable memory");
    }

    uint32_t bitmap_bytes = (total_pages + 7u) / 8u;
    uint32_t kernel_end = align_up((uint32_t)(uintptr_t)&_kernel_end, 16u);
    bitmap = (uint8_t *)(uintptr_t)kernel_end;
    for (uint32_t i = 0; i < bitmap_bytes; ++i) {
        bitmap[i] = 0xFF;
    }
    free_pages = 0;

    for (uint32_t i = 0; i < entry_count; ++i) {
        const struct mb2_mmap_entry *entry = (const struct mb2_mmap_entry *)((const uint8_t *)entries + i * entry_size);
        if (entry->type != 1) {
            continue;
        }
        uint64_t start64 = entry->addr;
        uint64_t end64 = entry->addr + entry->len;
        if (start64 > 0xFFFFFFFFull) {
            continue;
        }
        if (end64 > 0xFFFFFFFFull) {
            end64 = 0xFFFFFFFFull;
        }
        uint32_t start = (uint32_t)start64;
        uint32_t end = (uint32_t)end64;
        free_range(start, end);
    }

    uint32_t bitmap_end = align_up((uint32_t)(uintptr_t)bitmap + bitmap_bytes, 16u);
    heap_curr = (uint8_t *)(uintptr_t)bitmap_end;
    heap_limit = heap_curr + HEAP_SIZE;
    if ((uint32_t)(uintptr_t)heap_limit > max_addr32) {
        heap_limit = (uint8_t *)(uintptr_t)max_addr32;
    }

    reserve_range(0, (uint32_t)(uintptr_t)heap_limit);
}

void *kmalloc(uint32_t size, uint32_t align) {
    if (size == 0) {
        return 0;
    }
    if (align == 0) {
        align = 4;
    }
    uint32_t curr = align_up((uint32_t)(uintptr_t)heap_curr, align);
    uint32_t next = curr + size;
    if (next > (uint32_t)(uintptr_t)heap_limit) {
        return 0;
    }
    heap_curr = (uint8_t *)(uintptr_t)next;
    return (void *)(uintptr_t)curr;
}

uint32_t phys_alloc_page(void) {
    static uint32_t last = 0;
    for (uint32_t i = 0; i < total_pages; ++i) {
        uint32_t idx = (last + i) % total_pages;
        if (!bitmap_test(idx)) {
            bitmap_set(idx);
            if (free_pages > 0) {
                free_pages--;
            }
            last = idx + 1;
            return idx * PAGE_SIZE;
        }
    }
    return 0;
}

void phys_free_page(uint32_t addr) {
    if (addr % PAGE_SIZE != 0) {
        return;
    }
    uint32_t idx = addr / PAGE_SIZE;
    if (idx >= total_pages) {
        return;
    }
    if (bitmap_test(idx)) {
        bitmap_clear(idx);
        free_pages++;
    }
}

void memory_get_stats(struct memory_stats *out) {
    if (!out) {
        return;
    }
    out->total_pages = total_pages;
    out->free_pages = free_pages;
    out->used_pages = total_pages - free_pages;
    out->total_kb = total_pages * (PAGE_SIZE / 1024u);
}
