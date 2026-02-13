#pragma once

#include <stdint.h>

void vfs_init(void);
int vfs_mount_fat(uint32_t part_lba);
int vfs_list_root(void);
