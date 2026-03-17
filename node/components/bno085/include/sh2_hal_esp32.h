#ifndef SH2_HAL_ESP32_H
#define SH2_HAL_ESP32_H

#include "sh2_hal.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

// Configuration struct for the ESP32 SPI HAL
typedef struct {
    spi_host_device_t spi_host;  // e.g. SPI2_HOST (FSPI)
    gpio_num_t cs_pin;           // Chip Select pin
    gpio_num_t int_pin;          // Interrupt pin (Data Ready)
    gpio_num_t rst_pin;          // Reset pin
    int clock_speed_hz;          // SPI clock speed (max 10MHz, default 3MHz)
} bno085_hal_config_t;

// Returns the populated struct to be passed to sh2_open()
sh2_hal_t* sh2_hal_esp32_init(bno085_hal_config_t *config);

#endif // SH2_HAL_ESP32_H
