#include "sh2.h"
#include <stdio.h>

// Stub implementation of CEVA SH-2 library
// Once you drop the CEVA source folder, you can delete this file and update CMakeLists.txt

int sh2_open(sh2_hal_t *pHal, sh2_EventCallback_t event_cb, void *event_cookie) {
    if (pHal && pHal->open) {
        printf("SH2 Stub: Calling HAL open...\r\n");
        return pHal->open(pHal);
    }
    return -1;
}

void sh2_close(void) {
    printf("SH2 Stub: Closed.\r\n");
}

void sh2_service(void) {
    // Usually, this reads the HAL and triggers the event callback.
}
