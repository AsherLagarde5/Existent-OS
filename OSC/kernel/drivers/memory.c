#include "../boot/efi.h"
#include "memory.h"
#include <stdint.h>

static uint64_t total_memory_bytes = 0;

void memory_init(void)
{
    if (!gBS || !gST)
        return;

    static uint8_t memory_map_buffer[0x10000];
    size_t memory_map_size = sizeof(memory_map_buffer);
    size_t map_key = 0;
    size_t descriptor_size = 0;
    uint32_t descriptor_version = 0;

    EFI_GET_MEMORY_MAP get_memory_map = (EFI_GET_MEMORY_MAP)gBS->GetMemoryMap;
    EFI_EXIT_BOOT_SERVICES exit_boot_services = (EFI_EXIT_BOOT_SERVICES)gBS->ExitBootServices;

    EFI_STATUS status = get_memory_map(&memory_map_size,
                                      (EFI_MEMORY_DESCRIPTOR *)memory_map_buffer,
                                      &map_key,
                                      &descriptor_size,
                                      &descriptor_version);
    if (status != EFI_SUCCESS) {
        total_memory_bytes = 0;
        return;
    }

    total_memory_bytes = 0;
    size_t entry_count = memory_map_size / descriptor_size;
    uint8_t *ptr = memory_map_buffer;

    for (size_t i = 0; i < entry_count; i++) {
        EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)ptr;
        if (desc->Type == EfiConventionalMemory || desc->Type == EfiLoaderData || desc->Type == EfiBootServicesData) {
            total_memory_bytes += desc->NumberOfPages * 4096ULL;
        }
        ptr += descriptor_size;
    }

    if (total_memory_bytes == 0)
        total_memory_bytes = 512ULL * 1024ULL * 1024ULL;

    status = exit_boot_services(gImageHandle, map_key);
    if (status != EFI_SUCCESS) {
        /* Failed to exit boot services, but continue anyway */
        total_memory_bytes = 512ULL * 1024ULL * 1024ULL;
    }
}

uint64_t memory_total_bytes(void)
{
    return total_memory_bytes;
}
