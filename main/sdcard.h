#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "esp_camera.h"

#define MAX_FILES 5
#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MOSI 9
#define PIN_NUM_MISO 8
#define PIN_NUM_CLK 7
#define PIN_NUM_CS 21

esp_err_t init_sdcard(void);
esp_err_t save_jpeg(camera_fb_t*, const char*);