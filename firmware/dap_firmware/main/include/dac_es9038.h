#pragma once
#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

esp_err_t es9038_init();
esp_err_t es9038_set_mute(bool muted);
esp_err_t es9038_set_volume_percent(uint8_t percent);
esp_err_t es9038_probe();
