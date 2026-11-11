#pragma once
#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct AudioPlayerStatus {
    bool playing;
    bool paused;
    uint32_t sample_rate;
    uint8_t bit_depth;
    uint32_t elapsed_seconds;
    uint32_t duration_seconds;
    char path[256];
    char error[48];
};

esp_err_t audio_player_init();
esp_err_t audio_player_play(const char *path);
esp_err_t audio_player_toggle_pause();
esp_err_t audio_player_set_user_mute(bool muted);
esp_err_t audio_player_stop();
void audio_player_get_status(AudioPlayerStatus *out);
