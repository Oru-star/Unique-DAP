# Hardware Review and Bring-up Notes

This document records the firmware assumptions taken from the six-page KiCad schematic `DAP'S PCB(6).pdf`. It is not a substitute for ERC/DRC, PCB-layout review, datasheet review, or bench testing.

## 1. Pin map used by the firmware

| Function | ESP32-S3 GPIO | Schematic sheet |
|---|---:|---|
| BQ25895 interrupt (not yet serviced) | GPIO1 | Power Management / Core |
| Previous button | GPIO2 | Core / User Controls |
| ES9038Q2M RESETB | GPIO3 | DAC / Core |
| I²S BCLK | GPIO4 | DAC / Core |
| I²S LRCK / WS | GPIO5 | DAC / Core |
| I²S DATA to DAC | GPIO6 | DAC / Core |
| DAC_MCLK label | GPIO7 | Core |
| I²C SDA | GPIO8 | Power / DAC / Core |
| I²C SCL | GPIO9 | Power / DAC / Core |
| microSD CS / DAT3 | GPIO10 | Core |
| microSD MOSI / CMD | GPIO11 | Core |
| microSD SCLK / CLK | GPIO12 | Core |
| microSD MISO / DAT0 | GPIO13 | Core |
| Next button | GPIO14 | Core / User Controls |
| LCD SCLK | GPIO15 | Core / Display |
| LCD SI | GPIO16 | Core / Display |
| LCD SCS | GPIO17 | Core / Display |
| LCD EXTCOMIN | GPIO18 | Core / Display |
| LCD DISP | GPIO21 | Core / Display |
| Encoder 1 A / B / switch | GPIO38 / 39 / 40 | Core / User Controls |
| Encoder 2 A / B / switch | GPIO41 / 42 / 47 | Core / User Controls |
| Boost converter enable | GPIO45 | Power / Core |
| Play button | GPIO48 | Core / User Controls |

**GPIO7 is not driven by this firmware.** The `DAC_MCLK` text on GPIO7 has no matching net to the DAC XI pin in the supplied schematic. The separate oscillator Y1 output is labelled `DAC_XI` and is connected to ES9038Q2M XI. The ESP32-S3 generates BCLK and LRCK; it does not output MCLK on GPIO7.

## 2. DAC configuration assumptions

- The ES9038Q2M ADDR pin is shown with a 10 kΩ pull-down (R27). ESS documents ADDR low as the 8-bit bus address `0x90`, equivalent to the 7-bit address **`0x48`** used by ESP-IDF I²C APIs. If the fitted resistor or net differs from the drawing, confirm with an I²C scan before changing the firmware.
- I²C is configured at 400 kHz. The DAC and the other devices on the schematic share SDA/SCL, so future BQ25895 and TUSB320 drivers must attach to the existing I²C master bus rather than create another bus on GPIO8/9.
- I²S is Philips/I²S format with 32-bit stereo slots. At 48 kHz, expected BCLK is `48,000 × 2 × 32 = 3.072 MHz`; LRCK is 48 kHz.
- The external oscillator frequency and exact part number are not identified in the supplied schematic. Measure the signal at `DAC_XI` and verify it against the populated oscillator's datasheet. ESS allows the serial audio interface timing to be asynchronous to MCLK when the DPLL is enabled; its default lock-speed setting uses 5,461 audio-frame edges. The firmware sends 8,192 muted frames plus a DMA drain interval before unmuting, but does not read back the DAC's DPLL lock status.
- At startup, firmware writes Register `0x0E` = `0x8A`: bit 7 enables the analog output soft-start (required for analog output), bit 6 remains at its default, and the low five bits preserve the datasheet default ramp time of 10. It then writes Register `0x07` = `0x81` for the default apodizing filter plus soft mute. Registers `0x0F` and `0x10` set left/right digital attenuation. Filter tuning, DPLL bandwidth tuning and DAC status diagnostics are future work.
- The MAX97220A headphone amplifier SHDN pin appears tied high in the schematic. Its SHDN is active-low, so a high level enables the amplifier. The firmware can soft-mute the DAC but cannot independently shut down the amplifier through a GPIO in this design.

## 3. Sharp LS032B7DD02 memory LCD

The display connector on sheet 6 maps:

| Connector pin | Signal / supply | ESP32-S3 connection |
|---:|---|---|
| 1 | SCLK | GPIO15 |
| 2 | SI | GPIO16 |
| 3 | SCS | GPIO17 |
| 4 | EXTCOMIN | GPIO18 |
| 5 | DISP | GPIO21 |
| 6 | VDDA | +5V_LCD |
| 7 | VDD | +5V_LCD |
| 8 | EXTMODE | +5V_LCD |
| 9 | VSS | GND |
| 10 | VSSA | GND |

The panel is 336 × 536. The display signal inputs are logic-level pins; the schematic supplies +5V_LCD to VDDA, VDD and EXTMODE. EXTMODE high selects external EXTCOMIN operation. The driver uses 1 MHz SPI, SCS high during each transaction, 1-based 10-bit row addresses, 42 bytes per row and 16 trailing dummy clocks. The panel consumes serial bits LSB-first; the firmware stores the first pixel in bit 0 and reverses each byte before the ESP32's MSB-first SPI transmission. EXTCOMIN is toggled every 500 ms (1 Hz square wave).

Verify actual logic-high thresholds and the panel supply requirements against the exact panel datasheet and assembled wiring before first power-up. Do not connect 5 V directly to ESP32 GPIOs.

