#include "vfs.h"
#include "fat.h"
#include "log.h"

static struct fat_fs fat;
static int fat_mounted;

void vfs_init(void) {
    fat_mounted = 0;
}

int vfs_mount_fat(uint32_t part_lba) {
    fat_mounted = fat_mount(&fat, part_lba);
    return fat_mounted;
}

static void log_entry(const char *name, uint32_t size, uint8_t attr, void *ctx) {
    (void)ctx;
    log_puts("  ");
    log_puts(name);
    log_puts(" size=");
    log_dec32(size);
    log_puts(" attr=");
    log_hex32(attr);
    log_puts("\n");
}

int vfs_list_root(void) {
    if (!fat_mounted) {
        return 0;
    }
    return fat_list_root(&fat, log_entry, 0);
}
