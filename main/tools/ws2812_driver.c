#include "ws2812_driver.h"
#include "esp_log.h"
#include "esp_check.h"
#include <string.h>
#include <malloc.h>

static const char *TAG = "ws2812";

esp_err_t ws2812_init(int gpio_num, int num_leds, ws2812_driver_t **out_driver)
{
    ws2812_driver_t *driver = calloc(1, sizeof(ws2812_driver_t));
    if (driver == NULL) {
        return ESP_ERR_NO_MEM;
    }
    
    driver->gpio_num = gpio_num;
    driver->num_leds = num_leds;
    
    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio_num,
        .max_leds = num_leds,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags.invert_out = false,
    };
    
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    
    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &driver->led_strip);
    if (ret != ESP_OK) {
        free(driver);
        return ret;
    }
    
    *out_driver = driver;
    ESP_LOGI(TAG, "WS2812 initialized on GPIO%d with %d LEDs", gpio_num, num_leds);
    return ESP_OK;
}

esp_err_t ws2812_set_pixel(ws2812_driver_t *driver, int index, uint8_t r, uint8_t g, uint8_t b)
{
    if (driver == NULL || index < 0 || index >= driver->num_leds) {
        return ESP_ERR_INVALID_ARG;
    }
    return led_strip_set_pixel(driver->led_strip, index, r, g, b);
}

esp_err_t ws2812_refresh(ws2812_driver_t *driver)
{
    if (driver == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    return led_strip_refresh(driver->led_strip);
}

esp_err_t ws2812_clear(ws2812_driver_t *driver)
{
    if (driver == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    return led_strip_clear(driver->led_strip);
}

esp_err_t ws2812_deinit(ws2812_driver_t *driver)
{
    if (driver) {
        if (driver->led_strip) {
            led_strip_del(driver->led_strip);
        }
        free(driver);
    }
    return ESP_OK;
}
