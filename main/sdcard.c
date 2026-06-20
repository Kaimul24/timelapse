#include "sdcard.h"

static const char *TAG = "sdcard";

esp_err_t init_sdcard(void) {
    ESP_LOGI(TAG, "Initializing SD Card...");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus.");
        return ESP_FAIL;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = MAX_FILES,
    };

    sdmmc_card_t *card;
    esp_err_t err = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (err != ESP_OK) {
        if (err == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(err));
        }
        return err;
    }
    ESP_LOGI(TAG, "SD Card Initialized");
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

esp_err_t save_jpeg(camera_fb_t* pic) {
    char photo_name[50];
    sprintf(photo_name, MOUNT_POINT "/pic_%lli.jpg", pic->timestamp.tv_sec);
    ESP_LOGI(TAG, "Saving JPEG to %s", photo_name);
    FILE *file = fopen(photo_name, "w");

    if (file == NULL) {
        ESP_LOGE(TAG, "fopen failed!");
        return ESP_FAIL;
    } else {
        size_t ret = fwrite(pic->buf, 1, pic->len, file);
        if (ret != pic->len) ESP_LOGE(TAG, "fwrite failed!");
        fclose(file);
    }
    return ESP_OK;
}