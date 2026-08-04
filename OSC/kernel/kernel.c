#include "../boot/efi.h"
#include "memory.h"
#include "acpi.h"
#include "idt.h"
#include "cpu.h"
#include "pic.h"
#include "boot_info.h"

void kernel_startup(const kernel_boot_info_t *boot_info)
{
    __asm__ volatile ("cli");

    if (boot_info) {
        memory_set_total_bytes(boot_info->memory_bytes);
        acpi_set_rsdp(boot_info->rsdp);
    }

    /* At this point, ExitBootServices() MUST already be called.
       Hardware access, ACPI parsing, paging, APIC, etc. are now legal. */

    /* The final UEFI memory map was captured by the loader. */
    memory_init();

    // Parse ACPI tables (RSDP, XSDT, MADT, FADT, etc.)
    acpi_init();

    // Set up exception handlers + interrupt gates
    idt_init();

    cpu_init();
    pic_disable();

    /* No IRQ controller is configured yet.  Keep interrupts masked until a
       timer and proper IRQ handlers have been installed. */
    while (1)
        __asm__ volatile ("hlt");
}
