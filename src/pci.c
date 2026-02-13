#include "pci.h"
#include "portio.h"

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

static uint32_t pci_address(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    return 0x80000000u | ((uint32_t)bus << 16) | ((uint32_t)dev << 11) |
           ((uint32_t)func << 8) | (offset & 0xFCu);
}

uint32_t pci_read_config32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    outl(PCI_CONFIG_ADDR, pci_address(bus, dev, func, offset));
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_read_config16(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    uint32_t value = pci_read_config32(bus, dev, func, offset);
    return (uint16_t)(value >> ((offset & 2u) * 8u));
}

uint8_t pci_read_config8(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    uint32_t value = pci_read_config32(bus, dev, func, offset);
    return (uint8_t)(value >> ((offset & 3u) * 8u));
}

void pci_scan(pci_device_cb cb, void *ctx) {
    if (!cb) {
        return;
    }
    for (uint8_t bus = 0; bus < 256; ++bus) {
        for (uint8_t dev = 0; dev < 32; ++dev) {
            for (uint8_t func = 0; func < 8; ++func) {
                uint16_t vendor = pci_read_config16(bus, dev, func, 0x00);
                if (vendor == 0xFFFF) {
                    if (func == 0) {
                        break;
                    }
                    continue;
                }
                uint8_t class_code = pci_read_config8(bus, dev, func, 0x0B);
                uint8_t subclass = pci_read_config8(bus, dev, func, 0x0A);
                uint8_t prog_if = pci_read_config8(bus, dev, func, 0x09);
                cb(bus, dev, func, class_code, subclass, prog_if, ctx);
            }
        }
    }
}

void pci_scan_bus0(pci_device_cb cb, void *ctx) {
    if (!cb) {
        return;
    }
    uint8_t bus = 0;
    for (uint8_t dev = 0; dev < 32; ++dev) {
        for (uint8_t func = 0; func < 8; ++func) {
            uint16_t vendor = pci_read_config16(bus, dev, func, 0x00);
            if (vendor == 0xFFFF) {
                if (func == 0) {
                    break;
                }
                continue;
            }
            uint8_t class_code = pci_read_config8(bus, dev, func, 0x0B);
            uint8_t subclass = pci_read_config8(bus, dev, func, 0x0A);
            uint8_t prog_if = pci_read_config8(bus, dev, func, 0x09);
            cb(bus, dev, func, class_code, subclass, prog_if, ctx);
        }
    }
}
