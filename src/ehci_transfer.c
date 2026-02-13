#include "ehci.h"
#include "memory.h"

#define QTD_TOKEN_ACTIVE (1u << 7)
#define QTD_TOKEN_PID_SETUP (2u << 8)
#define QTD_TOKEN_PID_IN (1u << 8)
#define QTD_TOKEN_PID_OUT (0u << 8)

struct ehci_qtd {
    uint32_t next;
    uint32_t alt_next;
    uint32_t token;
    uint32_t buf[5];
} __attribute__((packed, aligned(32)));

struct ehci_qh {
    uint32_t horiz_link;
    uint32_t ep_char;
    uint32_t ep_cap;
    uint32_t curr_qtd;
    struct ehci_qtd overlay;
} __attribute__((packed, aligned(32)));

static void mmio_write32(volatile uint8_t *base, uint32_t off, uint32_t value) {
    *(volatile uint32_t *)(base + off) = value;
}

static uint32_t mmio_read32(volatile uint8_t *base, uint32_t off) {
    return *(volatile uint32_t *)(base + off);
}

static struct ehci_qtd *alloc_qtd(void) {
    struct ehci_qtd *qtd = (struct ehci_qtd *)kmalloc(sizeof(struct ehci_qtd), 32);
    if (!qtd) {
        return 0;
    }
    for (uint32_t i = 0; i < sizeof(*qtd) / 4; ++i) {
        ((uint32_t *)qtd)[i] = 0;
    }
    qtd->next = 1;
    qtd->alt_next = 1;
    return qtd;
}

static struct ehci_qh *alloc_qh(void) {
    struct ehci_qh *qh = (struct ehci_qh *)kmalloc(sizeof(struct ehci_qh), 32);
    if (!qh) {
        return 0;
    }
    for (uint32_t i = 0; i < sizeof(*qh) / 4; ++i) {
        ((uint32_t *)qh)[i] = 0;
    }
    qh->horiz_link = ((uint32_t)(uintptr_t)qh) | 0x2u;
    qh->overlay.next = 1;
    qh->overlay.alt_next = 1;
    return qh;
}

static void qtd_set_buffer(struct ehci_qtd *qtd, void *buf, uint32_t len) {
    uint32_t addr = (uint32_t)(uintptr_t)buf;
    for (int i = 0; i < 5; ++i) {
        qtd->buf[i] = addr;
        uint32_t next = (addr & 0xFFFFF000u) + 0x1000u;
        addr = next;
    }
    qtd->token |= (len << 16);
}

static int wait_qtd_complete(struct ehci_qtd *qtd) {
    for (uint32_t i = 0; i < 1000000; ++i) {
        if ((qtd->token & QTD_TOKEN_ACTIVE) == 0) {
            return 1;
        }
    }
    return 0;
}

int ehci_control_transfer(struct ehci_controller *ctrl,
                          uint8_t dev_addr,
                          uint8_t ep,
                          uint16_t max_packet,
                          const void *setup,
                          void *data,
                          uint32_t length,
                          int in_dir) {
    if (!ctrl || !ctrl->op_base || !setup) {
        return 0;
    }

    struct ehci_qh *qh = alloc_qh();
    struct ehci_qtd *qtd_setup = alloc_qtd();
    struct ehci_qtd *qtd_data = length ? alloc_qtd() : 0;
    struct ehci_qtd *qtd_status = alloc_qtd();
    if (!qh || !qtd_setup || !qtd_status || (length && !qtd_data)) {
        return 0;
    }

    qtd_setup->next = length ? (uint32_t)(uintptr_t)qtd_data : (uint32_t)(uintptr_t)qtd_status;
    qtd_setup->token = QTD_TOKEN_ACTIVE | QTD_TOKEN_PID_SETUP | (8u << 16);
    qtd_set_buffer(qtd_setup, (void *)setup, 8);

    if (length) {
        qtd_data->next = (uint32_t)(uintptr_t)qtd_status;
        qtd_data->token = QTD_TOKEN_ACTIVE | (in_dir ? QTD_TOKEN_PID_IN : QTD_TOKEN_PID_OUT) | (length << 16);
        qtd_set_buffer(qtd_data, data, length);
    }

    qtd_status->next = 1;
    qtd_status->token = QTD_TOKEN_ACTIVE | (in_dir ? QTD_TOKEN_PID_OUT : QTD_TOKEN_PID_IN) | (0u << 16);

    qh->ep_char = (max_packet & 0x7FFu) << 16;
    qh->ep_char |= (ep & 0x0Fu) << 8;
    qh->ep_char |= (dev_addr & 0x7Fu);
    qh->ep_char |= (1u << 14);

    qh->curr_qtd = 0;
    qh->overlay.next = (uint32_t)(uintptr_t)qtd_setup;
    qh->overlay.alt_next = 1;

    mmio_write32(ctrl->op_base, 0x18, (uint32_t)(uintptr_t)qh);
    uint32_t cmd = mmio_read32(ctrl->op_base, 0x00);
    cmd |= (1u << 5);
    mmio_write32(ctrl->op_base, 0x00, cmd);

    if (!wait_qtd_complete(qtd_status)) {
        return 0;
    }

    cmd = mmio_read32(ctrl->op_base, 0x00);
    cmd &= ~(1u << 5);
    mmio_write32(ctrl->op_base, 0x00, cmd);

    return 1;
}
