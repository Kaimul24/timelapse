#include "sdkconfig.h"

#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdcard.h"
#include "camera.h"

#define BOARD_ESP32S3_XIAO
#define CORE0 0

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

typedef enum {
    APP_STATE_BOOT = 0,
    APP_STATE_SETUP,
    APP_STATE_TEST_SHOT,
    APP_STATE_START_TIMELAPSE,
    APP_STATE_CAPTURE_FRAME,
    APP_STATE_SLEEP,
    APP_STATE_ERROR
} app_state_t;

static app_state_t app_state = APP_STATE_BOOT;
static const char *TAG = "Timelapse App";
static TaskHandle_t picture_task_handle = NULL;

static void picture_task(void* params) {
    for (;;) {
        camera_fb_t *pic = take_pic();
        if (pic) {
            // use pic->buf to access the image
            ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
            save_jpeg(pic);
            
            esp_camera_fb_return(pic);
        } else {
            ESP_LOGE(TAG, "Picture Failed!");
        }
        
        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(init_camera(FRAMESIZE_QXGA, 12));

#if defined(CONFIG_CAMERA_AF_SUPPORT) && CONFIG_CAMERA_AF_SUPPORT
    // Initialize autofocus if configured and supported by the sensor.
    // In menuconfig: Component config → Camera configuration → Enable autofocus support
    maybe_init_autofocus();
#endif

    ESP_ERROR_CHECK(init_sdcard());
    xTaskCreatePinnedToCore(picture_task, "picture_task", 4096, NULL, 3, &picture_task_handle, CORE0);

    while (1){ 
        vTaskDelay(pdMS_TO_TICKS(1000));  
    }
}
