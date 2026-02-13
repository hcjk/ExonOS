#pragma once

#include <stdint.h>

int ata_init(void);
int ata_read28(uint32_t lba, uint8_t count, void *buffer);
