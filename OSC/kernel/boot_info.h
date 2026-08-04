#ifndef KERNEL_BOOT_INFO_H
#define KERNEL_BOOT_INFO_H
#include <stdint.h>
typedef struct { uint64_t memory_bytes; void *rsdp; } kernel_boot_info_t;
#endif
