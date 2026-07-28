#include "pic.h"

void pic_init(void)
{
    /* No PIC initialization in pure UEFI mode; interrupt controllers are
       managed by firmware (or by APIC/MSI later in the kernel). */
}

void pic_send_eoi(uint8_t irq)
{
    (void)irq; /* No-op under UEFI */
}

void pic_disable(void)
{
    /* No-op under UEFI */
}