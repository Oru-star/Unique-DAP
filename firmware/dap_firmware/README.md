# Custom ESP32-S3 Hi-Res DAP Firmware

**Version:** 0.1.0 bring-up prototype  
**Target:** ESP32-S3-WROOM-1, configured for the N8R8 variant (8 MB flash + 8 MB octal PSRAM)  
**Framework:** ESP-IDF 5.5.6  
**Language:** C++17

This project is the first firmware foundation for the custom DAP described by the six-sheet KiCad schematic supplied with the project. It prioritizes safe subsystem bring-up and local playback rather than pretending that every planned feature is already implemented.

> **Hardware status:** This source has not been compiled with ESP-IDF or tested on the physical PCB in this environment. Treat it as an engineering prototype. Review `HARDWARE_REVIEW.md` before powering the board or connecting headphones.

## Implemented in this prototype

- ESP32-S3 application startup and GPIO control for the 5 V boost rail.
- Shared I²C master for the ES9038Q2M control interface.
- ES9038Q2M probe and reset; startup sets Register 14 to `0x8A` to enable the analog output soft-start (required by the datasheet), then applies the default apodizing filter, soft mute and conservative left/right digital attenuation.
- I²S standard-mode Philips-format TX using 32-bit stereo slots. BCLK, LRCK and DATA are driven by the ESP32-S3; the DAC XI reference clock remains on the external oscillator shown on the DAC sheet.
- microSD SPI mounting without automatic formatting.
- Recursive scan of `/MUSIC` (or the card root if `/MUSIC` is absent), up to 256 WAV/FLAC files and four directory levels.
- WAV PCM playback: 8/16/24/32-bit, mono or stereo, 8–96 kHz.
- Native FLAC playback: 16/24/32-bit, mono or stereo, 8–96 kHz. Decoded samples are converted/forwarded as signed 32-bit left-justified PCM.
- Sharp LS032B7DD02 336 × 536 memory-LCD initialization, LSB-first pixel/10-bit address packing, line writes, framebuffer and EXTCOMIN timing.
- Two quadrature encoders and three buttons, scanned with debounce.
- Basic now-playing/library screen, playback time, format, volume and device status.
- Muted I²S silence pre-roll before unmuting the DAC.

The FLAC decoder is `esphome/micro-flac` 0.3.x, which provides the streaming decoder API used here. See the component documentation for its output-buffer and sample-format contract: <https://components.espressif.com/components/esphome/micro-flac/versions/0.3.0/readme>.

## Not implemented yet

- USB Audio Class output over USB-C. The present schematic does not route the ESP32-S3 USB D+/D− pins to the USB-C receptacle.
- Bluetooth audio streaming. ESP32-S3 has no Bluetooth Classic A2DP; BLE control/playlist synchronization is a future feature, not present in this build.
- Wi-Fi playlist/metadata synchronization.
- BQ25895 charger configuration, battery/charging telemetry and battery percentage.
- TUSB320 Type-C role management.
- ID3/Vorbis tag display, album art, M3U playlists, shuffle, repeat, gapless playback, crossfade, seek, resume position and persistent settings.
- MP3, AAC, DSD, Ogg containers and WAVE_FORMAT_EXTENSIBLE.
- 192 kHz playback. The present firmware deliberately limits playback to 96 kHz pending clock-tree and analog verification.
- Software control of the MAX97220A headphone-amplifier shutdown pin. The schematic appears to tie SHDN high, so the amplifier is enabled by hardware.
- DAC DPLL lock-status readback or automatic recovery.

## Controls

| Control | Action |
|---|---|
| Encoder 1 rotate | Move through the sorted track list |
| Encoder 1 press | Play the selected track |
| Encoder 2 rotate | Adjust digital volume in 2% steps |
| Encoder 2 press | Toggle mute |
| Previous button | Select and immediately play the previous file |
| Play button | Play selected track, or pause/resume current playback |
| Next button | Select and immediately play the next file |

Selection clamps at the first/last item; it does not wrap. Playback does not automatically advance at end-of-file in this prototype.

## microSD card preparation

1. Format the card as **FAT32**. The default ESP-IDF FAT filesystem configuration in this project does not enable exFAT.
2. Create a `MUSIC` directory at the card root (optional). If it exists, only that tree is scanned; otherwise the card root is scanned.
3. Copy `.wav` and `.flac` files into that location.
4. Insert the card with power removed.

The firmware will never format the card automatically. 128 GB SDXC cards are commonly shipped as exFAT; reformat them to FAT32 on a PC using a suitable tool before use. Back up the card before reformatting.

## Build on Windows

Install **ESP-IDF v5.5.6** and use its ESP-IDF PowerShell or command prompt so `IDF_PATH`, CMake and the Xtensa toolchain are configured.

```powershell
cd path\to\dap_firmware
idf.py set-target esp32s3
idf.py reconfigure
idf.py build
```

The ESP-IDF Component Manager will fetch `esphome/micro-flac` from the component registry during configuration/build.

Flash and open the serial monitor (replace `COM7` with the actual UART port):

```powershell
idf.py -p COM7 flash monitor
```

For a clean rebuild:

```powershell
idf.py fullclean
idf.py reconfigure
idf.py build
```

### Configuration assumptions

`sdkconfig.defaults` selects:

- `esp32s3` target
- 240 MHz CPU
- Octal PSRAM at 80 MHz, allocated through the heap
- 8 MB flash
- 1 kHz FreeRTOS tick

The schematic symbol only says `ESP32-S3-WROOM-1`; it does not state the full suffix. Confirm that the populated module is actually **N8R8** before using these defaults. If the installed module has a different flash/PSRAM size or PSRAM mode, change the defaults using `idf.py menuconfig`.

## Source layout

```text
dap_firmware/
├── CMakeLists.txt
├── sdkconfig.defaults
├── README.md
├── HARDWARE_REVIEW.md
└── main/
    ├── CMakeLists.txt
    ├── idf_component.yml
    ├── app_main.cpp
    ├── board.cpp
    ├── dac_es9038.cpp
    ├── audio/audio_player.cpp
    ├── display/sharp_ls032.cpp
    ├── input/controls.cpp
    ├── storage/storage.cpp
    └── include/
```

## First bring-up sequence

1. Review the hardware warnings and the actual assembled BOM.
2. Build successfully before connecting the PCB.
3. Power the board from a current-limited bench supply and verify 3.3 V and switched 5 V rails before inserting a battery or headphones.
4. Confirm the serial log reports the DAC at I²C 7-bit address `0x48` and that the card mounts.
5. Verify DAC XI, I²S BCLK/LRCK/DATA and LCD clocks with an oscilloscope or logic analyzer.
6. Test with no headphones connected. Measure the headphone output and verify there is no damaging DC offset or startup transient before connecting valuable IEMs.
7. Start with a known-good 16-bit/48 kHz WAV, then try 24-bit FLAC and other sample rates.

See `HARDWARE_REVIEW.md` for the detailed pin map, clock checks and schematic-specific issues.
