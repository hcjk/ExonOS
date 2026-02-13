#pragma once

#include <stdint.h>

typedef void (*pci_device_cb)(uint8_t bus, uint8_t dev, uint8_t func,
                              uint8_t class_code, uint8_t subclass,
                              uint8_t prog_if, void *ctx);

uint32_t pci_read_config32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);
uint16_t pci_read_config16(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);
uint8_t pci_read_config8(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);

void pci_scan(pci_device_cb cb, void *ctx);
void pci_scan_bus0(pci_device_cb cb, void *ctx);
