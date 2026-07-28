#include "../boot/efi.h"
#include "acpi.h"
#include <stdint.h>

static const EFI_GUID Acpi20TableGuid = { 0x8868e871, 0xe4f1, 0x11d3, {0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81} };
static const EFI_GUID AcpiTableGuid = { 0xeb9d2d30, 0x2d88, 0x11d3, {0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d} };
static void *rsdp_address = 0;

static int guid_equal(const EFI_GUID *a, const EFI_GUID *b)
{
    if (a->Data1 != b->Data1 || a->Data2 != b->Data2 || a->Data3 != b->Data3)
        return 0;
    for (int i = 0; i < 8; i++) {
        if (a->Data4[i] != b->Data4[i])
            return 0;
    }
    return 1;
}

void acpi_init(void)
{
    if (!gST)
        return;

    for (size_t i = 0; i < gST->NumberOfTableEntries; i++) {
        EFI_CONFIGURATION_TABLE *table = &gST->ConfigurationTable[i];
        if (guid_equal(&table->VendorGuid, &Acpi20TableGuid) || guid_equal(&table->VendorGuid, &AcpiTableGuid)) {
            rsdp_address = table->VendorTable;
            return;
        }
    }
}

int acpi_is_available(void)
{
    return rsdp_address != NULL;
}
