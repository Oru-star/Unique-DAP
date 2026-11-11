#include "audio_player.h"
#include "board_pins.h"
#include "dac_es9038.h"

#include "driver/i2s_std.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "micro_flac/flac_decoder.h"

#include "freertos/semphr.h"

#include <algorithm>
#include <atomic>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <strings.h>

static const char *TAG = "audio_player";

namespace {
constexpr size_t COMMAND_QUEUE_LENGTH = 8;
constexpr size_t INPUT_BUFFER_SIZE = 16384;
constexpr size_t WAV_FRAMES_PER_BLOCK = 512;
constexpr size_t DAC_LOCK_SILENCE_FRAMES = 8192;
constexpr size_t I2S_DMA_DRAIN_FRAMES = 2048;
constexpr uint32_t MAX_SAMPLE_RATE = 96000;

enum class CommandType : uint8_t { Play, TogglePause, Stop };
struct PlayerCommand {
    CommandType type;
    char path[256];
};
struct PlaybackControl {
    bool paused = false;
    bool stop = false;
    bool restart = false;
    char next_path[256]{};
};
struct WavInfo {
    uint16_t channels = 0;
    uint16_t bits_per_sample = 0;
    uint16_t block_align = 0;
    uint32_t sample_rate = 0;
    uint32_t data_size = 0;
    long data_offset = 0;
};

QueueHandle_t s_command_queue = nullptr;
SemaphoreHandle_t s_status_mutex = nullptr;
AudioPlayerStatus s_status{};
i2s_chan_handle_t s_i2s_tx = nullptr;
bool s_i2s_enabled = false;
std::atomic<bool> s_stream_active{false};
bool s_user_muted = false; // guarded by s_status_mutex

void update_status(const AudioPlayerStatus &status)
{
    if (s_status_mutex == nullptr) return;
    if (xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(100)) != pdTRUE) return;
    s_status = status;
    xSemaphoreGive(s_status_mutex);
}

void update_status_fields(bool playing, bool paused, uint32_t rate, uint8_t bits,
                          uint32_t elapsed, uint32_t duration, const char *path,
                          const char *error)
{
    if (s_status_mutex == nullptr) return;
    if (xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(100)) != pdTRUE) return;
    s_status.playing = playing;
    s_status.paused = paused;
    s_status.sample_rate = rate;
    s_status.bit_depth = bits;
    s_status.elapsed_seconds = elapsed;
    s_status.duration_seconds = duration;
    if (path != nullptr) strlcpy(s_status.path, path, sizeof(s_status.path));
    if (error != nullptr) strlcpy(s_status.error, error, sizeof(s_status.error));
    xSemaphoreGive(s_status_mutex);
}

void update_pause_status(bool paused)
{
    if (s_status_mutex == nullptr) return;
    if (xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(100)) != pdTRUE) return;
    s_status.paused = paused;
    xSemaphoreGive(s_status_mutex);
}

bool get_user_muted()
{
    if (s_status_mutex == nullptr) return false;
    if (xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(100)) != pdTRUE) return true;
    const bool muted = s_user_muted;
    xSemaphoreGive(s_status_mutex);
    return muted;
}

void apply_output_mute(bool paused)
{
    (void)es9038_set_mute(paused || get_user_muted());
}

