idf_component_register(
    SRCS "bno085.c" "sh2_hal_esp32.c" "sh2_stub.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_gpio esp_driver_spi
    PRIV_REQUIRES esp_timer freertos
)
