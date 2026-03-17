#include "bno085.h"
#include "sh2.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "BNO085_DRIVER";

// Callback from the SH-2 library when data/events are ready
static void sensor_event_callback(void *cookie) {
    // Usually casts cookie to an event struct and parses quaternions
    // Just a stub for now.
    // ESP_LOGI(TAG, "Sensor event received!");
}

esp_err_t bno085_init(bno085_hal_config_t *config) {
    ESP_LOGI(TAG, "Initializing BNO085 wrapper...");

    // 1. Init actual ESP-IDF hardware (SPI + GPIO)
    sh2_hal_t *hal = sh2_hal_esp32_init(config);
    if (!hal) {
        ESP_LOGE(TAG, "Failed to initialize HAL.");
        return ESP_FAIL;
    }

    // 2. Pass the HAL back up to the CEVA SH-2 library
    int status = sh2_open(hal, sensor_event_callback, NULL);
    if (status != 0) {
        ESP_LOGE(TAG, "sh2_open failed with status: %d", status);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "BNO085 initialized successfully.");
    return ESP_OK;
}

void bno085_service_task(void *pvParameters) {
    ESP_LOGI(TAG, "Starting BNO085 service task...");

    while (1) {
        // Poll the CEVA library to check the HAL and process incoming data
        sh2_service();
        
        // Polling heavily right now; when you have interrupts, you can block
        // on a semaphore triggered by INT going low instead of vTaskDelay.
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
