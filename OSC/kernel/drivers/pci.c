#include "pci.h"

void pci_init(void)
{
    /* Under UEFI, PCI devices are managed by firmware until ExitBootServices. */
}

int pci_find_device(uint8_t class_code, uint8_t subclass, uint8_t prog_if, pci_device_t *out)
{
    (void)class_code;
    (void)subclass;
    (void)prog_if;
    (void)out;
    return 0;
}
