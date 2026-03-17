#ifndef SH2_HAL_H
#define SH2_HAL_H

#include <stdint.h>
#include <stdbool.h>

// This is a stub representation of the CEVA sh2_hal_t structure
// Replace this file with the actual ones from the CEVA driver later.
typedef struct sh2_hal_s {
    int (*open)(struct sh2_hal_s *self);
    void (*close)(struct sh2_hal_s *self);
    int (*read)(struct sh2_hal_s *self, uint8_t *pBuffer, unsigned len, uint32_t *t_us);
    int (*write)(struct sh2_hal_s *self, uint8_t *pBuffer, unsigned len);
    uint32_t (*getTimeUs)(struct sh2_hal_s *self);
} sh2_hal_t;

#endif // SH2_HAL_H
