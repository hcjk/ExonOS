#include "ata.h"
#include "portio.h"

#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

#define ATA_REG_DATA 0
#define ATA_REG_ERROR 1
#define ATA_REG_FEATURES 1
#define ATA_REG_SECCOUNT 2
#define ATA_REG_LBA0 3
#define ATA_REG_LBA1 4
#define ATA_REG_LBA2 5
#define ATA_REG_HDDEVSEL 6
#define ATA_REG_STATUS 7
#define ATA_REG_COMMAND 7

#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_READ_SECTORS 0x20

#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_ERR 0x01

static void io_wait(void) {
    outb(0x80, 0);
}

static int ata_wait_not_busy(void) {
    for (uint32_t i = 0; i < 100000; ++i) {
        uint8_t status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
        if ((status & ATA_STATUS_BSY) == 0) {
            return 1;
        }
    }
    return 0;
}

static int ata_wait_drq(void) {
    for (uint32_t i = 0; i < 100000; ++i) {
        uint8_t status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
        if (status & ATA_STATUS_ERR) {
            return 0;
        }
        if (status & ATA_STATUS_DRQ) {
            return 1;
        }
    }
    return 0;
}

int ata_init(void) {
    outb(ATA_PRIMARY_CTRL, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);
    io_wait();

    if (!ata_wait_not_busy()) {
        return 0;
    }

    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    uint8_t status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
    if (status == 0) {
        return 0;
    }

    if (!ata_wait_drq()) {
        return 0;
    }

    for (uint32_t i = 0; i < 256; ++i) {
        (void)inw(ATA_PRIMARY_IO + ATA_REG_DATA);
    }

    return 1;
}

int ata_read28(uint32_t lba, uint8_t count, void *buffer) {
    if (!buffer || count == 0) {
        return 0;
    }

    if (!ata_wait_not_busy()) {
        return 0;
    }

    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    io_wait();
    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT, count);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, (uint8_t)(lba & 0xFF));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    uint16_t *out = (uint16_t *)buffer;
    for (uint8_t s = 0; s < count; ++s) {
        if (!ata_wait_drq()) {
            return 0;
        }
        for (uint32_t i = 0; i < 256; ++i) {
            out[i] = inw(ATA_PRIMARY_IO + ATA_REG_DATA);
        }
        out += 256;
    }

    return 1;
}