esp_err_t configure_i2s(uint32_t sample_rate)
{
    if (sample_rate < 8000 || sample_rate > MAX_SAMPLE_RATE) return ESP_ERR_NOT_SUPPORTED;

    i2s_std_clk_config_t clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate);
    clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;

    if (s_i2s_tx == nullptr) {
        i2s_chan_config_t channel_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
        channel_cfg.dma_desc_num = 8;
        channel_cfg.dma_frame_num = 256;
        ESP_RETURN_ON_ERROR(i2s_new_channel(&channel_cfg, &s_i2s_tx, nullptr), TAG,
                            "I2S TX channel create failed");

        i2s_std_config_t std_cfg{};
        std_cfg.clk_cfg = clk_cfg;
        std_cfg.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO);
        std_cfg.gpio_cfg.mclk = I2S_GPIO_UNUSED; // DAC XI is driven by the board oscillator
        std_cfg.gpio_cfg.bclk = board::I2S_BCLK;
        std_cfg.gpio_cfg.ws = board::I2S_LRCK;
        std_cfg.gpio_cfg.dout = board::I2S_DOUT;
        std_cfg.gpio_cfg.din = I2S_GPIO_UNUSED;
        std_cfg.gpio_cfg.invert_flags.mclk_inv = false;
        std_cfg.gpio_cfg.invert_flags.bclk_inv = false;
        std_cfg.gpio_cfg.invert_flags.ws_inv = false;

        ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(s_i2s_tx, &std_cfg), TAG,
                            "I2S standard mode init failed");
        ESP_RETURN_ON_ERROR(i2s_channel_enable(s_i2s_tx), TAG, "I2S TX enable failed");
        s_i2s_enabled = true;
        ESP_LOGI(TAG, "I2S TX configured: %lu Hz, 32-bit stereo, no MCU MCLK",
                 static_cast<unsigned long>(sample_rate));
        return ESP_OK;
    }

    if (s_i2s_enabled) {
        ESP_RETURN_ON_ERROR(i2s_channel_disable(s_i2s_tx), TAG, "I2S TX disable failed for rate change");
        s_i2s_enabled = false;
    }
    ESP_RETURN_ON_ERROR(i2s_channel_reconfig_std_clock(s_i2s_tx, &clk_cfg), TAG,
                        "I2S sample-rate reconfiguration failed");
    ESP_RETURN_ON_ERROR(i2s_channel_enable(s_i2s_tx), TAG, "I2S TX re-enable failed");
    s_i2s_enabled = true;
    return ESP_OK;
}

esp_err_t write_pcm(const int32_t *samples, size_t sample_count)
{
    if (s_i2s_tx == nullptr || !s_i2s_enabled || samples == nullptr || sample_count == 0)
        return ESP_ERR_INVALID_STATE;

    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(samples);
    size_t remaining = sample_count * sizeof(int32_t);
    while (remaining > 0) {
        size_t bytes_written = 0;
        const esp_err_t err = i2s_channel_write(s_i2s_tx, bytes, remaining,
                                               &bytes_written, portMAX_DELAY);
        if (err != ESP_OK) return err;
        if (bytes_written == 0 || bytes_written > remaining) return ESP_ERR_INVALID_STATE;
        bytes += bytes_written;
        remaining -= bytes_written;
    }
    return ESP_OK;
}

esp_err_t start_output(uint32_t sample_rate)
{
    (void)es9038_set_mute(true);
    s_stream_active.store(false);

    esp_err_t err = configure_i2s(sample_rate);
    if (err != ESP_OK) return err;

    // Feed more than the ES9038Q2M default DPLL lock window (5,461 frame
    // edges) while muted. Then allow the I2S DMA pipeline to drain before
    // unmuting; this avoids an audible transient during clock acquisition.
    int32_t silence[WAV_FRAMES_PER_BLOCK * 2]{};
    constexpr size_t silence_blocks = DAC_LOCK_SILENCE_FRAMES / WAV_FRAMES_PER_BLOCK;
    for (size_t i = 0; i < silence_blocks; ++i) {
        err = write_pcm(silence, WAV_FRAMES_PER_BLOCK * 2);
        if (err != ESP_OK) return err;
    }

    const uint32_t drain_ms = (static_cast<uint32_t>(I2S_DMA_DRAIN_FRAMES) * 1000U +
                               sample_rate - 1U) / sample_rate + 3U;
    vTaskDelay(pdMS_TO_TICKS(drain_ms));
    s_stream_active.store(true);
    apply_output_mute(false);
    return ESP_OK;
}

void stop_output()
{
    (void)es9038_set_mute(true);
    s_stream_active.store(false);
}

void handle_command(const PlayerCommand &command, PlaybackControl &control)
{
    switch (command.type) {
        case CommandType::Play:
            strlcpy(control.next_path, command.path, sizeof(control.next_path));
            control.restart = true;
            control.stop = true;
            break;
        case CommandType::Stop:
            control.stop = true;
            control.restart = false;
            break;
        case CommandType::TogglePause:
            if (!control.stop) {
                control.paused = !control.paused;
                update_pause_status(control.paused);
                apply_output_mute(control.paused);
            }
            break;
    }
}

