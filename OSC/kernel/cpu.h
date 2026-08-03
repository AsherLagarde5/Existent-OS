#ifndef CPU_H
#define CPU_H

#include <stdint.h>

typedef enum {
    CPU_VENDOR_UNKNOWN = 0,
    CPU_VENDOR_INTEL = 1,
    CPU_VENDOR_AMD = 2
} cpu_vendor_t;

typedef struct {
    cpu_vendor_t vendor;
    uint32_t family;
    uint32_t model;
    uint32_t stepping;
    uint32_t features_ecx;
    uint32_t features_edx;
} cpu_info_t;

void cpu_init(void);
int cpu_get_info(cpu_info_t *info);
int cpu_is_intel(void);

#endif
