#ifndef BNO085_H
#define BNO085_H

#include "sh2_hal_esp32.h"
#include "esp_err.h"

// Initialize the generic BNO085 driver
esp_err_t bno085_init(bno085_hal_config_t *config);

// Run the service polling loop for the BNO085 driver
void bno085_service_task(void *pvParameters);

#endif // BNO085_H
