#include "sh2_hal_esp32.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "SH2_HAL_ESP32";

typedef struct {
    sh2_hal_t hal; 
    spi_device_handle_t spi;
    bno085_hal_config_t cfg;
} sh2_hal_esp32_t;

static sh2_hal_esp32_t p_hal;

// HAL Func: Open the interface (Reset and wait)
static int hal_open(sh2_hal_t *self) {
    sh2_hal_esp32_t *ctx = (sh2_hal_esp32_t*)self;
    ESP_LOGI(TAG, "HAL Open: Resetting BNO085...");

    if (ctx->cfg.rst_pin != GPIO_NUM_NC) {
        gpio_set_level(ctx->cfg.rst_pin, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(ctx->cfg.rst_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(100)); // Wait for boot
    }
    return 0; // Success
}

// HAL Func: Close the interface
static void hal_close(sh2_hal_t *self) {
    sh2_hal_esp32_t *ctx = (sh2_hal_esp32_t*)self;
    ESP_LOGI(TAG, "HAL Close");
    if (ctx->cfg.rst_pin != GPIO_NUM_NC) {
        gpio_set_level(ctx->cfg.rst_pin, 0); // Put into reset
    }
}

// HAL Func: Read data from sensor
static int hal_read(sh2_hal_t *self, uint8_t *pBuffer, unsigned len, uint32_t *t_us) {
    sh2_hal_esp32_t *ctx = (sh2_hal_esp32_t*)self;
    
    // Check INT pin here if we were doing pure polling.
    // In SPI mode, BNO085 drops INT low when data is ready.
    if (gpio_get_level(ctx->cfg.int_pin) != 0) {
        return 0; // No data ready
    }

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8; // In bits
    t.rx_buffer = pBuffer;
    t.tx_buffer = NULL; // Full duplex, but we just want to read

    esp_err_t ret = spi_device_transmit(ctx->spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI Read failed: %s", esp_err_to_name(ret));
        return 0;
    }

    if (t_us) {
        *t_us = (uint32_t)esp_timer_get_time();
    }
    
    // For SHTP over SPI, the first 2 bytes describe the packet length
    uint16_t packet_len = pBuffer[0] | (pBuffer[1] << 8);
    packet_len &= 0x7FFF; // Ignore MSB which is continuation flag
    
    return packet_len;
}

// HAL Func: Write data to sensor
static int hal_write(sh2_hal_t *self, uint8_t *pBuffer, unsigned len) {
    sh2_hal_esp32_t *ctx = (sh2_hal_esp32_t*)self;

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8; // In bits
    t.tx_buffer = pBuffer;
    t.rx_buffer = NULL;

    esp_err_t ret = spi_device_transmit(ctx->spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI Write failed: %s", esp_err_to_name(ret));
        return 0; // Failure
    }
    return len;
}

// HAL Func: Get current time
static uint32_t hal_getTimeUs(sh2_hal_t *self) {
    return (uint32_t)esp_timer_get_time();
}

sh2_hal_t* sh2_hal_esp32_init(bno085_hal_config_t *config) {
    p_hal.cfg = *config;
    p_hal.hal.open = hal_open;
    p_hal.hal.close = hal_close;
    p_hal.hal.read = hal_read;
    p_hal.hal.write = hal_write;
    p_hal.hal.getTimeUs = hal_getTimeUs;

    // 1. Configure the SPI Device (BNO085 requires Mode 3)
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = config->clock_speed_hz > 0 ? config->clock_speed_hz : 3000000,
        .mode = 3,          // CPOL=1, CPHA=1 as per datasheet
        .spics_io_num = config->cs_pin,
        .queue_size = 7,    
        // CS hold and setup could be adjusted if there are timing issues
    };
    
    esp_err_t ret = spi_bus_add_device(config->spi_host, &devcfg, &p_hal.spi);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
        return NULL;
    }

    // 2. Configure INT and RST pins
    gpio_config_t io_conf = {};
    if (config->rst_pin != GPIO_NUM_NC) {
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << config->rst_pin);
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;
        gpio_config(&io_conf);
        gpio_set_level(config->rst_pin, 1);
    }

    io_conf.intr_type = GPIO_INTR_DISABLE; // Could enable falling edge interrupt later
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << config->int_pin);
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "HAL initialized. SPI Device attached.");
    return &p_hal.hal;
}
