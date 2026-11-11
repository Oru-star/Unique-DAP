#include "board.h"
#include "board_pins.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "board";

esp_err_t board_power_init()
{
    // GPIO45 is an ESP32-S3 strapping pin. The external 100 kOhm pulldown on
    // BOOST_EN keeps the boost converter disabled during reset/strap sampling.
    // Only configure it after the ESP-IDF application has started.
    gpio_config_t boost_cfg{};
    boost_cfg.pin_bit_mask = 1ULL << board::BOOST_EN;
    boost_cfg.mode = GPIO_MODE_OUTPUT;
    boost_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    boost_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    boost_cfg.intr_type = GPIO_INTR_DISABLE;
    esp_err_t err = gpio_config(&boost_cfg);
    if (err != ESP_OK) return err;

    // Hold the external 5 V rail off while GPIO state is established.
    ESP_RETURN_ON_ERROR(gpio_set_level(board::BOOST_EN, 0), TAG, "Could not hold boost disabled");
    vTaskDelay(pdMS_TO_TICKS(20));

    // The 5 V rail feeds the Sharp display and headphone amplifier supply.
    ESP_RETURN_ON_ERROR(gpio_set_level(board::BOOST_EN, 1), TAG, "Could not enable 5 V boost");
    vTaskDelay(pdMS_TO_TICKS(50));

    ESP_LOGI(TAG, "BOOST_EN asserted; allow external 5 V rail to settle");
    return ESP_OK;
}
