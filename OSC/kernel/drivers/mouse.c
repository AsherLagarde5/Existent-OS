#include "mouse.h"

static uint8_t mouse_enabled = 0;

void mouse_init(void)
{
    /* Mouse initialization stub - not yet implemented */
    mouse_enabled = 1;
}

void mouse_handle_interrupt(uint8_t status)
{
    if (!mouse_enabled)
        return;
    /* Mouse interrupt handling stub - not yet implemented */
}
