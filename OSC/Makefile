CC ?= x86_64-linux-gnu-gcc
LD ?= ld
OBJCOPY ?= objcopy
QEMU ?= qemu-system-x86_64

CFLAGS = -ffreestanding -O2 -Wall -Wextra -m64 -mno-red-zone \
		 -Iboot -Ikernel -Ikernel/drivers -Ikernel/drivers/system \
		 -I/usr/include/efi -I/usr/include/efi/x86_64

LDFLAGS = -nostdlib -T efi.ld

KERNEL_C_SRCS = kernel/kernel.c kernel/drivers/pci.c kernel/drivers/storage.c \
				kernel/drivers/usb.c kernel/drivers/acpi.c kernel/drivers/battery.c \
				kernel/drivers/memory.c kernel/drivers/system/pic.c \
				kernel/drivers/system/idt.c kernel/drivers/system/irq_handlers.c

KERNEL_ASM_SRCS = kernel/entry.S kernel/drivers/system/interrupts.S

KERNEL_OBJS = $(KERNEL_C_SRCS:.c=.o)
KERNEL_ASM_OBJS = $(KERNEL_ASM_SRCS:.S=.o)
KERNEL_OBJS_ALL = $(KERNEL_OBJS) $(KERNEL_ASM_OBJS)

all: EFI/BOOT/BOOTX64.EFI

EFI/BOOT/BOOTX64.EFI: boot/efi_main.c $(KERNEL_OBJS_ALL)
	mkdir -p EFI/BOOT
	$(CC) $(CFLAGS) -c boot/efi_main.c -o boot/efi_main.o
	$(LD) $(LDFLAGS) -o EFI/BOOT/BOOTX64.elf boot/efi_main.o $(KERNEL_OBJS_ALL)
	$(OBJCOPY) \
	  -j .text -j .data -j .rodata -j .bss \
	  --subsystem=10 \
	  -I elf64-x86-64 -O pei-x86-64 \
	  EFI/BOOT/BOOTX64.elf $@
	rm -f EFI/BOOT/BOOTX64.elf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c -x assembler-with-cpp $< -o $@

run: EFI/BOOT/BOOTX64.EFI
	$(QEMU) -bios /usr/share/ovmf/OVMF_CODE.fd \
			-drive file=fat:rw:EFI,format=raw,if=virtio

clean:
	rm -rf EFI $(KERNEL_OBJS_ALL) boot/efi_main.o
