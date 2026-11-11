#include "storage.h"
#include "board_pins.h"

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <strings.h>
#include <sys/stat.h>

static const char *TAG = "storage";
static constexpr char MOUNT_POINT[] = "/sdcard";
static char s_tracks[DAP_MAX_TRACKS][DAP_PATH_MAX]{};
static size_t s_track_count = 0;
static sdmmc_card_t *s_card = nullptr;
static bool s_mounted = false;

namespace {
bool is_audio_file(const char *name)
{
    const char *dot = strrchr(name, '.');
    if (dot == nullptr) return false;
    return strcasecmp(dot, ".flac") == 0 || strcasecmp(dot, ".wav") == 0;
}

void scan_directory(const char *path, unsigned depth)
{
    if (depth > 4 || s_track_count >= DAP_MAX_TRACKS) return;
    DIR *dir = opendir(path);
    if (dir == nullptr) return;

    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr && s_track_count < DAP_MAX_TRACKS) {
        if (entry->d_name[0] == '.') continue;
        char full_path[DAP_PATH_MAX]{};
        const int n = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (n <= 0 || static_cast<size_t>(n) >= sizeof(full_path)) continue;

        struct stat st{};
        if (stat(full_path, &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) {
            scan_directory(full_path, depth + 1);
        } else if (S_ISREG(st.st_mode) && is_audio_file(entry->d_name)) {
            strlcpy(s_tracks[s_track_count], full_path, sizeof(s_tracks[s_track_count]));
            ++s_track_count;
        }
    }
    closedir(dir);
}

int compare_track_paths(const void *a, const void *b)
{
    const auto *pa = static_cast<const char *>(a);
    const auto *pb = static_cast<const char *>(b);
    return strcasecmp(pa, pb);
}
}

esp_err_t storage_init()
{
    if (s_mounted) return ESP_OK;

    // DAT3 is used as chip select in SPI mode. Drive it high before the host
    // is initialized; add an external 10 kOhm pull-up on the PCB as well.
    gpio_config_t cs_cfg{};
    cs_cfg.pin_bit_mask = 1ULL << board::SD_CS;
    cs_cfg.mode = GPIO_MODE_OUTPUT;
    cs_cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    cs_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cs_cfg.intr_type = GPIO_INTR_DISABLE;
    ESP_RETURN_ON_ERROR(gpio_config(&cs_cfg), TAG, "SD CS GPIO config failed");
    ESP_RETURN_ON_ERROR(gpio_set_level(board::SD_CS, 1), TAG, "Could not idle SD CS high");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    host.max_freq_khz = 20000; // begin conservatively; raise after signal-integrity testing

    spi_bus_config_t bus_cfg{};
    bus_cfg.mosi_io_num = board::SD_MOSI;
    bus_cfg.miso_io_num = board::SD_MISO;
    bus_cfg.sclk_io_num = board::SD_SCLK;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = 4096;
    esp_err_t err = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SD SPI bus init failed: %s", esp_err_to_name(err));
        return err;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.host_id = host.slot;
    slot_config.gpio_cs = board::SD_CS;

    esp_vfs_fat_mount_config_t mount_config{};
    mount_config.format_if_mount_failed = false; // never format a user's music card automatically
    mount_config.max_files = 8;
    mount_config.allocation_unit_size = 16 * 1024;

    err = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &s_card);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "microSD mount failed: %s (use a FAT32-formatted card)", esp_err_to_name(err));
        spi_bus_free(host.slot);
        return err;
    }

    s_mounted = true;
    sdmmc_card_print_info(stdout, s_card);
    return storage_scan();
}

esp_err_t storage_scan()
{
    if (!s_mounted) return ESP_ERR_INVALID_STATE;
    s_track_count = 0;

    char music_dir[DAP_PATH_MAX]{};
    snprintf(music_dir, sizeof(music_dir), "%s/MUSIC", MOUNT_POINT);
    struct stat st{};
    const char *root = (stat(music_dir, &st) == 0 && S_ISDIR(st.st_mode)) ? music_dir : MOUNT_POINT;
    scan_directory(root, 0);
    qsort(s_tracks, s_track_count, sizeof(s_tracks[0]), compare_track_paths);

    ESP_LOGI(TAG, "Found %u WAV/FLAC tracks under %s", static_cast<unsigned>(s_track_count), root);
    return ESP_OK;
}

size_t storage_track_count()
{
    return s_track_count;
}

bool storage_get_track_path(size_t index, char *out, size_t out_size)
{
    if (out == nullptr || out_size == 0 || index >= s_track_count) return false;
    strlcpy(out, s_tracks[index], out_size);
    return true;
}

void storage_get_track_title(size_t index, char *out, size_t out_size)
{
    if (out == nullptr || out_size == 0) return;
    out[0] = '\0';
    if (index >= s_track_count) return;

    const char *base = strrchr(s_tracks[index], '/');
    base = (base == nullptr) ? s_tracks[index] : base + 1;
    strlcpy(out, base, out_size);
    char *dot = strrchr(out, '.');
    if (dot != nullptr) *dot = '\0';
}
