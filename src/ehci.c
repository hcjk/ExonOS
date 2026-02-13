#include "ehci.h"
#include "log.h"

static uint32_t mmio_read32(volatile uint8_t *base, uint32_t off) {
    return *(volatile uint32_t *)(base + off);
}

static void mmio_write32(volatile uint8_t *base, uint32_t off, uint32_t value) {
    *(volatile uint32_t *)(base + off) = value;
}

static void spin_delay(uint32_t loops) {
    for (volatile uint32_t i = 0; i < loops; ++i) {
    }
}

int ehci_init(struct ehci_controller *out, uint32_t bar0) {
    if (!out) {
        return 0;
    }
    uint32_t base_addr = bar0 & 0xFFFFFFF0u;
    if (base_addr == 0) {
        return 0;
    }

    out->base = (volatile uint8_t *)(uintptr_t)base_addr;
    out->cap_length = mmio_read32(out->base, 0x00) & 0xFFu;
    out->hcs_params = mmio_read32(out->base, 0x04);
    out->hcc_params = mmio_read32(out->base, 0x08);
    out->op_base = out->base + out->cap_length;

    if (out->cap_length < 0x10u || out->cap_length > 0x40u) {
        return 0;
    }

    log_puts("EHCI caplen=");
    log_dec32(out->cap_length);
    log_puts(" ports=");
    log_dec32((out->hcs_params >> 0) & 0x0F);
    log_puts("\n");

    uint32_t usbcmd = mmio_read32(out->op_base, 0x00);
    usbcmd &= ~1u;
    mmio_write32(out->op_base, 0x00, usbcmd);
    spin_delay(100000);
    mmio_write32(out->op_base, 0x04, 0x3F);
    mmio_write32(out->op_base, 0x08, 0);
    mmio_write32(out->op_base, 0x00, usbcmd | (1u << 1));
    spin_delay(100000);
    mmio_write32(out->op_base, 0x00, usbcmd & ~(1u << 1));
    spin_delay(100000);

    mmio_write32(out->op_base, 0x40, 1u);
    uint32_t ports = (out->hcs_params >> 0) & 0x0F;
    for (uint32_t i = 0; i < ports; ++i) {
        uint32_t portsc = mmio_read32(out->op_base, 0x44 + i * 4);
        if (portsc & 1u) {
            log_puts("EHCI port connect: ");
            log_dec32(i + 1);
            log_puts("\n");
            mmio_write32(out->op_base, 0x44 + i * 4, portsc | (1u << 8));
            spin_delay(200000);
            portsc = mmio_read32(out->op_base, 0x44 + i * 4);
            mmio_write32(out->op_base, 0x44 + i * 4, portsc & ~(1u << 8));
        }
    }
    return 1;
}
