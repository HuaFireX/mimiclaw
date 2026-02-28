#include "tool_rgb.h"
#include "tools/tool_registry.h"
#include "ws2812_driver.h"
#include "esp_log.h"
#include "cJSON.h"
#include <stdio.h>

static const char *TAG = "rgb";
static ws2812_driver_t *strip = NULL;

esp_err_t tool_rgb_execute(const char *input_json, char *output, size_t output_size)
{
    // 初始化 LED 驱动 (只执行一次)
    if (strip == NULL) {
        // 使用 GPIO48，根据用户反馈这是正确的引脚
        esp_err_t init_ret = ws2812_init(48, 1, &strip);
        if (init_ret != ESP_OK) {
            snprintf(output, output_size, "Failed to initialize RGB driver: %s", esp_err_to_name(init_ret));
            return init_ret;
        }
    }
    
    // 使用 cJSON 解析输入，比 sscanf 更健壮
    cJSON *root = cJSON_Parse(input_json);
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON: %s", input_json);
        snprintf(output, output_size, "Invalid JSON input for rgb_set");
        return ESP_ERR_INVALID_ARG;
    }

    int r = 0, g = 0, b = 0;
    
    // 支持大小写不敏感的查找 (以防 LLM 生成 R/G/B)
    cJSON *item_r = cJSON_GetObjectItem(root, "r");
    if (!item_r) item_r = cJSON_GetObjectItem(root, "R");
    
    cJSON *item_g = cJSON_GetObjectItem(root, "g");
    if (!item_g) item_g = cJSON_GetObjectItem(root, "G");
    
    cJSON *item_b = cJSON_GetObjectItem(root, "b");
    if (!item_b) item_b = cJSON_GetObjectItem(root, "B");

    // 尝试解析值，即使它是字符串格式
    if (cJSON_IsNumber(item_r)) r = item_r->valueint;
    else if (cJSON_IsString(item_r)) r = atoi(item_r->valuestring);

    if (cJSON_IsNumber(item_g)) g = item_g->valueint;
    else if (cJSON_IsString(item_g)) g = atoi(item_g->valuestring);

    if (cJSON_IsNumber(item_b)) b = item_b->valueint;
    else if (cJSON_IsString(item_b)) b = atoi(item_b->valuestring);

    cJSON_Delete(root);
    
    // 限制范围 0-255
    r = (r > 255) ? 255 : ((r < 0) ? 0 : r);
    g = (g > 255) ? 255 : ((g < 0) ? 0 : g);
    b = (b > 255) ? 255 : ((b < 0) ? 0 : b);
    
    ESP_LOGI(TAG, "Executing RGB set: R=%d, G=%d, B=%d", r, g, b);
    
    esp_err_t ret = ws2812_set_pixel(strip, 0, r, g, b);
    if (ret == ESP_OK) {
        ret = ws2812_refresh(strip);
    }
    
    if (ret == ESP_OK) {
        snprintf(output, output_size, "RGB LED successfully set to R:%d G:%d B:%d", r, g, b);
        ESP_LOGI(TAG, "RGB Set Success");
    } else {
        snprintf(output, output_size, "Failed to set RGB: %s", esp_err_to_name(ret));
        ESP_LOGE(TAG, "RGB Set Failed: %s", esp_err_to_name(ret));
    }
    
    return ret;
}
