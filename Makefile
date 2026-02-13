TARGET := myos
BUILD_DIR := build
ISO_DIR := isodir

CROSS ?= i686-elf-
NASM := nasm
CC := $(CROSS)gcc
LD := $(CROSS)ld

CFLAGS := -m32 -fno-pie -fno-stack-protector -fno-builtin -nostdlib -nodefaultlibs -ffreestanding -O2 -Wall -Wextra -Iinclude
LDFLAGS := -m elf_i386

OBJS := \
	$(BUILD_DIR)/boot.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/memory.o \
	$(BUILD_DIR)/mb2.o \
	$(BUILD_DIR)/framebuffer.o \
	$(BUILD_DIR)/font8x8.o \
	$(BUILD_DIR)/input.o \
	$(BUILD_DIR)/ata.o \
	$(BUILD_DIR)/mbr.o \
	$(BUILD_DIR)/fat.o \
	$(BUILD_DIR)/vfs.o \
	$(BUILD_DIR)/log.o \
	$(BUILD_DIR)/panic.o \
	$(BUILD_DIR)/scheduler.o \
	$(BUILD_DIR)/pci.o \
	$(BUILD_DIR)/usb.o \
	$(BUILD_DIR)/ehci.o \
	$(BUILD_DIR)/ehci_transfer.o \
	$(BUILD_DIR)/usb_hid.o \
	$(BUILD_DIR)/MagicUI.o \
	$(BUILD_DIR)/ui_apps.o \
	$(BUILD_DIR)/app_apps.o \
	$(BUILD_DIR)/app_settings.o \
	$(BUILD_DIR)/app_files.o \
	$(BUILD_DIR)/app_usb.o \
	$(BUILD_DIR)/app_test.o \
	$(BUILD_DIR)/ui.o

.PHONY: all clean build-iso run

all: $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: src/boot.asm | $(BUILD_DIR)
	$(NASM) -f elf32 $< -o $@

$(BUILD_DIR)/kernel.o: src/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memory.o: src/memory.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/mb2.o: src/mb2.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/framebuffer.o: src/framebuffer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/font8x8.o: src/font8x8.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/input.o: src/input.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ata.o: src/ata.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/mbr.o: src/mbr.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/fat.o: src/fat.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/vfs.o: src/vfs.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/pci.o: src/pci.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/usb.o: src/usb.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ehci.o: src/ehci.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ehci_transfer.o: src/ehci_transfer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/usb_hid.o: src/usb_hid.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/MagicUI.o: src/MagicUI.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ui_apps.o: src/ui_apps.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/app_apps.o: src/app_apps.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/app_settings.o: src/app_settings.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/app_files.o: src/app_files.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/app_usb.o: src/app_usb.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/app_test.o: src/app_test.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/log.o: src/log.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/panic.o: src/panic.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/scheduler.o: src/scheduler.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ui.o: src/ui.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).bin: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $(OBJS)

build-iso: all
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BUILD_DIR)/$(TARGET).bin $(ISO_DIR)/boot/$(TARGET).bin
	printf 'set timeout=0\nset default=0\nset gfxmode=1024x768x32\nset gfxpayload=keep\n\nmenuentry "MyOS" {\n    multiboot2 /boot/$(TARGET).bin\n    boot\n}\n' > $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(TARGET).iso $(ISO_DIR)

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR) $(TARGET).iso