void service_commands(PlaybackControl &control)
{
    PlayerCommand command{};
    while (!control.stop && xQueueReceive(s_command_queue, &command, 0) == pdTRUE) {
        handle_command(command, control);
    }
    while (control.paused && !control.stop) {
        if (xQueueReceive(s_command_queue, &command, pdMS_TO_TICKS(100)) == pdTRUE)
            handle_command(command, control);
    }
}

int32_t wav_sample_to_32(const uint8_t *p, uint16_t bits)
{
    switch (bits) {
        case 8:
            return (static_cast<int32_t>(p[0]) - 128) * 16777216;
        case 16: {
            const int16_t value = static_cast<int16_t>(static_cast<uint16_t>(p[0]) |
                                                       (static_cast<uint16_t>(p[1]) << 8));
            return static_cast<int32_t>(value) * 65536;
        }
        case 24: {
            int32_t value = static_cast<int32_t>(p[0]) |
                            (static_cast<int32_t>(p[1]) << 8) |
                            (static_cast<int32_t>(p[2]) << 16);
            if ((value & 0x00800000) != 0) value |= static_cast<int32_t>(0xFF000000);
            return value * 256;
        }
        case 32: {
            const uint32_t raw = static_cast<uint32_t>(p[0]) |
                                 (static_cast<uint32_t>(p[1]) << 8) |
                                 (static_cast<uint32_t>(p[2]) << 16) |
                                 (static_cast<uint32_t>(p[3]) << 24);
            return static_cast<int32_t>(raw);
        }
        default:
            return 0;
    }
}

bool parse_wav(FILE *file, WavInfo &info)
{
    uint8_t riff_header[12]{};
    if (fread(riff_header, 1, sizeof(riff_header), file) != sizeof(riff_header)) return false;
    if (memcmp(riff_header, "RIFF", 4) != 0 || memcmp(riff_header + 8, "WAVE", 4) != 0)
        return false;

    bool have_fmt = false;
    bool have_data = false;
    while (!feof(file)) {
        uint8_t chunk_header[8]{};
        if (fread(chunk_header, 1, sizeof(chunk_header), file) != sizeof(chunk_header)) break;
        const uint32_t chunk_size = static_cast<uint32_t>(chunk_header[4]) |
                                    (static_cast<uint32_t>(chunk_header[5]) << 8) |
                                    (static_cast<uint32_t>(chunk_header[6]) << 16) |
                                    (static_cast<uint32_t>(chunk_header[7]) << 24);
        const long payload_offset = ftell(file);
        if (payload_offset < 0) return false;

        if (memcmp(chunk_header, "fmt ", 4) == 0) {
            uint8_t fmt[40]{};
            const size_t to_read = std::min<size_t>(chunk_size, sizeof(fmt));
            if (to_read < 16 || fread(fmt, 1, to_read, file) != to_read) return false;
            const uint16_t format_tag = static_cast<uint16_t>(fmt[0] | (fmt[1] << 8));
            info.channels = static_cast<uint16_t>(fmt[2] | (fmt[3] << 8));
            info.sample_rate = static_cast<uint32_t>(fmt[4]) |
                               (static_cast<uint32_t>(fmt[5]) << 8) |
                               (static_cast<uint32_t>(fmt[6]) << 16) |
                               (static_cast<uint32_t>(fmt[7]) << 24);
            info.block_align = static_cast<uint16_t>(fmt[12] | (fmt[13] << 8));
            info.bits_per_sample = static_cast<uint16_t>(fmt[14] | (fmt[15] << 8));
            if (format_tag != 1) return false; // PCM only; WAVE_FORMAT_EXTENSIBLE is not enabled yet
            have_fmt = true;
        } else if (memcmp(chunk_header, "data", 4) == 0) {
            info.data_offset = payload_offset;
            info.data_size = chunk_size;
            have_data = true;
        }

        const uint64_t next_offset = static_cast<uint64_t>(payload_offset) + chunk_size + (chunk_size & 1U);
        if (next_offset > static_cast<uint64_t>(LONG_MAX)) return false;
        if (fseek(file, static_cast<long>(next_offset), SEEK_SET) != 0) return false;
        if (have_fmt && have_data) break;
    }

    if (!have_fmt || !have_data || info.channels < 1 || info.channels > 2 ||
        info.sample_rate < 8000 || info.sample_rate > MAX_SAMPLE_RATE ||
        (info.bits_per_sample != 8 && info.bits_per_sample != 16 &&
         info.bits_per_sample != 24 && info.bits_per_sample != 32)) return false;

    const uint16_t expected_align = static_cast<uint16_t>(info.channels * (info.bits_per_sample / 8));
    if (info.block_align != expected_align || info.block_align == 0 ||
        info.data_size < info.block_align || (info.data_size % info.block_align) != 0) return false;
    return fseek(file, info.data_offset, SEEK_SET) == 0;
}

