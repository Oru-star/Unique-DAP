idf_component_register(
    SRCS
        "app_main.cpp"
        "board.cpp"
        "dac_es9038.cpp"
        "storage/storage.cpp"
        "input/controls.cpp"
        "display/sharp_ls032.cpp"
        "audio/audio_player.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_i2s
        esp_driver_spi
        esp_timer
        fatfs
        sdmmc
        nvs_flash
        micro-flac
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wshadow -Wconversion)
