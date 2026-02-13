#pragma once

#include <stdint.h>

struct fat_fs {
    uint32_t part_lba;
    uint32_t fat_lba;
    uint32_t root_dir_lba;
    uint32_t data_lba;
    uint32_t sectors_per_fat;
    uint32_t total_sectors;
    uint32_t root_dir_sectors;
    uint32_t root_cluster;
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint8_t fat_count;
    uint8_t fat_type;
};

typedef void (*fat_dir_cb)(const char *name, uint32_t size, uint8_t attr, void *ctx);

int fat_mount(struct fat_fs *fs, uint32_t part_lba);
int fat_list_root(struct fat_fs *fs, fat_dir_cb cb, void *ctx);
