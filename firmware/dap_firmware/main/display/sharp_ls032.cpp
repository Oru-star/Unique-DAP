#include "sharp_ls032.h"
#include "board_pins.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>

static const char *TAG = "sharp_lcd";
static spi_device_handle_t s_lcd = nullptr;
static esp_timer_handle_t s_com_timer = nullptr;
static bool s_com_level = false;
static bool s_ready = false;
static uint8_t s_framebuffer[board::LCD_FRAME_BYTES];
static uint8_t s_previous[board::LCD_FRAME_BYTES];

namespace {
constexpr size_t LINE_TRANSFER_BYTES = 2 + board::LCD_ROW_BYTES + 2;

// The LS032B7DD02 consumes serial bits LSB-first. ESP-IDF SPI transmits MSB-first
// by default, so reverse each byte before passing it to the SPI peripheral.
uint8_t reverse_bits(uint8_t value)
{
    value = static_cast<uint8_t>(((value & 0x55U) << 1) | ((value >> 1) & 0x55U));
    value = static_cast<uint8_t>(((value & 0x33U) << 2) | ((value >> 2) & 0x33U));
    return static_cast<uint8_t>((value << 4) | (value >> 4));
}

// 5x7 column font, bit 0 is the top pixel. Intentionally limited to the
// ASCII characters used by filenames and the status UI.
const uint8_t *glyph(char c)
{
    static const uint8_t blank[5] = {0, 0, 0, 0, 0};
    static const uint8_t digits[10][5] = {
        {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
        {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
        {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
        {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
        {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E}
    };
    static const uint8_t letters[26][5] = {
        {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36},
        {0x3E,0x41,0x41,0x41,0x22}, {0x7F,0x41,0x41,0x22,0x1C},
        {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
        {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F},
        {0x00,0x41,0x7F,0x41,0x00}, {0x20,0x40,0x41,0x3F,0x01},
        {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
        {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F},
        {0x3E,0x41,0x41,0x41,0x3E}, {0x7F,0x09,0x09,0x09,0x06},
        {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
        {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01},
        {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F},
        {0x3F,0x40,0x38,0x40,0x3F}, {0x63,0x14,0x08,0x14,0x63},
        {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
    };
    static const uint8_t dash[5] = {0x08,0x08,0x08,0x08,0x08};
    static const uint8_t underscore[5] = {0x40,0x40,0x40,0x40,0x40};
    static const uint8_t period[5] = {0x00,0x60,0x60,0x00,0x00};
    static const uint8_t colon[5] = {0x00,0x36,0x36,0x00,0x00};
    static const uint8_t slash[5] = {0x20,0x10,0x08,0x04,0x02};
    static const uint8_t percent[5] = {0x63,0x13,0x08,0x64,0x63};
    static const uint8_t plus[5] = {0x08,0x08,0x3E,0x08,0x08};

    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    if (c >= '0' && c <= '9') return digits[c - '0'];
    if (c >= 'A' && c <= 'Z') return letters[c - 'A'];
    switch (c) {
        case '-': return dash;
        case '_': return underscore;
        case '.': return period;
        case ':': return colon;
        case '/': return slash;
        case '%': return percent;
        case '+': return plus;
        default: return blank;
    }
}

void pixel(int x, int y, bool black)
{
    if (x < 0 || x >= board::LCD_WIDTH || y < 0 || y >= board::LCD_HEIGHT) return;
    const size_t offset = static_cast<size_t>(y) * board::LCD_ROW_BYTES +
                          static_cast<size_t>(x / 8);
    // Bit 0 is the first pixel on the wire (the panel is LSB-first).
    const uint8_t mask = static_cast<uint8_t>(1U << (x & 7));
    if (black) s_framebuffer[offset] &= static_cast<uint8_t>(~mask); // 0 = black
    else s_framebuffer[offset] |= mask;                              // 1 = white
}

void fill_white()
{
    memset(s_framebuffer, 0xFF, sizeof(s_framebuffer));
}

void line(int x0, int y0, int x1, int y1)
{
    const int dx = abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    while (true) {
        pixel(x0, y0, true);
        if (x0 == x1 && y0 == y1) break;
        const int e2 = 2 * error;
        if (e2 >= dy) { error += dy; x0 += sx; }
        if (e2 <= dx) { error += dx; y0 += sy; }
    }
}

void rect(int x, int y, int w, int h)
{
    line(x, y, x + w - 1, y);
    line(x, y + h - 1, x + w - 1, y + h - 1);
    line(x, y, x, y + h - 1);
    line(x + w - 1, y, x + w - 1, y + h - 1);
}

int draw_text(int x, int y, const char *text, int scale = 2, int max_width = board::LCD_WIDTH - 24)
{
    if (text == nullptr) return x;
    const int start_x = x;
    while (*text != '\0' && x - start_x < max_width) {
        const uint8_t *columns = glyph(*text++);
        for (int col = 0; col < 5; ++col) {
            for (int row = 0; row < 7; ++row) {
                if ((columns[col] & (1U << row)) == 0) continue;
                for (int sx = 0; sx < scale; ++sx)
                    for (int sy = 0; sy < scale; ++sy)
                        pixel(x + col * scale + sx, y + row * scale + sy, true);
            }
        }
        x += 6 * scale;
    }
    return x;
}

void com_toggle_callback(void *)
{
    s_com_level = !s_com_level;
    gpio_set_level(board::LCD_EXTCOMIN, s_com_level ? 1 : 0);
}

esp_err_t transmit_bytes(const uint8_t *data, size_t length)
{
    spi_transaction_t transaction{};
    transaction.length = length * 8;
    transaction.tx_buffer = data;
    gpio_set_level(board::LCD_SCS, 1);
    esp_err_t err = spi_device_polling_transmit(s_lcd, &transaction);
    gpio_set_level(board::LCD_SCS, 0);
    return err;
}

esp_err_t clear_panel()
{
    // Logical LSB-first command: M0=0, M1=0, M2=1, followed by 13 dummy
    // clocks. Bit-reversed bytes are {0x20, 0x00} for MSB-first SPI.
    const uint8_t command[2] = {reverse_bits(0x04), reverse_bits(0x00)};
    return transmit_bytes(command, sizeof(command));
}

esp_err_t flush_framebuffer()
{
    uint8_t tx[LINE_TRANSFER_BYTES]{};
    for (int y = 0; y < board::LCD_HEIGHT; ++y) {
        const size_t row_offset = static_cast<size_t>(y) * board::LCD_ROW_BYTES;
        if (memcmp(&s_framebuffer[row_offset], &s_previous[row_offset], board::LCD_ROW_BYTES) == 0)
            continue;

        // Wire order is M0, M1, M2, 3 dummy bits, then AG0..AG9. Build
        // the 10-bit address LSB-first across the two logical bytes, reverse
        // each byte, then let the ESP32 SPI peripheral send MSB-first.
        const uint16_t address = static_cast<uint16_t>(y + 1); // panel addresses are 1..536
        const uint8_t logical_header0 = static_cast<uint8_t>(0x01U | ((address & 0x03U) << 6));
        const uint8_t logical_header1 = static_cast<uint8_t>((address >> 2) & 0xFFU);
        tx[0] = reverse_bits(logical_header0);
        tx[1] = reverse_bits(logical_header1);
        for (size_t i = 0; i < board::LCD_ROW_BYTES; ++i) {
            tx[2 + i] = reverse_bits(s_framebuffer[row_offset + i]);
        }
        tx[2 + board::LCD_ROW_BYTES] = 0x00;
        tx[3 + board::LCD_ROW_BYTES] = 0x00;

        esp_err_t err = transmit_bytes(tx, sizeof(tx));
        if (err != ESP_OK) return err;
        memcpy(&s_previous[row_offset], &s_framebuffer[row_offset], board::LCD_ROW_BYTES);
    }
    return ESP_OK;
}
}

esp_err_t sharp_display_init()
{
    gpio_config_t output_cfg{};
    output_cfg.pin_bit_mask = (1ULL << board::LCD_SCS) |
                              (1ULL << board::LCD_EXTCOMIN) |
                              (1ULL << board::LCD_DISP);
    output_cfg.mode = GPIO_MODE_OUTPUT;
    output_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    output_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    output_cfg.intr_type = GPIO_INTR_DISABLE;
    ESP_RETURN_ON_ERROR(gpio_config(&output_cfg), TAG, "LCD GPIO config failed");
    gpio_set_level(board::LCD_SCS, 0);
    gpio_set_level(board::LCD_EXTCOMIN, 0);
    gpio_set_level(board::LCD_DISP, 0);

    spi_bus_config_t bus_cfg{};
    bus_cfg.mosi_io_num = board::LCD_SI;
    bus_cfg.miso_io_num = -1;
    bus_cfg.sclk_io_num = board::LCD_SCLK;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = LINE_TRANSFER_BYTES;
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_DISABLED), TAG,
                        "LCD SPI bus init failed");

    spi_device_interface_config_t dev_cfg{};
    dev_cfg.clock_speed_hz = 1000000; // Sharp reference condition: 1 MHz
    dev_cfg.mode = 0;
    dev_cfg.spics_io_num = -1;        // SCS is controlled as a GPIO
    dev_cfg.queue_size = 1;
    ESP_RETURN_ON_ERROR(spi_bus_add_device(SPI3_HOST, &dev_cfg, &s_lcd), TAG,
                        "LCD SPI device add failed");

    fill_white();
    memset(s_previous, 0x00, sizeof(s_previous));
    vTaskDelay(pdMS_TO_TICKS(10)); // 5 V rail has already been enabled by board_power_init()
    ESP_RETURN_ON_ERROR(clear_panel(), TAG, "LCD all-clear command failed");

    // DISP is raised only after initialization; leave at least 30 us before SCS activity.
    gpio_set_level(board::LCD_DISP, 1);
    esp_rom_delay_us(40);

    esp_timer_create_args_t timer_args{};
    timer_args.callback = com_toggle_callback;
    timer_args.arg = nullptr;
    timer_args.dispatch_method = ESP_TIMER_TASK;
    timer_args.name = "lcd_com";
    ESP_RETURN_ON_ERROR(esp_timer_create(&timer_args, &s_com_timer), TAG, "COM timer create failed");
    // Toggle every 500 ms -> EXTCOMIN square wave of 1 Hz.
    ESP_RETURN_ON_ERROR(esp_timer_start_periodic(s_com_timer, 500000), TAG, "COM timer start failed");

    ESP_RETURN_ON_ERROR(flush_framebuffer(), TAG, "Initial LCD framebuffer flush failed");
    s_ready = true;
    ESP_LOGI(TAG, "Sharp LS032B7DD02 initialized: %dx%d, SPI 1 MHz, EXTCOMIN 1 Hz",
             board::LCD_WIDTH, board::LCD_HEIGHT);
    return ESP_OK;
}

void sharp_display_render(const DisplayState &state)
{
    if (!s_ready) return;
    fill_white();

    // Main header and separators.
    draw_text(18, 18, "HIFI PLAYER", 2);
    line(16, 48, 320, 48);
    draw_text(18, 66, "LIBRARY", 1);

    char count_text[40]{};
    snprintf(count_text, sizeof(count_text), "%u / %u",
             static_cast<unsigned>(state.track_count == 0 ? 0 : state.track_index + 1),
             static_cast<unsigned>(state.track_count));
    draw_text(245, 66, count_text, 1, 75);

    draw_text(18, 104, "SELECTED", 1);
    draw_text(18, 126, state.selected_track != nullptr ? state.selected_track : "NO TRACKS", 2, 300);
    line(16, 162, 320, 162);

    draw_text(18, 182, "NOW PLAYING", 1);
    draw_text(18, 208, state.playing_track != nullptr ? state.playing_track : "STOPPED", 2, 300);

    const char *play_state = state.playing ? (state.paused ? "PAUSED" : "PLAYING") : "STOPPED";
    draw_text(18, 256, play_state, 2);

    char format_text[48]{};
    if (state.sample_rate != 0 && state.bit_depth != 0)
        snprintf(format_text, sizeof(format_text), "%lu HZ  /  %u BIT",
                 static_cast<unsigned long>(state.sample_rate), static_cast<unsigned>(state.bit_depth));
    else
        strlcpy(format_text, "PCM OUTPUT IDLE", sizeof(format_text));
    draw_text(18, 304, format_text, 2);

    char time_text[48]{};
    snprintf(time_text, sizeof(time_text), "%02lu:%02lu / %02lu:%02lu",
             static_cast<unsigned long>(state.elapsed_seconds / 60),
             static_cast<unsigned long>(state.elapsed_seconds % 60),
             static_cast<unsigned long>(state.duration_seconds / 60),
             static_cast<unsigned long>(state.duration_seconds % 60));
    draw_text(18, 360, time_text, 2);

    rect(18, 390, 300, 18);
    if (state.duration_seconds > 0) {
        const uint32_t elapsed = std::min(state.elapsed_seconds, state.duration_seconds);
        const int width = static_cast<int>((static_cast<uint64_t>(elapsed) * 296U) /
                                           state.duration_seconds);
        if (width > 0) {
            for (int y = 394; y < 404; ++y)
                for (int x = 20; x < 20 + width; ++x)
                    pixel(x, y, true);
        }
    }

    char volume_text[32]{};
    snprintf(volume_text, sizeof(volume_text), "VOLUME  %u%%", static_cast<unsigned>(state.volume_percent));
    draw_text(18, 434, volume_text, 2);
    rect(18, 460, 300, 12);
    const int volume_width = static_cast<int>((state.volume_percent * 296U) / 100U);
    for (int y = 463; y < 469; ++y)
        for (int x = 20; x < 20 + volume_width; ++x)
            pixel(x, y, true);

    line(16, 492, 320, 492);
    draw_text(18, 506, "ENC1 SELECT  CLICK PLAY", 1, 300);

    if (state.message != nullptr && state.message[0] != '\0') {
        draw_text(18, 526, state.message, 1, 300);
    } else {
        draw_text(18, 526, "ENC2 VOLUME  PREV / NEXT", 1, 300);
    }

    const esp_err_t err = flush_framebuffer();
    if (err != ESP_OK) ESP_LOGE(TAG, "LCD update failed: %s", esp_err_to_name(err));
}
