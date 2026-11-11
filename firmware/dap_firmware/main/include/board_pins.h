#pragma once

#include "driver/gpio.h"

// Pin assignments transcribed from the uploaded KiCad schematic (pages 4-6).
// GPIO3 and GPIO45 are ESP32-S3 strapping pins. External resistors must
// establish their boot-time states before the firmware configures them.
// GPIO7 is intentionally NOT used: the ES9038Q2M XI pin is driven by the
// separate external oscillator on the DAC sheet; GPIO7's DAC_MCLK label has
// no matching connection at the DAC.
namespace board {
constexpr gpio_num_t BOOST_EN       = GPIO_NUM_45;
constexpr gpio_num_t DAC_RESET_N    = GPIO_NUM_3;
constexpr gpio_num_t BQ_INT_N       = GPIO_NUM_1;

constexpr gpio_num_t I2C_SDA        = GPIO_NUM_8;
constexpr gpio_num_t I2C_SCL        = GPIO_NUM_9;

constexpr gpio_num_t I2S_BCLK       = GPIO_NUM_4;
constexpr gpio_num_t I2S_LRCK       = GPIO_NUM_5;
constexpr gpio_num_t I2S_DOUT       = GPIO_NUM_6;

constexpr gpio_num_t SD_CS          = GPIO_NUM_10;
constexpr gpio_num_t SD_MOSI        = GPIO_NUM_11;
constexpr gpio_num_t SD_SCLK        = GPIO_NUM_12;
constexpr gpio_num_t SD_MISO        = GPIO_NUM_13;

constexpr gpio_num_t LCD_SCLK       = GPIO_NUM_15;
constexpr gpio_num_t LCD_SI         = GPIO_NUM_16;
constexpr gpio_num_t LCD_SCS        = GPIO_NUM_17;
constexpr gpio_num_t LCD_EXTCOMIN   = GPIO_NUM_18;
constexpr gpio_num_t LCD_DISP       = GPIO_NUM_21;

constexpr gpio_num_t ENC1_A         = GPIO_NUM_38;
constexpr gpio_num_t ENC1_B         = GPIO_NUM_39;
constexpr gpio_num_t ENC1_SW        = GPIO_NUM_40;
constexpr gpio_num_t ENC2_A         = GPIO_NUM_41;
constexpr gpio_num_t ENC2_B         = GPIO_NUM_42;
constexpr gpio_num_t ENC2_SW        = GPIO_NUM_47;

constexpr gpio_num_t BTN_PREV       = GPIO_NUM_2;
constexpr gpio_num_t BTN_NEXT       = GPIO_NUM_14;
constexpr gpio_num_t BTN_PLAY       = GPIO_NUM_48;

constexpr int DAC_I2C_ADDRESS       = 0x48; // ADDR is pulled low by R27 in the schematic.
constexpr int LCD_WIDTH             = 336;
constexpr int LCD_HEIGHT            = 536;
constexpr int LCD_ROW_BYTES         = LCD_WIDTH / 8;
constexpr int LCD_FRAME_BYTES       = LCD_ROW_BYTES * LCD_HEIGHT;
}
