/*
    - HTTP requests
    - User presses button on page to send commands
        - Each button pressed on page triggers a specific POST request
        - For config changes (quality, shot interval, etc.), just save to NVS
            - After preview is over, device will read NVS for settings
        - For state changes (setup->start_timelapse), can terminate/pause preview sets the appropriate state change
    - Handler for all user-configurable features




    endpoints:
        - GET stream
        - GET capture jpg
        - GET test shot
        - GET config

        - POST start
        - POST config

*/
#pragma once

#include "freertos/queue.h"
#include "esp_http_server.h"
#include "esp_err.h"
#include "esp_log.h"

#include "app_events.h"


esp_err_t start_server(QueueHandle_t, httpd_handle_t*);
esp_err_t stop_server(httpd_handle_t*);