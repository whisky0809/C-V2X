#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

/**
 * @brief Initializes the MicroSD card using the high-speed SDIO protocol (4-bit bus).
 * * This function configures the native SDMMC host of the ESP32 and mounts the FAT
 * file system on the SD card, making it ready for high-frequency RTK GNSS data logging.
 * If the card is not formatted in FAT32, the function will attempt to format it automatically.
 * * @return esp_err_t Returns ESP_OK if the mounting and initialization are successful, 
 * otherwise returns a specific ESP-IDF error code.
 */

esp_err_t init_microsd_sdio(void) {
    esp_err_t ret;
    
    // Configuration for mounting the FAT file system
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true, // Automatically format if the SD is empty/unformatted
        .max_files = 5,                 // Maximum number of concurrently open files
        .allocation_unit_size = 16 * 1024
    };

    // Use default settings for the SDMMC host (maps to hardware SDIO pins)
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED; // Set to the maximum allowed speed

    // Configure the slot (ESP32 supports 1-bit or 4-bit, we go straight to 4-bit)
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 4; // Enable the 4-line data bus for maximum performance

    sdmmc_card_t *card;
    const char mount_point[] = "/sdcard";

    printf("Initializing MicroSD on 4-line SDIO bus...\n");

    // Mount the partition and physically initialize the card
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        printf("Critical Error: Failed to mount the MicroSD card!\n");
        return ret;
    }

    printf("MicroSD successfully mounted at path %s\n", mount_point);
    
    // Print card info to the terminal (capacity, speed, type)
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}


/**
 * @brief Appends a string of GNSS data to a log file on the MicroSD card.
 *
 * This function opens the specified file in append mode ("a"), writes the 
 * provided data string, and safely closes the file. If the file does not 
 * exist, it will be automatically created.
 *
 * @param file_path The absolute path of the file (e.g., "/sdcard/rtk_log.csv").
 * @param data_string The null-terminated string containing the GNSS data to log.
 * @return esp_err_t Returns ESP_OK on success, or ESP_FAIL if the file cannot be opened.
 */

esp_err_t log_gnss_data(const char* file_path, const char* data_string) {
    // Open the file in append mode ("a") so we don't overwrite previous logs
    FILE* f = fopen(file_path, "a");
    if (f == NULL) {
        printf("Error: Failed to open file %s for appending\n", file_path);
        return ESP_FAIL;
    }

    // Write the data string to the file, followed by a newline character
    fprintf(f, "%s\n", data_string);
    
    // Close the file immediately to ensure data is physically saved to the SD card
    fclose(f);

    return ESP_OK;
}


