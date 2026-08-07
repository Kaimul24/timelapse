#include "server.h"
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "http_parser.h"

#include "camera.h"
#include "portmacro.h"

static const char* TAG = "webserver";

static esp_err_t stream_get_handler(httpd_req_t*);
static esp_err_t capture_jpg_handler(httpd_req_t*);
static esp_err_t test_shot_handler(httpd_req_t*);
static esp_err_t timelapse_start_handler(httpd_req_t*);
static esp_err_t config_get_handler(httpd_req_t*);
static esp_err_t config_post_handler(httpd_req_t*);
static esp_err_t shutdown_handler(httpd_req_t*);

static httpd_uri_t stream = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_get_handler
};

static httpd_uri_t capture_jpeg = {
    .uri = "/capture.jpg",
    .method = HTTP_GET,
    .handler = capture_jpg_handler
};

static httpd_uri_t test_shot = {
    .uri = "/test_shot",
    .method = HTTP_POST,
    .handler = test_shot_handler,
    .user_ctx = NULL
};

static httpd_uri_t timelapse_start = {
    .uri = "/start",
    .method = HTTP_POST,
    .handler = timelapse_start_handler,
    .user_ctx = NULL
};

static httpd_uri_t config_get = {
    .uri = "/config",
    .method = HTTP_GET,
    .handler = config_get_handler
};

static httpd_uri_t config_post = {
    .uri = "/config",
    .method = HTTP_POST,
    .handler = config_post_handler
};

static httpd_uri_t shutdown = {
    .uri = "/shutdown",
    .method = HTTP_POST,
    .handler = shutdown_handler,
    .user_ctx = NULL
};

esp_err_t start_server(QueueHandle_t queue, httpd_handle_t *server) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    if (httpd_start(server, &config) == ESP_OK) {
        // handle URIs
        test_shot.user_ctx = queue;
        timelapse_start.user_ctx = queue;
        shutdown.user_ctx = queue;

        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &stream);
        httpd_register_uri_handler(server, &capture_jpeg);
        httpd_register_uri_handler(server, &test_shot);
        httpd_register_uri_handler(server, &timelapse_start);
        httpd_register_uri_handler(server, &config_get);
        httpd_register_uri_handler(server, &config_post);
        ESP_LOGI(TAG, "Started web server");
        return ESP_OK;
    }   

    ESP_LOGE(TAG, "Failed to start web server");
    return ESP_FAIL;
}

esp_err_t stop_server(httpd_handle_t *server) {
    return httpd_stop(server);
}

static esp_err_t stream_get_handler(httpd_req_t* req) {

}

static esp_err_t capture_jpg_handler(httpd_req_t* req) {
    camera_fb_t *frame = cam_get_frame();

    if (frame == NULL) {
        return httpd_resp_send_err(
            req,
            HTTPD_500_INTERNAL_SERVER_ERROR,
            "Camera busy"
        );
    }

    esp_err_t res = httpd_resp_set_type(req, "image/jpeg");
    if (res == ESP_OK) {
        res = httpd_resp_set_hdr(
            req, "Cache-Control", "no-store");
    }

    if (res == ESP_OK) {
        res = httpd_resp_send(
            req,
            (const char *)frame->buf,
            frame->len
        );
    }

    cam_release_frame(frame);
    return res;
}

static esp_err_t test_shot_handler(httpd_req_t* req) { 
    QueueHandle_t queue = (QueueHandle_t)req->user_ctx;
    app_event_t event = APP_EVENT_TEST_SHOT;

    if (xQueueSendToBack(queue, &event, pdMS_TO_TICKS(100)) != pdTRUE ) {
        return httpd_resp_send_err(
            req,
            HTTPD_500_INTERNAL_SERVER_ERROR,
            "Queue full"
        );
    }

    httpd_resp_set_status(req, "202 Accepted");
    return httpd_resp_sendstr(req, "Test shot requested");
}

static esp_err_t timelapse_start_handler(httpd_req_t* req) {

}

static esp_err_t config_get_handler(httpd_req_t* req) {

}

static esp_err_t config_post_handler(httpd_req_t* req) {

}

static esp_err_t shutdown_handler(httpd_req_t* req) {

}