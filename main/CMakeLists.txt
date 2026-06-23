idf_component_register(SRCS take_picture.c sdcard.c camera.c wifi_setup.c
                        PRIV_INCLUDE_DIRS .
                        PRIV_REQUIRES nvs_flash esp_psram fatfs esp_driver_spi esp_wifi) 
