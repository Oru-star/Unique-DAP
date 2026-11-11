#include "controls.h"
#include "board_pins.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/task.h"

static const char *TAG = "controls";
static QueueHandle_t s_queue = nullptr;

namespace {
struct EncoderState {
    gpio_num_t a;
    gpio_num_t b;
    uint8_t previous;
    int8_t accumulator;
    ControlEventType event;
};

struct ButtonState {
    gpio_num_t pin;
    ControlEventType event;
    bool stable_pressed;
    bool candidate_pressed;
    TickType_t candidate_since;
};

// Gray-code transition table. One full detent is four legal transitions.
constexpr int8_t QUAD_TABLE[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

EncoderState s_encoders[2] = {
    {board::ENC1_A, board::ENC1_B, 0, 0, ControlEventType::Encoder1Step},
    {board::ENC2_A, board::ENC2_B, 0, 0, ControlEventType::Encoder2Step}
};

ButtonState s_buttons[5] = {
    {board::ENC1_SW, ControlEventType::Encoder1Click, false, false, 0},
    {board::ENC2_SW, ControlEventType::Encoder2Click, false, false, 0},
    {board::BTN_PREV, ControlEventType::Previous, false, false, 0},
    {board::BTN_PLAY, ControlEventType::PlayPause, false, false, 0},
    {board::BTN_NEXT, ControlEventType::Next, false, false, 0}
};

void send_event(ControlEventType type, int8_t delta = 0)
{
    if (s_queue == nullptr) return;
    const ControlEvent event{type, delta};
    (void)xQueueSend(s_queue, &event, 0);
}

void scan_controls_task(void *)
{
    constexpr TickType_t DEBOUNCE_TICKS = pdMS_TO_TICKS(25);
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        for (auto &enc : s_encoders) {
            const uint8_t current = static_cast<uint8_t>((gpio_get_level(enc.a) << 1) |
                                                         gpio_get_level(enc.b));
            const uint8_t transition = static_cast<uint8_t>((enc.previous << 2) | current);
            enc.accumulator = static_cast<int8_t>(enc.accumulator + QUAD_TABLE[transition]);
            enc.previous = current;

            if (enc.accumulator >= 4) {
                send_event(enc.event, 1);
                enc.accumulator = 0;
            } else if (enc.accumulator <= -4) {
                send_event(enc.event, -1);
                enc.accumulator = 0;
            }
        }

        const TickType_t now = xTaskGetTickCount();
        for (auto &button : s_buttons) {
            const bool pressed = gpio_get_level(button.pin) == 0;
            if (pressed != button.candidate_pressed) {
                button.candidate_pressed = pressed;
                button.candidate_since = now;
            } else if ((pressed != button.stable_pressed) &&
                       (now - button.candidate_since >= DEBOUNCE_TICKS)) {
                button.stable_pressed = pressed;
                if (pressed) send_event(button.event);
            }
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(2));
    }
}

esp_err_t configure_input(gpio_num_t pin)
{
    gpio_config_t cfg{};
    cfg.pin_bit_mask = 1ULL << pin;
    cfg.mode = GPIO_MODE_INPUT;
    cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_DISABLE;
    return gpio_config(&cfg);
}
}

esp_err_t controls_init(QueueHandle_t event_queue)
{
    if (event_queue == nullptr) return ESP_ERR_INVALID_ARG;
    s_queue = event_queue;

    for (const auto &enc : s_encoders) {
        ESP_RETURN_ON_ERROR(configure_input(enc.a), TAG, "Encoder A GPIO config failed");
        ESP_RETURN_ON_ERROR(configure_input(enc.b), TAG, "Encoder B GPIO config failed");
    }
    for (const auto &button : s_buttons) {
        ESP_RETURN_ON_ERROR(configure_input(button.pin), TAG, "Button GPIO config failed");
    }

    for (auto &enc : s_encoders) {
        enc.previous = static_cast<uint8_t>((gpio_get_level(enc.a) << 1) | gpio_get_level(enc.b));
    }

    BaseType_t result = xTaskCreatePinnedToCore(scan_controls_task, "dap_controls", 4096,
                                                nullptr, 5, nullptr, 0);
    if (result != pdPASS) return ESP_ERR_NO_MEM;
    ESP_LOGI(TAG, "Encoders and buttons initialized (2 ms scan, 25 ms debounce)");
    return ESP_OK;
}
