CC      ?= x86_64-linux-gnu-gcc
LD      ?= ld.lld
OBJCOPY ?= objcopy
QEMU    ?= qemu-system-x86_64
OVMF_CODE ?= $(firstword $(wildcard /usr/share/OVMF/OVMF_CODE.fd /usr/share/OVMF/OVMF_CODE_4M.fd))

CFLAGS  = -ffreestanding -O2 -Wall -Wextra -m64 -mno-red-zone \
          -fno-stack-protector -fpic -fshort-wchar \
          -Iboot -Ikernel -Ikernel/drivers -Ikernel/drivers/system \
          -I/usr/include/efi -I/usr/include/efi/x86_64

# === Sources ===
KERNEL_C_SRCS   := $(shell find kernel -name '*.c' 2>/dev/null)
KERNEL_ASM_SRCS := $(shell find kernel -name '*.S' 2>/dev/null)
KERNEL_OBJS     := $(KERNEL_C_SRCS:.c=.o) $(KERNEL_ASM_SRCS:.S=.o)
KERNEL_RUNTIME_OBJS := $(filter-out kernel/drivers/eosfs.o kernel/drivers/accounts.o,$(KERNEL_OBJS))
KERNEL_ELF := kernel/kernel.elf
KERNEL_BIN := kernel.bin

EFI_SRC  = boot/efi_main.c
EFI_OBJ  = boot/efi_main.o
EFI_ELF  = EFI/BOOT/BOOTX64.elf
EFI_BIN  = EFI/BOOT/BOOTX64.EFI
LOADER_OBJS = $(EFI_OBJ) kernel/drivers/eosfs.o kernel/drivers/accounts.o

ISO_DIR  = iso
ISO_IMG  = FisixOS.iso
ESP_IMG  = esp.img

.PHONY: all clean iso run run-iso

all: $(EFI_BIN)

kernel: $(KERNEL_BIN)

$(KERNEL_BIN): $(KERNEL_RUNTIME_OBJS) kernel/kernel.ld
	$(CC) -nostdlib -no-pie -Wl,-T,kernel/kernel.ld -o $(KERNEL_ELF) $(KERNEL_RUNTIME_OBJS)
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_BIN)
	rm -f $(KERNEL_ELF)

# === Build the EFI application ===
$(EFI_BIN): $(LOADER_OBJS)
	mkdir -p EFI/BOOT
	$(CC) -nostdlib -Wl,-T,efi.ld -o $(EFI_ELF) $(LOADER_OBJS) \
		-Wl,-shared -Wl,-Bsymbolic -Wl,-znocombreloc
	$(OBJCOPY) -I elf64-x86-64 -O efi-app-x86_64 --image-base 0x100000 $(EFI_ELF) $(EFI_BIN) || \
	$(OBJCOPY) --target=efi-app-x86_64 --image-base 0x100000 $(EFI_ELF) $(EFI_BIN)
	rm -f $(EFI_ELF)

$(EFI_OBJ): $(EFI_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c -x assembler-with-cpp $< -o $@

# === Create a proper ESP image ===
$(ESP_IMG): $(EFI_BIN) $(KERNEL_BIN)
	dd if=/dev/zero of=$(ESP_IMG) bs=1M count=32 status=none
	mkfs.fat -F 32 $(ESP_IMG)
	mmd -i $(ESP_IMG) ::/EFI
	mmd -i $(ESP_IMG) ::/EFI/BOOT
	mcopy -i $(ESP_IMG) $(EFI_BIN) ::/EFI/BOOT/BOOTX64.EFI
	mcopy -i $(ESP_IMG) $(KERNEL_BIN) ::/kernel.bin

# === Create a pure UEFI-bootable ISO ===
iso: $(ESP_IMG)
	mkdir -p $(ISO_DIR)/EFI/BOOT
	cp $(EFI_BIN) $(ISO_DIR)/EFI/BOOT/BOOTX64.EFI
	cp $(KERNEL_BIN) $(ISO_DIR)/kernel.bin
	cp $(ESP_IMG) $(ISO_DIR)/esp.img
	xorriso -as mkisofs \
		-R -f \
		-eltorito-alt-boot \
		-e esp.img \
		-no-emul-boot \
		-isohybrid-gpt-basdat \
		-o $(ISO_IMG) \
		$(ISO_DIR)

# === Run just the EFI binary (fastest for testing) ===
run: $(EFI_BIN)
	$(QEMU) -m 512 \
		-drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
		-drive if=pflash,format=raw,file=OVMF_VARS.fd \
		-drive format=raw,file=fat:rw:EFI \
		-net none

# === Run the full ISO ===
run-iso: iso
	$(QEMU) -m 512 \
		-drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
		-drive if=pflash,format=raw,file=OVMF_VARS.fd \
		-cdrom $(ISO_IMG) \
		-boot d \
		-net none

clean:
	rm -rf EFI $(ISO_DIR) $(ISO_IMG) $(ESP_IMG) \
	       $(KERNEL_OBJS) $(EFI_OBJ) $(EFI_ELF) $(KERNEL_ELF) $(KERNEL_BIN)
