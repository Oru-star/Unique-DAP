#pragma once
#include "esp_err.h"
#include <stddef.h>

constexpr size_t DAP_MAX_TRACKS = 256;
constexpr size_t DAP_PATH_MAX = 256;

esp_err_t storage_init();
esp_err_t storage_scan();
size_t storage_track_count();
bool storage_get_track_path(size_t index, char *out, size_t out_size);
void storage_get_track_title(size_t index, char *out, size_t out_size);