bool process_wav(FILE *file, const char *path, PlaybackControl &control)
{
    WavInfo info{};
    if (!parse_wav(file, info)) {
        update_status_fields(false, false, 0, 0, 0, 0, path, "Unsupported/invalid WAV");
        return false;
    }
    const uint64_t total_frames = info.data_size / info.block_align;
    const uint32_t duration = static_cast<uint32_t>(total_frames / info.sample_rate);

    esp_err_t err = start_output(info.sample_rate);
    if (err != ESP_OK) {
        stop_output();
        update_status_fields(false, false, info.sample_rate, static_cast<uint8_t>(info.bits_per_sample),
                             0, duration, path, "I2S start/clock lock failed");
        return false;
    }
    update_status_fields(true, false, info.sample_rate, static_cast<uint8_t>(info.bits_per_sample),
                         0, duration, path, "");

    uint8_t raw[WAV_FRAMES_PER_BLOCK * 2 * 4]{};
    int32_t stereo[WAV_FRAMES_PER_BLOCK * 2]{};
    uint64_t frames_played = 0;
    uint32_t last_display_second = UINT32_MAX;

    while (!control.stop) {
        service_commands(control);
        if (control.stop) break;

        const size_t frames_read = fread(raw, info.block_align, WAV_FRAMES_PER_BLOCK, file);
        if (frames_read == 0) break;

        for (size_t frame = 0; frame < frames_read; ++frame) {
            const uint8_t *frame_ptr = raw + frame * info.block_align;
            const size_t bytes_per_sample = info.bits_per_sample / 8;
            const int32_t left = wav_sample_to_32(frame_ptr, info.bits_per_sample);
            const int32_t right = (info.channels == 2)
                ? wav_sample_to_32(frame_ptr + bytes_per_sample, info.bits_per_sample)
                : left;
            stereo[frame * 2] = left;
            stereo[frame * 2 + 1] = right;
        }

        err = write_pcm(stereo, frames_read * 2);
        if (err != ESP_OK) {
            update_status_fields(false, false, info.sample_rate, static_cast<uint8_t>(info.bits_per_sample),
                                 static_cast<uint32_t>(frames_played / info.sample_rate), duration,
                                 path, "I2S write error");
            stop_output();
            return false;
        }
        frames_played += frames_read;
        const uint32_t elapsed = static_cast<uint32_t>(frames_played / info.sample_rate);
        if (elapsed != last_display_second) {
            update_status_fields(true, control.paused, info.sample_rate,
                                 static_cast<uint8_t>(info.bits_per_sample), elapsed,
                                 duration, path, "");
            last_display_second = elapsed;
        }
    }
    stop_output();
    if (ferror(file)) {
        update_status_fields(false, false, info.sample_rate, static_cast<uint8_t>(info.bits_per_sample),
                             static_cast<uint32_t>(frames_played / info.sample_rate), duration,
                             path, "microSD read error");
        return false;
    }
    if (!control.stop && frames_played < total_frames) {
        update_status_fields(false, false, info.sample_rate, static_cast<uint8_t>(info.bits_per_sample),
                             static_cast<uint32_t>(frames_played / info.sample_rate), duration,
                             path, "Truncated WAV data chunk");
        return false;
    }
    return true;
}

