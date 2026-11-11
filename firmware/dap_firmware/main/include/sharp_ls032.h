#pragma once
#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct DisplayState {
    const char *selected_track;
    const char *playing_track;
    bool playing;
    bool paused;
    uint32_t sample_rate;
    uint8_t bit_depth;
    uint8_t volume_percent;
    size_t track_index;
    size_t track_count;
    uint32_t elapsed_seconds;
    uint32_t duration_seconds;
    const char *message;
};

esp_err_t sharp_display_init();
void sharp_display_render(const DisplayState &state);
