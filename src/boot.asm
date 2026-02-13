; Multiboot-compliant entry stub for MyOS
; Assemble: nasm -f elf32 src/boot.asm -o build/boot.o

BITS 32

%define MULTIBOOT2_MAGIC 0xE85250D6
%define MULTIBOOT2_ARCH 0x00000000

section .multiboot
align 8
mb2_header_start:
    dd MULTIBOOT2_MAGIC
    dd MULTIBOOT2_ARCH
    dd mb2_header_end - mb2_header_start
    dd -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCH + (mb2_header_end - mb2_header_start))

    ; Framebuffer request tag
    dw 5
    dw 0
    dd 24
    dd 1024
    dd 768
    dd 32
    dd 0

    ; End tag
    dw 0
    dw 0
    dd 8
mb2_header_end:

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top
    push ebx
    push eax
    call kernel_main
    add esp, 8

.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
