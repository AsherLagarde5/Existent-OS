CC ?= x86_64-linux-gnu-gcc
LD ?= ld
OBJCOPY ?= objcopy
QEMU ?= qemu-system-x86_64

CFLAGS = -ffreestanding -O2 -Wall -Wextra -m64 -mno-red-zone \
         -Iboot -Ikernel -Ikernel/drivers -Ikernel/drivers/system \
         -I/usr/include/efi -I/usr/include/efi/x86_64

LDFLAGS = -nostdlib -T efi.ld

# === Kernel sources (recursive) ===
KERNEL_C_SRCS := $(shell find kernel -name '*.c')
KERNEL_ASM_SRCS := $(shell find kernel -name '*.S')

KERNEL_OBJS := $(KERNEL_C_SRCS:.c=.o)
KERNEL_ASM_OBJS := $(KERNEL_ASM_SRCS:.S=.o)
KERNEL_OBJS_ALL := $(KERNEL_OBJS) $(KERNEL_ASM_OBJS)

# === Bootloader sources ===
EFI_SRC = boot/efi_main.c
EFI_OBJ = boot/efi_main.o
EFI_ELF = EFI/BOOT/BOOTX64.elf
EFI_BIN = EFI/BOOT/BOOTX64.EFI

all: $(EFI_BIN)

# === Build UEFI loader ===
$(EFI_BIN): $(EFI_OBJ)
    mkdir -p EFI/BOOT
    $(LD) $(LDFLAGS) -o $(EFI_ELF) $(EFI_OBJ)
    $(OBJCOPY) \
      -j .text -j .data -j .rodata -j .bss \
      --subsystem=10 \
      -I elf64-x86-64 -O pei-x86-64 \
      $(EFI_ELF) $(EFI_BIN)
    rm -f $(EFI_ELF)

$(EFI_OBJ): $(EFI_SRC)
    $(CC) $(CFLAGS) -c $< -o $@

# === Kernel build ===
%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
    $(CC) $(CFLAGS) -c -x assembler-with-cpp $< -o $@

# === Run in QEMU ===
run: $(EFI_BIN)
    $(QEMU) -bios /usr/share/ovmf/OVMF_CODE.fd \
            -drive file=fat:rw:EFI,format=raw,if=virtio

clean:
    rm -rf EFI $(KERNEL_OBJS_ALL) $(EFI_OBJ)
