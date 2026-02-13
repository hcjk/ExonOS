#pragma once

#include <stdint.h>

struct ehci_controller {
    volatile uint8_t *base;
    volatile uint8_t *op_base;
    uint32_t cap_length;
    uint32_t hcs_params;
    uint32_t hcc_params;
};

int ehci_init(struct ehci_controller *out, uint32_t bar0);
int ehci_control_transfer(struct ehci_controller *ctrl,
                          uint8_t dev_addr,
                          uint8_t ep,
                          uint16_t max_packet,
                          const void *setup,
                          void *data,
                          uint32_t length,
                          int in_dir);
