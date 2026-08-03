#include "../boot/efi.h"
#include "memory.h"
#include "acpi.h"
#include "idt.h"
#include "cpu.h"

void kernel_main(EfiMemoryMap *final_map)
{
    /* At this point, ExitBootServices() MUST already be called.
       Hardware access, ACPI parsing, paging, APIC, etc. are now legal. */

    // Initialize paging + physical memory manager using final UEFI map
    memory_init(final_map);

    // Parse ACPI tables (RSDP, XSDT, MADT, FADT, etc.)
    acpi_init();

    // Set up exception handlers + interrupt gates
    idt_init();

    // Initialize CPU/APIC now that UEFI is gone
    cpu_init();      // LAPIC, x2APIC, MSRs, SMP prep (if you want)

    // Enable interrupts only after IDT + APIC are ready
    __asm__ volatile ("sti");

    // Kernel idle loop
    while (1)
        __asm__ volatile ("hlt");
}
