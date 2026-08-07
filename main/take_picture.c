#include "esp_camera.h"
#include "sdkconfig.h"

#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <stdbool.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "app_events.h"
#include "sdcard.h"
#include "camera.h"
#include "wifi_setup.h"
#include "server.h"

#define BOARD_ESP32S3_XIAO
#define CORE0 0

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#define QUEUE_SIZE 5

typedef enum {
    APP_STATE_BOOT = 0,
    APP_STATE_SETUP,
    APP_STATE_TEST_SHOT,
    APP_STATE_START_TIMELAPSE,
    APP_STATE_CAPTURE_FRAME,
    APP_STATE_SLEEP,
    APP_STATE_ERROR
} app_state_t;

typedef struct {
    QueueHandle_t event_queue;
    httpd_handle_t server;
    bool active;
} setup_context_t;

static app_state_t app_state = APP_STATE_BOOT;
static setup_context_t setup;
static const char *TAG = "Timelapse App";

// TODO: Add persisent state via NVS
app_state_t handle_boot(void) {
    return APP_STATE_SETUP;
}

/*
    1. Start WiFi
    2. Start web server
    3. Remain in setup mode unless user moves to TEST_SHOT or START_TIMELAPSE
*/
app_state_t handle_setup(void) {
    app_event_t event;

    if (!setup.active) {
        wifi_init_softap();
        setup.event_queue = xQueueCreate(QUEUE_SIZE, sizeof(app_event_t));
        esp_err_t ret = start_server(setup.event_queue, &setup.server);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Server failed to start!");
            wifi_deinit_softap();
            return APP_STATE_ERROR;
        }
        setup.active = true;
    }
    
    if (xQueueReceive(setup.event_queue, &event, portMAX_DELAY) != pdTRUE) {
        return APP_STATE_ERROR;
    }

    switch (event) {
        case APP_EVENT_TEST_SHOT:
            return APP_STATE_TEST_SHOT;
        case APP_EVENT_START_TIMELAPSE:
            wifi_deinit_softap();
            stop_server(setup.server);
            setup.active = false;
            return APP_STATE_START_TIMELAPSE;
        case APP_EVENT_SHUTDOWN:
            wifi_deinit_softap();
            stop_server(setup.server);
            setup.active = false;
            return APP_STATE_SLEEP;
        default:
            wifi_deinit_softap();
            stop_server(setup.server);
            setup.active = false;
            return APP_STATE_ERROR;
    }

}  

/*      
    1. Pause preview streaming
    2. Take shot
    3. Send + display to user's device
    4. Resume preview streaming after confirmation from user
*/
app_state_t handle_test_shot(void) {
    camera_fb_t *frame = cam_get_frame();
    
    if (frame == NULL) {
        ESP_LOGE(TAG, "Could not take picture, returning to setup");
        return APP_STATE_SETUP;
    }

    // Write to micro sd
    save_jpeg(frame, "test_shot");

    cam_release_frame(frame);
    return APP_STATE_SETUP;
}

app_state_t handle_start_timelapse(void) {

}

app_state_t handle_capture_frame(void) {
    camera_fb_t *frame = cam_get_frame();

    if (frame == NULL) {
        ESP_LOGE(TAG, "Could not take picture");
        return APP_STATE_ERROR;
    }

    save_jpeg(frame, "pic");

    cam_release_frame(frame);
    return APP_STATE_SLEEP;
}

static void enter_sleep(void) {

}

static void handle_error(void) {

}

void app_main(void) {
    while (1) {
        switch (app_state) {
            case APP_STATE_BOOT:
                app_state = handle_boot();
                break;

            case APP_STATE_SETUP:
                app_state = handle_setup();
                break;

            case APP_STATE_TEST_SHOT:
                app_state = handle_test_shot();
                break;

            case APP_STATE_START_TIMELAPSE:
                app_state = handle_start_timelapse();
                break;

            case APP_STATE_CAPTURE_FRAME:
                app_state = handle_capture_frame();
                break;

            case APP_STATE_SLEEP:
                enter_sleep();
                break;

            case APP_STATE_ERROR:
            default:
                handle_error();
                app_state = APP_STATE_SLEEP;
                break;
        }
    }
}

// void app_main(void) {
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//       ESP_ERROR_CHECK(nvs_flash_erase());
//       ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     ESP_ERROR_CHECK(init_camera(FRAMESIZE_QXGA, 12));

// #if defined(CONFIG_CAMERA_AF_SUPPORT) && CONFIG_CAMERA_AF_SUPPORT
//     // Initialize autofocus if configured and supported by the sensor.
//     // In menuconfig: Component config → Camera configuration → Enable autofocus support
//     maybe_init_autofocus();
// #endif

//     ESP_ERROR_CHECK(init_sdcard());
//     wifi_init_softap();

//     while (1){ 
//         vTaskDelay(pdMS_TO_TICKS(1000));  
//     }
// }
