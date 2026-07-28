#ifndef __MOUSE_H__
#define __MOUSE_H__

#include <stdint.h>

void mouse_init(void);
void mouse_handle_interrupt(uint8_t status);

#endif
