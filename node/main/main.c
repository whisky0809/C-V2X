#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "bno085.h"

static const char *TAG = "MAIN";

// Default standard ESP32-S3 FSPI Pins
#define BNO085_MISO_PIN GPIO_NUM_13
#define BNO085_MOSI_PIN GPIO_NUM_11
#define BNO085_SCLK_PIN GPIO_NUM_12
#define BNO085_CS_PIN   GPIO_NUM_10
#define BNO085_INT_PIN  GPIO_NUM_9
#define BNO085_RST_PIN  GPIO_NUM_8

void app_main(void)
{
    ESP_LOGI(TAG, "Starting BNO085 SPI C-V2X Node...");

    // 1. Initialize the SPI Bus
    spi_bus_config_t buscfg = {
        .miso_io_num = BNO085_MISO_PIN,
        .mosi_io_num = BNO085_MOSI_PIN,
        .sclk_io_num = BNO085_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 1024,
    };

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    // 2. Configure BNO085 HAL
    bno085_hal_config_t cfg = {
        .spi_host = SPI2_HOST,
        .cs_pin = BNO085_CS_PIN,
        .int_pin = BNO085_INT_PIN,
        .rst_pin = BNO085_RST_PIN,
        .clock_speed_hz = 3000000 // 3 MHz baseline
    };

    // 3. Initialize wrapper
    if (bno085_init(&cfg) == ESP_OK) {
        // Start the background data processing task
        xTaskCreate(bno085_service_task, "bno085_service", 4096, NULL, 5, NULL);
    } else {
        ESP_LOGE(TAG, "Initialization failed. Halting.");
        while(1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    while (1) {
        // Main thread can handle V2X or other business logic here
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
