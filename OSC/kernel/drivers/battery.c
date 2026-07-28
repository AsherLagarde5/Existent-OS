#include "battery.h"

static battery_status_t battery_state = {0, 0, 0};

void battery_init(void)
{
    /* Battery state under UEFI is typically exposed through firmware runtime services
       or platform-specific ACPI methods. Avoid direct EC/SMBus access from the kernel. */
    battery_state.present = 0;
    battery_state.percent = 0;
    battery_state.charging = 0;
}

battery_status_t battery_get_status(void)
{
    return battery_state;
}
