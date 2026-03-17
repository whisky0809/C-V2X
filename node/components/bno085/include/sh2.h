#ifndef SH2_H
#define SH2_H

#include "sh2_hal.h"

// Stub representations of CEVA sh2 functions. 
// Replace with the real CEVA sh2.h header later.

typedef void (*sh2_EventCallback_t)(void *cookie);

int sh2_open(sh2_hal_t *pHal, sh2_EventCallback_t event_cb, void *event_cookie);
void sh2_close(void);
void sh2_service(void);

#endif // SH2_H
