#include "mbr.h"
#include "ata.h"

int mbr_read(struct mbr *out) {
    if (!out) {
        return 0;
    }
    if (!ata_read28(0, 1, out)) {
        return 0;
    }
    if (out->signature != 0xAA55) {
        return 0;
    }
    return 1;
}
