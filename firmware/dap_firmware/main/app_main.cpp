#include "audio_player.h"
#include "board.h"
#include "controls.h"
#include "dac_es9038.h"
#include "sharp_ls032.h"
#include "storage.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

static const char *TAG = "dap_app";

namespace {
QueueHandle_t s_control_queue = nullptr;
bool s_dac_ready = false;
bool s_sd_ready = false;
bool s_display_ready = false;
bool s_muted = false;
uint8_t s_volume = 33;
size_t s_selected_track = 0;

void play_selected_track()
{
    if (!s_dac_ready || !s_sd_ready || storage_track_count() == 0) return;
    char path[DAP_PATH_MAX]{};
    if (!storage_get_track_path(s_selected_track, path, sizeof(path))) return;
    const esp_err_t err = audio_player_play(path);
    if (err != ESP_OK) ESP_LOGE(TAG, "Could not queue track: %s", esp_err_to_name(err));
}

void move_selection(int delta)
{
    const size_t count = storage_track_count();
    if (count == 0) return;
    const int64_t next = static_cast<int64_t>(s_selected_track) + delta;
    s_selected_track = static_cast<size_t>(std::clamp<int64_t>(next, 0, static_cast<int64_t>(count - 1)));
}

void handle_control(const ControlEvent &event)
{
    switch (event.type) {
        case ControlEventType::Encoder1Step:
            move_selection(event.delta);
            break;
        case ControlEventType::Encoder1Click:
            play_selected_track();
            break;
        case ControlEventType::Encoder2Step: {
            const int next = std::clamp(static_cast<int>(s_volume) + static_cast<int>(event.delta) * 2, 0, 100);
            s_volume = static_cast<uint8_t>(next);
            if (s_dac_ready) {
                (void)es9038_set_volume_percent(s_volume);
                if (s_muted) s_muted = false;
                (void)audio_player_set_user_mute(s_muted || s_volume == 0);
            }
            break;
        }
        case ControlEventType::Encoder2Click:
            if (s_dac_ready) {
                s_muted = !s_muted;
                (void)audio_player_set_user_mute(s_muted || s_volume == 0);
            }
            break;
        case ControlEventType::Previous:
            move_selection(-1);
            play_selected_track();
            break;
        case ControlEventType::Next:
            move_selection(1);
            play_selected_track();
            break;
        case ControlEventType::PlayPause: {
            AudioPlayerStatus status{};
            audio_player_get_status(&status);
            if (status.playing) (void)audio_player_toggle_pause();
            else play_selected_track();
            break;
        }
    }
}

void basename_without_extension(const char *path, char *out, size_t out_size)
{
    if (out == nullptr || out_size == 0) return;
    out[0] = '\0';
    if (path == nullptr) return;
    const char *base = strrchr(path, '/');
    base = (base == nullptr) ? path : base + 1;
    strlcpy(out, base, out_size);
    char *dot = strrchr(out, '.');
    if (dot != nullptr) *dot = '\0';
}

void render_ui()
{
    if (!s_display_ready) return;

    char selected_title[128] = "NO TRACKS";
    if (storage_track_count() > 0)
        storage_get_track_title(s_selected_track, selected_title, sizeof(selected_title));

    AudioPlayerStatus audio{};
    audio_player_get_status(&audio);
    char playing_title[128] = "STOPPED";
    if (audio.path[0] != '\0') basename_without_extension(audio.path, playing_title, sizeof(playing_title));

    const char *message = nullptr;
    if (!s_sd_ready) message = "SD CARD NOT MOUNTED";
    else if (!s_dac_ready) message = "DAC NOT DETECTED";
    else if (audio.error[0] != '\0') message = audio.error;

    DisplayState state{};
    state.selected_track = selected_title;
    state.playing_track = playing_title;
    state.playing = audio.playing;
    state.paused = audio.paused;
    state.sample_rate = audio.sample_rate;
    state.bit_depth = audio.bit_depth;
    state.volume_percent = s_volume;
    state.track_index = s_selected_track;
    state.track_count = storage_track_count();
    state.elapsed_seconds = audio.elapsed_seconds;
    state.duration_seconds = audio.duration_seconds;
    state.message = message;
    sharp_display_render(state);
}
}

extern "C" void app_main(void)
{
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES || nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_err);

    ESP_ERROR_CHECK(board_power_init());

    const esp_err_t dac_err = es9038_init();
    s_dac_ready = (dac_err == ESP_OK);
    if (!s_dac_ready) ESP_LOGE(TAG, "DAC init failed; audio output will remain disabled: %s",
                               esp_err_to_name(dac_err));

    const esp_err_t sd_err = storage_init();
    s_sd_ready = (sd_err == ESP_OK);
    if (!s_sd_ready) ESP_LOGE(TAG, "microSD unavailable: %s", esp_err_to_name(sd_err));

    const esp_err_t display_err = sharp_display_init();
    s_display_ready = (display_err == ESP_OK);
    if (!s_display_ready) ESP_LOGE(TAG, "Sharp display init failed: %s", esp_err_to_name(display_err));

    ESP_ERROR_CHECK(audio_player_init());
    s_control_queue = xQueueCreate(16, sizeof(ControlEvent));
    if (s_control_queue == nullptr) ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    ESP_ERROR_CHECK(controls_init(s_control_queue));

    if (s_dac_ready) {
        (void)es9038_set_volume_percent(s_volume);
        (void)audio_player_set_user_mute(s_muted || s_volume == 0);
        (void)es9038_set_mute(true);
    }

    ESP_LOGI(TAG, "DAP firmware started; tracks=%u, SD=%s, DAC=%s, LCD=%s",
             static_cast<unsigned>(storage_track_count()), s_sd_ready ? "OK" : "FAIL",
             s_dac_ready ? "OK" : "FAIL", s_display_ready ? "OK" : "FAIL");

    TickType_t last_render = 0;
    while (true) {
        ControlEvent event{};
        if (xQueueReceive(s_control_queue, &event, pdMS_TO_TICKS(40)) == pdTRUE) {
            handle_control(event);
            while (xQueueReceive(s_control_queue, &event, 0) == pdTRUE) handle_control(event);
        }

        const TickType_t now = xTaskGetTickCount();
        if (now - last_render >= pdMS_TO_TICKS(250)) {
            render_ui();
            last_render = now;
        }
    }
}
