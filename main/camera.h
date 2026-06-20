#include <esp_log.h>
#include "esp_camera.h"

#if defined(CONFIG_CAMERA_AF_SUPPORT) && CONFIG_CAMERA_AF_SUPPORT
#include "esp_camera_af.h"
#endif

// ESP32S3 (XIAO)
#define CAM_PIN_PWDN -1
#define CAM_PIN_RESET -1
#define CAM_PIN_VSYNC 38
#define CAM_PIN_HREF 47
#define CAM_PIN_PCLK 13
#define CAM_PIN_XCLK 10
#define CAM_PIN_SIOD 40
#define CAM_PIN_SIOC 39
#define CAM_PIN_D0 15
#define CAM_PIN_D1 17
#define CAM_PIN_D2 18
#define CAM_PIN_D3 16
#define CAM_PIN_D4 14
#define CAM_PIN_D5 12
#define CAM_PIN_D6 11
#define CAM_PIN_D7 48

esp_err_t init_camera(framesize_t, int);
camera_fb_t* take_pic(void);
#if defined(CONFIG_CAMERA_AF_SUPPORT) && CONFIG_CAMERA_AF_SUPPORT
static void maybe_init_autofocus(void);
#endif

