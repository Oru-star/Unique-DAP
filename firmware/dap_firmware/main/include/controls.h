#pragma once
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>

enum class ControlEventType : uint8_t {
    Encoder1Step,
    Encoder1Click,
    Encoder2Step,
    Encoder2Click,
    Previous,
    PlayPause,
    Next
};

struct ControlEvent {
    ControlEventType type;
    int8_t delta;
};

esp_err_t controls_init(QueueHandle_t event_queue);