bool process_flac(FILE *file, const char *path, PlaybackControl &control)
{
    using namespace micro_flac;
    FLACDecoder decoder;
    decoder.set_crc_check_enabled(true);

    uint8_t *input = static_cast<uint8_t *>(heap_caps_malloc(
        INPUT_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (input == nullptr)
        input = static_cast<uint8_t *>(heap_caps_malloc(INPUT_BUFFER_SIZE, MALLOC_CAP_8BIT));
    if (input == nullptr) {
        update_status_fields(false, false, 0, 0, 0, 0, path, "FLAC input buffer allocation failed");
        return false;
    }
    size_t begin = 0;
    size_t end = fread(input, 1, INPUT_BUFFER_SIZE, file);
    bool eof = (end == 0);
    bool header_ready = false;
    int32_t *decoded = nullptr;
    int32_t *stereo = nullptr;
    size_t output_capacity = 0;
    uint8_t channels = 0;
    uint8_t bit_depth = 0;
    uint32_t sample_rate = 0;
    uint64_t total_samples = 0;
    uint64_t frames_played = 0;
    uint32_t duration = 0;
    uint32_t last_display_second = UINT32_MAX;
    bool success = true;

    while (!control.stop) {
        service_commands(control);
        if (control.stop) break;

        if (begin == end && !eof) {
            begin = 0;
            end = fread(input, 1, INPUT_BUFFER_SIZE, file);
            if (end == 0) eof = true;
        }
        // Even with zero bytes left, call decode() once with a non-null input
        // pointer. The decoder can then report END_OF_STREAM versus a truncated
        // frame (NEED_MORE_DATA).
        size_t consumed = 0;
        size_t samples_decoded = 0;
        const auto result = decoder.decode(input + begin, end - begin,
                                           decoded, output_capacity,
                                           consumed, samples_decoded);
        begin += consumed;

        if (consumed == 0 && samples_decoded == 0 &&
            result != FLAC_DECODER_HEADER_READY && result != FLAC_DECODER_NEED_MORE_DATA &&
            result != FLAC_DECODER_END_OF_STREAM) {
            success = false;
            update_status_fields(false, false, sample_rate, bit_depth, 0, duration,
                                 path, "FLAC decoder made no progress");
            break;
        }

        if (result == FLAC_DECODER_HEADER_READY) {
            const auto &info = decoder.get_stream_info();
            channels = static_cast<uint8_t>(info.num_channels());
            bit_depth = static_cast<uint8_t>(info.bits_per_sample());
            sample_rate = info.sample_rate();
            total_samples = info.total_samples_per_channel();
            if (!info.is_valid() || (channels != 1 && channels != 2) ||
                (bit_depth != 16 && bit_depth != 24 && bit_depth != 32) ||
                sample_rate < 8000 || sample_rate > MAX_SAMPLE_RATE) {
                success = false;
                update_status_fields(false, false, sample_rate, bit_depth, 0, 0,
                                     path, "Unsupported FLAC format");
                break;
            }
            duration = total_samples > 0
                ? static_cast<uint32_t>(total_samples / sample_rate) : 0;
            output_capacity = decoder.get_output_buffer_size_samples();
            if (output_capacity == 0 || output_capacity > 262144) {
                success = false;
                update_status_fields(false, false, sample_rate, bit_depth, 0, duration,
                                     path, "FLAC block size invalid");
                break;
            }
            decoded = static_cast<int32_t *>(heap_caps_malloc(output_capacity * sizeof(int32_t),
                                                               MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
            if (decoded == nullptr)
                decoded = static_cast<int32_t *>(heap_caps_malloc(output_capacity * sizeof(int32_t),
                                                                   MALLOC_CAP_8BIT));
            stereo = static_cast<int32_t *>(heap_caps_malloc(output_capacity * sizeof(int32_t) * 2,
                                                              MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
            if (stereo == nullptr)
                stereo = static_cast<int32_t *>(heap_caps_malloc(output_capacity * sizeof(int32_t) * 2,
                                                                  MALLOC_CAP_8BIT));
            if (decoded == nullptr || stereo == nullptr) {
                success = false;
                update_status_fields(false, false, sample_rate, bit_depth, 0, duration,
                                     path, "FLAC buffer allocation failed");
                break;
            }

            esp_err_t err = start_output(sample_rate);
            if (err != ESP_OK) {
                success = false;
                stop_output();
                update_status_fields(false, false, sample_rate, bit_depth, 0, duration,
                                     path, "I2S start/clock lock failed");
                break;
            }
            update_status_fields(true, false, sample_rate, bit_depth, 0, duration, path, "");
            header_ready = true;
        } else if (result == FLAC_DECODER_SUCCESS) {
            if (!header_ready) {
                success = false;
                break;
            }
            if (samples_decoded > 0) {
                if ((samples_decoded % channels) != 0 || samples_decoded > output_capacity) {
                    success = false;
                    update_status_fields(false, false, sample_rate, bit_depth,
                                         static_cast<uint32_t>(frames_played / sample_rate), duration,
                                         path, "Invalid FLAC decoder output size");
                    break;
                }
                const size_t frame_count = samples_decoded / channels;
                if (channels == 2) {
                    memcpy(stereo, decoded, samples_decoded * sizeof(int32_t));
                } else {
                    for (size_t i = 0; i < frame_count; ++i) {
                        stereo[i * 2] = decoded[i];
                        stereo[i * 2 + 1] = decoded[i];
                    }
                }
                const esp_err_t err = write_pcm(stereo, frame_count * 2);
                if (err != ESP_OK) {
                    success = false;
                    update_status_fields(false, false, sample_rate, bit_depth,
                                         static_cast<uint32_t>(frames_played / sample_rate), duration,
                                         path, "I2S write error");
                    break;
                }
                frames_played += frame_count;
                const uint32_t elapsed = static_cast<uint32_t>(frames_played / sample_rate);
                if (elapsed != last_display_second) {
                    update_status_fields(true, control.paused, sample_rate, bit_depth,
                                         elapsed, duration, path, "");
                    last_display_second = elapsed;
                }
            }
        } else if (result == FLAC_DECODER_NEED_MORE_DATA) {
            if (begin < end) {
                const size_t remaining = end - begin;
                memmove(input, input + begin, remaining);
                begin = 0;
                end = remaining;
            } else {
                begin = 0;
                end = 0;
            }
            if (eof) {
                if (header_ready && total_samples > 0 && frames_played >= total_samples) break;
                success = false;
                update_status_fields(false, false, sample_rate, bit_depth,
                                     static_cast<uint32_t>(frames_played / (sample_rate == 0 ? 1 : sample_rate)),
                                     duration, path, "Truncated FLAC stream");
                break;
            }
            if (end >= INPUT_BUFFER_SIZE) {
                success = false;
                update_status_fields(false, false, sample_rate, bit_depth, 0, duration,
                                     path, "FLAC input frame too large");
                break;
            }
            const size_t read = fread(input + end, 1, INPUT_BUFFER_SIZE - end, file);
            end += read;
            if (read == 0) eof = true;
        } else if (result == FLAC_DECODER_END_OF_STREAM) {
            if (!header_ready || (total_samples > 0 && frames_played < total_samples)) {
                success = false;
                update_status_fields(false, false, sample_rate, bit_depth,
                                     sample_rate == 0 ? 0 : static_cast<uint32_t>(frames_played / sample_rate),
                                     duration, path, "Incomplete FLAC stream");
            }
            break;
        } else {
            success = false;
            update_status_fields(false, false, sample_rate, bit_depth,
                                 sample_rate == 0 ? 0 : static_cast<uint32_t>(frames_played / sample_rate),
                                 duration, path, "FLAC decode error");
            break;
        }
    }

    if (!header_ready) {
        success = false;
        update_status_fields(false, false, 0, 0, 0, 0, path, "Invalid or empty FLAC file");
    }
    stop_output();
    if (ferror(file)) {
        success = false;
        update_status_fields(false, false, sample_rate, bit_depth,
                             sample_rate == 0 ? 0 : static_cast<uint32_t>(frames_played / sample_rate),
                             duration, path, "microSD read error");
    }
    if (decoded != nullptr) heap_caps_free(decoded);
    if (stereo != nullptr) heap_caps_free(stereo);
    heap_caps_free(input);
    return success;
}

void player_task(void *)
{
    PlayerCommand command{};
    char current_path[256]{};

    while (true) {
        if (current_path[0] == '\0') {
            if (xQueueReceive(s_command_queue, &command, portMAX_DELAY) != pdTRUE) continue;
            if (command.type != CommandType::Play) continue;
            strlcpy(current_path, command.path, sizeof(current_path));
        }

        FILE *file = fopen(current_path, "rb");
        if (file == nullptr) {
            update_status_fields(false, false, 0, 0, 0, 0, current_path, "Could not open file");
            current_path[0] = '\0';
            continue;
        }

        PlaybackControl control{};
        update_status_fields(true, false, 0, 0, 0, 0, current_path, "");
        const char *dot = strrchr(current_path, '.');
        bool ok = false;
        if (dot != nullptr && strcasecmp(dot, ".wav") == 0)
            ok = process_wav(file, current_path, control);
        else if (dot != nullptr && strcasecmp(dot, ".flac") == 0)
            ok = process_flac(file, current_path, control);
        else
            update_status_fields(false, false, 0, 0, 0, 0, current_path, "Unsupported file type");

        fclose(file);
        if (!control.restart) {
            (void)es9038_set_mute(true);
            AudioPlayerStatus final_status{};
            if (s_status_mutex != nullptr && xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                final_status = s_status;
                xSemaphoreGive(s_status_mutex);
            }
            final_status.playing = false;
            final_status.paused = false;
            if (!ok && final_status.error[0] == '\0')
                strlcpy(final_status.error, "Playback stopped", sizeof(final_status.error));
            update_status(final_status);
            current_path[0] = '\0';
        } else {
            strlcpy(current_path, control.next_path, sizeof(current_path));
        }
    }
}
}

esp_err_t audio_player_init()
{
    s_status_mutex = xSemaphoreCreateMutex();
    if (s_status_mutex == nullptr) return ESP_ERR_NO_MEM;
    s_command_queue = xQueueCreate(COMMAND_QUEUE_LENGTH, sizeof(PlayerCommand));
    if (s_command_queue == nullptr) return ESP_ERR_NO_MEM;

    BaseType_t result = xTaskCreatePinnedToCore(player_task, "dap_audio", 24576,
                                                nullptr, 6, nullptr, 1);
    if (result != pdPASS) return ESP_ERR_NO_MEM;
    return ESP_OK;
}

esp_err_t audio_player_play(const char *path)
{
    if (path == nullptr || path[0] == '\0' || s_command_queue == nullptr) return ESP_ERR_INVALID_ARG;
    if (strlen(path) >= 256U) return ESP_ERR_INVALID_SIZE;
    PlayerCommand command{};
    command.type = CommandType::Play;
    strlcpy(command.path, path, sizeof(command.path));
    return xQueueSend(s_command_queue, &command, pdMS_TO_TICKS(100)) == pdTRUE
        ? ESP_OK : ESP_ERR_TIMEOUT;
}

esp_err_t audio_player_toggle_pause()
{
    if (s_command_queue == nullptr) return ESP_ERR_INVALID_STATE;
    PlayerCommand command{};
    command.type = CommandType::TogglePause;
    return xQueueSend(s_command_queue, &command, pdMS_TO_TICKS(100)) == pdTRUE
        ? ESP_OK : ESP_ERR_TIMEOUT;
}

esp_err_t audio_player_set_user_mute(bool muted)
{
    if (s_status_mutex == nullptr) return ESP_ERR_INVALID_STATE;
    if (xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(100)) != pdTRUE) return ESP_ERR_TIMEOUT;
    s_user_muted = muted;
    const bool paused = s_status.paused;
    xSemaphoreGive(s_status_mutex);

    // Do not unmute an idle DAC. start_output() owns the playback transition.
    if (s_stream_active.load()) return es9038_set_mute(muted || paused);
    return ESP_OK;
}

esp_err_t audio_player_stop()
{
    if (s_command_queue == nullptr) return ESP_ERR_INVALID_STATE;
    PlayerCommand command{};
    command.type = CommandType::Stop;
    return xQueueSend(s_command_queue, &command, pdMS_TO_TICKS(100)) == pdTRUE
        ? ESP_OK : ESP_ERR_TIMEOUT;
}

void audio_player_get_status(AudioPlayerStatus *out)
{
    if (out == nullptr) return;
    if (s_status_mutex == nullptr || xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        memset(out, 0, sizeof(*out));
        return;
    }
    *out = s_status;
    xSemaphoreGive(s_status_mutex);
}
