#pragma once

#include "esp_err.h"
#include "led_strip.h"

typedef struct {
    int gpio_num;
    int num_leds;
    led_strip_handle_t led_strip;
} ws2812_driver_t;

esp_err_t ws2812_init(int gpio_num, int num_leds, ws2812_driver_t **out_driver);
esp_err_t ws2812_set_pixel(ws2812_driver_t *driver, int index, uint8_t r, uint8_t g, uint8_t b);
esp_err_t ws2812_refresh(ws2812_driver_t *driver);
esp_err_t ws2812_clear(ws2812_driver_t *driver);
esp_err_t ws2812_deinit(ws2812_driver_t *driver);
