#include "../boot/efi.h"
#include "memory.h"
#include "acpi.h"
#include "idt.h"

void kernel_main(void)
{
    acpi_init();
    memory_init();

    /* Under UEFI, direct hardware access is not available until after ExitBootServices.
       Do not perform legacy PCI, storage, USB, battery, or PIC initialization here. */

    idt_init();

    while (1)
    {
        __asm__ volatile ("hlt");
    }
}
