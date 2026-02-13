#pragma once

#include <stdint.h>

#define MBR_PARTITION_COUNT 4

struct mbr_partition {
    uint8_t status;
    uint8_t chs_first[3];
    uint8_t type;
    uint8_t chs_last[3];
    uint32_t lba_start;
    uint32_t sectors;
} __attribute__((packed));

struct mbr {
    uint8_t boot_code[446];
    struct mbr_partition partitions[MBR_PARTITION_COUNT];
    uint16_t signature;
} __attribute__((packed));

int mbr_read(struct mbr *out);
