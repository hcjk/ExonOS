#pragma once

#include <stdint.h>

struct memory_stats {
    uint32_t total_pages;
    uint32_t free_pages;
    uint32_t used_pages;
    uint32_t total_kb;
};

void memory_init(uint32_t mb_info_addr);
void *kmalloc(uint32_t size, uint32_t align);
uint32_t phys_alloc_page(void);
void phys_free_page(uint32_t addr);
void memory_get_stats(struct memory_stats *out);
