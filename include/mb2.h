#pragma once

#include <stdint.h>
#include "framebuffer.h"

#define MULTIBOOT2_MAGIC 0x36D76289u

struct mb2_mmap_entry {
	uint64_t addr;
	uint64_t len;
	uint32_t type;
	uint32_t reserved;
};

int mb2_find_framebuffer(uint32_t mb_info_addr, struct framebuffer *out_fb);
int mb2_find_basic_meminfo(uint32_t mb_info_addr, uint32_t *mem_lower_kb, uint32_t *mem_upper_kb);
int mb2_get_mmap(uint32_t mb_info_addr,
				 const struct mb2_mmap_entry **entries,
				 uint32_t *entry_size,
				 uint32_t *entry_count);