## 4. Schematic items to verify or improve

### microSD CS idle pull-up

The card's DAT3/CD pin is used as SPI chip-select (`SD_CS`, GPIO10). The schematic shows pull-ups on DAT2 and DAT1, but no obvious pull-up on DAT3/CS. Firmware drives CS high before mounting, but add an external **10 kΩ pull-up from SD_CS/DAT3 to 3.3 V** on the PCB if the design is not yet frozen. An external resistor keeps the card deselected during ESP32 reset and boot. Verify the MISO/DAT0 idle behavior as well.

### ESP32-S3 strapping pins

- GPIO45 is a strapping pin controlling VDD_SPI selection. R1 is shown as a 100 kΩ pull-down, consistent with the 3.3 V VDD_SPI strap. Keep the external pull-down fitted and verify the actual module's flash voltage before fabrication/assembly. Firmware only raises GPIO45 after boot to enable the 5 V boost converter.
- GPIO3 is also a strapping pin (JTAG source selection) and is used here for DAC RESETB with an external 10 kΩ pull-up. The strap is sampled at reset before firmware drives the pin. Verify that the chosen GPIO3 strap level is compatible with your desired USB/JTAG debug route and eFuse configuration.

### Oscillator / DAC MCLK

The oscillator frequency is not stated in the schematic title block or symbol label. Do not infer its frequency from the GPIO7 `DAC_MCLK` label; GPIO7 is unused in this PCB connection. Identify the exact Y1 manufacturer part number and measure its output at the DAC XI pin. Verify clean startup and the DAC's clocking/DPLL behavior across 44.1 kHz and 48 kHz sample-rate families.

### USB-C digital audio

The J1 USB-C D+/D− signals on the power sheet are routed into the charging/USB detection circuitry, not to ESP32-S3 GPIO19/20. The current schematic therefore does **not** provide a functioning USB Audio Class digital-audio output path. This firmware does not advertise or attempt USB digital output. A future USB audio design needs a deliberate topology, routing, VBUS/role control and firmware support.

### Power and charging

The firmware enables the TPS61023 5 V boost rail using GPIO45. The 5 V rail is used by the LCD and headphone amplifier. The current firmware does not program the BQ25895, read charge status, estimate battery percentage, or control USB-C roles through TUSB320. Keep charger and USB power-path validation as a separate hardware/firmware task.

### Audio amplifier

The MAX97220A is enabled by the high level on SHDN (active-low shutdown). There is no MCU GPIO shown for this signal. The application uses ES9038Q2M soft mute and digital attenuation; it does not implement a hardware amplifier power-down sequence.

## 5. Recommended bench bring-up

1. **Before power:** inspect component polarity/orientation, battery connector polarity, solder bridges, exposed-pad grounding and rail-to-ground resistance. Check the PCB power nets against the schematic and actual BOM.
2. **Current-limited supply:** initially power the battery/system input from a current-limited bench supply at the battery's nominal voltage. Start with a conservative current limit for short-circuit detection, then increase only after confirming no abnormal heating and measuring the rails. Do not connect headphones.
3. **Power rails:** measure 3.3 V first. Confirm the boost rail is off during reset and turns on only after firmware starts; then measure +5V_RAW, +5V_AUD and +5V_LCD. Confirm rail stability under the display and audio-amplifier load.
4. **Serial boot:** flash via the intended UART programming header and verify stable boot logs. If using JTAG, investigate the GPIO3 strap and debug-source selection.
5. **I²C:** scan GPIO8/9 and confirm the DAC acknowledges at 7-bit `0x48`. Confirm there is no address collision with the charger or Type-C controller.
6. **Oscillator:** scope Y1 output at `DAC_XI`; verify frequency, amplitude, startup and signal integrity at the DAC pin.
7. **I²S:** play a test file with headphones disconnected. At 48 kHz and 32-bit stereo slots, expect 48 kHz LRCK and 3.072 MHz BCLK on GPIO4; observe serial data on GPIO6. GPIO7 should not be used as MCLK in the present schematic.
8. **LCD:** verify 1 MHz SCLK, SCS active-high bursts, DISP high and EXTCOMIN toggling at 1 Hz. Confirm the +5V_LCD supply and 3.3 V logic levels.
9. **microSD:** use a FAT32 card. Verify CS stays high at reset and the card mounts at 20 MHz SPI. If mount fails, first check DAT3/CS pull-up, card formatting and SPI pin mapping.
10. **Analog output:** before connecting IEMs/headphones, test the headphone jack with an appropriate dummy load and measurement equipment. Check DC offset, turn-on/off transients, noise and gain. Start at the firmware's conservative digital volume setting and increase gradually.

## 6. Firmware limitations relevant to qualification

- Playback is limited to 96 kHz and the formats listed in `README.md`.
- No measured THD+N, noise floor, channel balance, output impedance, crosstalk or output-power claims are made.
- No protection against every malformed media file is guaranteed; test with a curated sample set.
- No watchdog recovery, persistent settings, battery telemetry, power-button shutdown state machine or low-battery shutdown is implemented yet.
- The audio start sequence waits through the default DPLL lock window but does not read the DAC lock status. Confirm lock, sample-rate accuracy and jitter behavior on the assembled board.
- The schematic PDF is a functional block-level review source; the PCB layout, netlist, BOM variants and component datasheets must also be checked before fabrication or first power-up.
