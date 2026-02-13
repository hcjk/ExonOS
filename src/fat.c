#include "fat.h"
#include "ata.h"

#define FAT_TYPE_16 16
#define FAT_TYPE_32 32

struct bpb_common {
    uint8_t jmp_boot[3];
    uint8_t oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_entries;
    uint16_t total_sectors16;
    uint8_t media;
    uint16_t sectors_per_fat16;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors32;
} __attribute__((packed));

struct bpb_fat32 {
    uint32_t sectors_per_fat32;
    uint16_t flags;
    uint16_t version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot;
    uint8_t reserved[12];
} __attribute__((packed));

struct dir_entry {
    uint8_t name[11];
    uint8_t attr;
    uint8_t ntres;
    uint8_t crt_time_tenths;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t last_access_date;
    uint16_t first_cluster_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t first_cluster_lo;
    uint32_t file_size;
} __attribute__((packed));

static int read_sector(uint32_t lba, void *buf) {
    return ata_read28(lba, 1, buf);
}

static void name_to_str(const uint8_t name[11], char out[13]) {
    int oi = 0;
    for (int i = 0; i < 8; ++i) {
        if (name[i] == ' ') {
            break;
        }
        out[oi++] = (char)name[i];
    }
    if (name[8] != ' ') {
        out[oi++] = '.';
        for (int i = 8; i < 11; ++i) {
            if (name[i] == ' ') {
                break;
            }
            out[oi++] = (char)name[i];
        }
    }
    out[oi] = '\0';
}

static uint32_t get_total_sectors(const struct bpb_common *bpb) {
    if (bpb->total_sectors16 != 0) {
        return bpb->total_sectors16;
    }
    return bpb->total_sectors32;
}

static uint32_t get_sectors_per_fat(const struct bpb_common *bpb, const uint8_t *sector) {
    if (bpb->sectors_per_fat16 != 0) {
        return bpb->sectors_per_fat16;
    }
    const struct bpb_fat32 *fat32 = (const struct bpb_fat32 *)(sector + 36);
    return fat32->sectors_per_fat32;
}

static uint8_t detect_fat_type(const struct bpb_common *bpb, uint32_t total_sectors, uint32_t sectors_per_fat) {
    uint32_t root_dir_sectors = ((bpb->root_entries * 32u) + (bpb->bytes_per_sector - 1u)) / bpb->bytes_per_sector;
    uint32_t data_sectors = total_sectors - (bpb->reserved_sectors + (bpb->fat_count * sectors_per_fat) + root_dir_sectors);
    uint32_t cluster_count = data_sectors / bpb->sectors_per_cluster;
    if (cluster_count < 4085) {
        return 12;
    }
    if (cluster_count < 65525) {
        return FAT_TYPE_16;
    }
    return FAT_TYPE_32;
}

static uint32_t fat_entry_offset(const struct fat_fs *fs, uint32_t cluster) {
    if (fs->fat_type == FAT_TYPE_16) {
        return cluster * 2u;
    }
    return cluster * 4u;
}

static uint32_t fat_get_next_cluster(const struct fat_fs *fs, uint32_t cluster, uint8_t *sector) {
    uint32_t offset = fat_entry_offset(fs, cluster);
    uint32_t lba = fs->fat_lba + (offset / fs->bytes_per_sector);
    uint32_t entry_off = offset % fs->bytes_per_sector;
    if (!read_sector(lba, sector)) {
        return 0xFFFFFFFFu;
    }
    if (fs->fat_type == FAT_TYPE_16) {
        uint16_t val = *(uint16_t *)(sector + entry_off);
        return val;
    }
    uint32_t val = *(uint32_t *)(sector + entry_off) & 0x0FFFFFFFu;
    return val;
}

int fat_mount(struct fat_fs *fs, uint32_t part_lba) {
    if (!fs) {
        return 0;
    }
    uint8_t sector[512];
    if (!read_sector(part_lba, sector)) {
        return 0;
    }

    const struct bpb_common *bpb = (const struct bpb_common *)sector;
    if (bpb->bytes_per_sector == 0 || bpb->sectors_per_cluster == 0) {
        return 0;
    }

    uint32_t total_sectors = get_total_sectors(bpb);
    uint32_t sectors_per_fat = get_sectors_per_fat(bpb, sector);
    uint32_t root_dir_sectors = ((bpb->root_entries * 32u) + (bpb->bytes_per_sector - 1u)) / bpb->bytes_per_sector;

    uint8_t fat_type = detect_fat_type(bpb, total_sectors, sectors_per_fat);
    if (fat_type != FAT_TYPE_16 && fat_type != FAT_TYPE_32) {
        return 0;
    }

    fs->part_lba = part_lba;
    fs->bytes_per_sector = bpb->bytes_per_sector;
    fs->sectors_per_cluster = bpb->sectors_per_cluster;
    fs->fat_count = bpb->fat_count;
    fs->sectors_per_fat = sectors_per_fat;
    fs->total_sectors = total_sectors;
    fs->root_dir_sectors = root_dir_sectors;
    fs->fat_type = fat_type;
    fs->fat_lba = part_lba + bpb->reserved_sectors;
    fs->root_dir_lba = fs->fat_lba + (fs->fat_count * fs->sectors_per_fat);
    fs->data_lba = fs->root_dir_lba + root_dir_sectors;
    fs->root_cluster = 0;

    if (fat_type == FAT_TYPE_32) {
        const struct bpb_fat32 *fat32 = (const struct bpb_fat32 *)(sector + 36);
        fs->root_cluster = fat32->root_cluster;
    }

    return 1;
}

int fat_list_root(struct fat_fs *fs, fat_dir_cb cb, void *ctx) {
    if (!fs || !cb) {
        return 0;
    }
    uint8_t sector[512];
    char name[13];

    if (fs->fat_type == FAT_TYPE_16) {
        for (uint32_t i = 0; i < fs->root_dir_sectors; ++i) {
            if (!read_sector(fs->part_lba + fs->root_dir_lba + i, sector)) {
                return 0;
            }
            for (uint32_t off = 0; off < fs->bytes_per_sector; off += sizeof(struct dir_entry)) {
                struct dir_entry *ent = (struct dir_entry *)(sector + off);
                if (ent->name[0] == 0x00) {
                    return 1;
                }
                if (ent->name[0] == 0xE5 || ent->attr == 0x0F) {
                    continue;
                }
                name_to_str(ent->name, name);
                cb(name, ent->file_size, ent->attr, ctx);
            }
        }
        return 1;
    }

    uint32_t cluster = fs->root_cluster;
    uint8_t fat_sector[512];
    while (cluster < 0x0FFFFFF8u) {
        uint32_t first_lba = fs->data_lba + (cluster - 2u) * fs->sectors_per_cluster;
        for (uint32_t s = 0; s < fs->sectors_per_cluster; ++s) {
            if (!read_sector(fs->part_lba + first_lba + s, sector)) {
                return 0;
            }
            for (uint32_t off = 0; off < fs->bytes_per_sector; off += sizeof(struct dir_entry)) {
                struct dir_entry *ent = (struct dir_entry *)(sector + off);
                if (ent->name[0] == 0x00) {
                    return 1;
                }
                if (ent->name[0] == 0xE5 || ent->attr == 0x0F) {
                    continue;
                }
                name_to_str(ent->name, name);
                cb(name, ent->file_size, ent->attr, ctx);
            }
        }
        cluster = fat_get_next_cluster(fs, cluster, fat_sector);
        if (cluster == 0xFFFFFFFFu) {
            return 0;
        }
    }

    return 1;
}
