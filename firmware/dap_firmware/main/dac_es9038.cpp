#include "dac_es9038.h"
#include "board_pins.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

static const char *TAG = "es9038";
static i2c_master_bus_handle_t s_i2c_bus = nullptr;
static i2c_master_dev_handle_t s_dac = nullptr;
static SemaphoreHandle_t s_i2c_mutex = nullptr;

namespace {
constexpr uint8_t REG_FILTER_SETTINGS = 0x07;
constexpr uint8_t REG_SOFT_START = 0x0E;
constexpr uint8_t REG_VOLUME_LEFT = 0x0F;
constexpr uint8_t REG_VOLUME_RIGHT = 0x10;
constexpr uint8_t DAC_MUTE_BIT = 0x01;
constexpr uint8_t FILTER_DEFAULT_APODIZING = 0x80;
// Register 14: enable the analog output ramp; preserve datasheet default
// soft_start_time=10 and soft_start_on_lock=0 (0x80 | 0x0A).
constexpr uint8_t SOFT_START_NORMAL = 0x8A;

esp_err_t write_reg_locked(uint8_t reg, uint8_t value)
{
    if (s_dac == nullptr) return ESP_ERR_INVALID_STATE;
    const uint8_t tx[2] = {reg, value};
    return i2c_master_transmit(s_dac, tx, sizeof(tx), 100);
}
}

esp_err_t es9038_probe()
{
    if (s_i2c_bus == nullptr) return ESP_ERR_INVALID_STATE;
    return i2c_master_probe(s_i2c_bus, board::DAC_I2C_ADDRESS, 100);
}

esp_err_t es9038_init()
{
    gpio_config_t reset_cfg{};
    reset_cfg.pin_bit_mask = 1ULL << board::DAC_RESET_N;
    reset_cfg.mode = GPIO_MODE_OUTPUT;
    reset_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    reset_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    reset_cfg.intr_type = GPIO_INTR_DISABLE;
    ESP_RETURN_ON_ERROR(gpio_config(&reset_cfg), TAG, "DAC reset GPIO config failed");

    // RESETB has an external 10 kOhm pull-up in the schematic. Assert reset,
    // allow the 3.3 V/oscillator rails to settle, then release it.
    ESP_RETURN_ON_ERROR(gpio_set_level(board::DAC_RESET_N, 0), TAG, "Could not assert DAC reset");
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_RETURN_ON_ERROR(gpio_set_level(board::DAC_RESET_N, 1), TAG, "Could not release DAC reset");
    vTaskDelay(pdMS_TO_TICKS(50));

    i2c_master_bus_config_t bus_cfg{};
    bus_cfg.i2c_port = I2C_NUM_0;
    bus_cfg.sda_io_num = board::I2C_SDA;
    bus_cfg.scl_io_num = board::I2C_SCL;
    bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_cfg.glitch_ignore_cnt = 7;
    bus_cfg.intr_priority = 0;
    bus_cfg.trans_queue_depth = 0;
    bus_cfg.flags.enable_internal_pullup = false; // external 4.7 kOhm pull-ups are fitted
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &s_i2c_bus), TAG, "I2C bus init failed");

    i2c_device_config_t dev_cfg{};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = board::DAC_I2C_ADDRESS;
    dev_cfg.scl_speed_hz = 400000;
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(s_i2c_bus, &dev_cfg, &s_dac), TAG,
                        "Could not add ES9038Q2M to I2C bus");

    s_i2c_mutex = xSemaphoreCreateMutex();
    if (s_i2c_mutex == nullptr) return ESP_ERR_NO_MEM;

    esp_err_t probe_err = es9038_probe();
    if (probe_err != ESP_OK) {
        ESP_LOGE(TAG, "ES9038Q2M not responding at 7-bit address 0x%02X", board::DAC_I2C_ADDRESS);
        return probe_err;
    }

    // Register 14 bit 7 must be set or the DAC analog outputs remain ramped
    // to GND. Preserve the documented default ramp time (10) and mode.
    // Then start muted with the default apodizing filter (Register 7 bit 0).
    if (xSemaphoreTake(s_i2c_mutex, pdMS_TO_TICKS(100)) != pdTRUE) return ESP_ERR_TIMEOUT;
    esp_err_t err = write_reg_locked(REG_SOFT_START, SOFT_START_NORMAL);
    if (err == ESP_OK) {
        err = write_reg_locked(REG_FILTER_SETTINGS,
                               static_cast<uint8_t>(FILTER_DEFAULT_APODIZING | DAC_MUTE_BIT));
    }
    xSemaphoreGive(s_i2c_mutex);
    if (err != ESP_OK) return err;

    // Conservative power-on level: 0x50 is approximately -40 dB per channel
    // in the ES9038Q2M's 0.5 dB digital attenuation register.
    ESP_RETURN_ON_ERROR(es9038_set_volume_percent(33), TAG, "Initial DAC volume failed");
    ESP_LOGI(TAG, "ES9038Q2M ready at I2C 0x%02X (initial mute, ~-40 dB)", board::DAC_I2C_ADDRESS);
    return ESP_OK;
}

esp_err_t es9038_set_mute(bool muted)
{
    if (s_i2c_mutex == nullptr) return ESP_ERR_INVALID_STATE;
    if (xSemaphoreTake(s_i2c_mutex, pdMS_TO_TICKS(100)) != pdTRUE) return ESP_ERR_TIMEOUT;
    esp_err_t err = write_reg_locked(REG_FILTER_SETTINGS,
                                     static_cast<uint8_t>(FILTER_DEFAULT_APODIZING |
                                                          (muted ? DAC_MUTE_BIT : 0)));
    xSemaphoreGive(s_i2c_mutex);
    return err;
}

esp_err_t es9038_set_volume_percent(uint8_t percent)
{
    if (s_i2c_mutex == nullptr) return ESP_ERR_INVALID_STATE;
    if (percent > 100) percent = 100;

    // Map 0..100 to 60..0 dB attenuation in 0.5 dB steps.
    // 33% is approximately -40 dB, making the startup level conservative.
    const uint8_t attenuation_half_db = static_cast<uint8_t>(((100U - percent) * 120U + 50U) / 100U);

    if (xSemaphoreTake(s_i2c_mutex, pdMS_TO_TICKS(100)) != pdTRUE) return ESP_ERR_TIMEOUT;
    esp_err_t err = write_reg_locked(REG_VOLUME_LEFT, attenuation_half_db);
    if (err == ESP_OK) err = write_reg_locked(REG_VOLUME_RIGHT, attenuation_half_db);
    xSemaphoreGive(s_i2c_mutex);
    return err;
}
