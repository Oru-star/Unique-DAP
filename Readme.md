# DAP — Procurement BOM

**BOM revision:** 11 November 2026  
**Status:** Engineering-review procurement list — **not yet released for fabrication or bulk purchasing**.

This repository BOM records the parts identified from the current KiCad schematic and footprint-assignment inventory. It includes the Sharp display, audio DAC and headphone amplifier, power-management ICs, ESP32-S3 module, connectors, passives, oscillator, and the newly assigned microSD socket and 128 GB card.

## BOM files

The [BOM CSV](./Bom.csv) contains the detailed procurement data.

## Added microSD storage

| RefDes | Item | Manufacturer / MPN | KiCad footprint | Qty |
|---|---|---|---|---:|
| J6 | microSD socket, right-angle SMT, push-push, top-mount | Hirose `DM3AT-SF-PEJM5` | `Connector_Card:microSD_HC_Hirose_DM3AT-SF-PEJM5` | 1 |
| SD1 | 128 GB microSDXC UHS-I card | SanDisk `SDSQUAB-128G-GN6MN` | Removable media; not mounted on PCB | 1 |

### J6 — Hirose DM3AT-SF-PEJM5

- [Manufacturer product page, drawings and STEP model](https://www.hirose.com/en/product/p/CL0609-0031-0-00)
- [DigiKey India listing](https://www.digikey.in/en/products/detail/hirose-electric-co-ltd/DM3AT-SF-PEJM5/2533565)
- [KiCad Connector_Card footprint library](https://kicad.github.io/footprints/Connector_Card.html)
- DigiKey price snapshot: **₹339.20 each**; inventory and lead time must be reconfirmed at checkout.
- Hirose describes this connector as a push-push socket with a card-detection switch. The DAP's earlier requirement was **no card-detect function**. Do not wire the switch merely because the connector has it: verify the schematic symbol-to-footprint pad mapping and leave detection unused unless the requirement changes.
- Hirose provides an exact STEP model in a ZIP on its product page. The published KiCad `Connector_Card` 3D package lists a `DM3D-SF` model, not this exact `DM3AT-SF-PEJM5` model. Use the Hirose STEP file for accurate enclosure/mechanical checking; do not substitute a visually similar connector model.

### SD1 — 128 GB microSDXC

- Selected candidate: SanDisk Ultra `SDSQUAB-128G-GN6MN`.
- [SanDisk India product specifications](https://www.sandisk.com/en-in/products/memory-cards/microsd-cards/sandisk-ultra-uhs-i-microsd?sku=SDSQUAB-128G-GN6MN)
- [MDComputers listing](https://mdcomputers.in/product/mobile-accessories/memory-card/sandisk-ultra-microsd-card-sdsquab-128g-gn6mn)
- Price snapshot: **₹2,578**; check current availability, seller and warranty before purchase.
- SanDisk specifies microSDXC, UHS-I and up to 140 MB/s read performance. The high advertised transfer rate is not required for audio playback; reliable media and filesystem compatibility matter more.
- A 128 GB card is SDXC. Verify the ESP32-S3 firmware filesystem supports the card's format (commonly exFAT from factory) or define and test a supported FAT32 formatting workflow. Test the full usable capacity and long FLAC/WAV playback.

## Procurement snapshot

The workbook summary calculates the priced subtotal from line items with captured unit prices. It is **not** a landed-cost quotation and excludes taxes not shown by the supplier, shipping, customs/import duties, PCB fabrication, assembly and any rows without a captured price. Stock and prices are time-sensitive; recheck them before checkout.

## Engineering holds before ordering/fabrication

1. **L1/L2/L3 power inductors:** the current footprint inventory shows 0805 footprints, while the selected power-inductor candidates use larger power packages. Update footprints and revalidate saturation current, RMS current, DCR, thermal rise and converter layout before ordering.
2. **U4 TUSB320LAIRWBR:** pad dimensions were changed during DRC cleanup. Recheck against TI's recommended land pattern, solder-mask web and the PCB fabricator's capability before release.
3. **U3 BQ25895:** confirm TS/NTC arrangement, charge-current setting, battery protection and thermal design.
4. **Y1 / ES9038Q2M:** confirm the master-clock frequency and divider plan for every supported sample rate before finalizing the oscillator.
5. **U6 ESP32-S3:** confirm the exact WROOM memory suffix and firmware target.
6. **J4 / Sharp display:** verify FPC contact side, thickness, orientation and exact pin mapping.
7. **J6 / microSD:** verify Hirose symbol-to-pad numbering, card-detect switch handling, shield pads and mechanical card access.
8. **Switches and encoders:** resolve symbol/footprint mismatches and confirm exact mechanical variants.
9. **PCB DRC:** rerun after footprint updates and finish connectivity, unconnected-track/via review, copper-zone checks and final fabrication-rule review.

## Status meanings

- **BUY** — a candidate part and source price were identified; final design notes still apply.
- **VERIFY** — confirm one or more electrical, mechanical, firmware, supplier-stock or pricing details.
- **HOLD** — do not order until the listed design issue is resolved.

## Important procurement rule

Do not order components solely because a footprint has been assigned in KiCad. Verify the exact manufacturer part number, datasheet drawing, pad numbering, package dimensions, electrical ratings and mating/mechanical requirements first.

## Complete BOM

| Category | RefDes | Qty | Item / value | Manufacturer | MPN / candidate | Unit price (INR) | Status |
|---|---|---:|---|---|---|---:|---|
| Display | DISP1 | 1 | 3.16-in reflective memory LCD, 336 × 536, serial interface | Sharp | LS032B7DD02 | 3739.83 | BUY |
| DAC | IC1 | 1 | 32-bit stereo audio DAC | ESS / formerly ESS Technology | ES9038Q2M | 1967.96 | BUY |
| Power IC | U1 | 1 | 3.7-A boost converter | Texas Instruments | TPS61023DRLR | 116.56 | BUY |
| Power IC | U2 | 1 | Buck-boost converter | Texas Instruments | TPS63021DSJR | 311.22 | BUY |
| Charger IC | U3 | 1 | 1-cell Li-ion/Li-poly charger with power-path management | Texas Instruments | BQ25895RTWR | 321.05 | VERIFY |
| USB-C controller | U4 | 1 | USB Type-C configuration channel controller | Texas Instruments | TUSB320LAIRWBR | 174.05 | BUY |
| Headphone amplifier | U5 | 1 | Stereo DirectDrive headphone amplifier | Analog Devices / Maxim | MAX97220AETE+T | 231.09 | BUY |
| MCU module | U6 | 1 | ESP32-S3 Wi-Fi/Bluetooth module; candidate 8 MB flash + 8 MB PSRAM | Espressif | ESP32-S3-WROOM-1-N8R8 (verify suffix) | 599.99 | VERIFY |
| USB-C receptacle | J1 | 1 | USB 2.0 Type-C top-mount receptacle | GCT | USB4105-GF-A (candidate) | 76.44 | VERIFY |
| Battery header | J2 | 1 | 2-position 2.54 mm straight through-hole pin header | Samtec | TSW-102-07-G-S (candidate) | 20.07 | VERIFY |
| Programming header | J3 | 1 | 5-position 2.54 mm straight through-hole pin header | Samtec | TSW-105-07-G-S (candidate) | 40.66 | BUY |
| Display FPC connector | J4 | 1 | 10-position 0.5 mm FPC/FFC ZIF connector, right-angle | Hirose | FH34SRJ-10S-0.5SH(50) | 86.66 | VERIFY |
| Headphone jack | J5 | 1 | 3.5 mm stereo TRS right-angle through-hole jack, unswitched | Same Sky / CUI Devices | SJ1-3533NG | 160.74 | BUY |
| Ferrite bead | FB1 | 1 | 600 Ω @ 100 MHz ferrite bead, 0805 | Murata | BLM21PG601SN1D | 11.47 | VERIFY |
| Power inductor | L1 | 1 | 1.0 µH shielded power inductor; TI-listed candidate | Würth Elektronik | 74438357010 | 161.46 | HOLD |
| Power inductor | L2 | 1 | 1.5 µH shielded power inductor; TI-listed candidate | Coilcraft | XFL4020-152ME | — | HOLD |
| Power inductor | L3 | 1 | 2.2 µH shielded power inductor | Murata | DFE322512F-2R2M=P2 | 46.82 | HOLD |
| Oscillator | Y1 | 1 | 40.000 MHz CMOS oscillator, 3.3 V, 4-pad 3.2 × 2.5 mm | Abracon | ASE-40.000MHZ-L-C-T | 99.37 | VERIFY |
| Capacitor | C1,C8-C10,C13,C14,C30 | 7 | 10 µF MLCC, 0603; suggested 10 V X5R | Samsung (candidate) | CL10A106KP8NNND (candidate) | — | VERIFY |
| Capacitor | C2,C16-C21,C23,C28,C31 | 10 | 100 nF MLCC, 0603; suggested 50 V X7R | Murata | GRM188R71H104KA93D (candidate) | — | VERIFY |
| Capacitor | C3-C6 | 4 | 22 µF MLCC, 0603, 6.3 V X5R | Yageo | CC0603MRX5R5BB226 | 54.46 | HOLD |
| Capacitor | C7,C12,C24-C27,C29,C32,C34,C35 | 10 | 1 µF MLCC, 0603, 16 V X7R | Murata | GRM188R71C105KA12D | — | VERIFY |
| Capacitor | C15,C22,C36-C39 | 6 | 4.7 µF MLCC, 0603; suggested 10 V X7R | Murata (candidate) | GRM188R71A475KE15D (candidate) | — | VERIFY |
| Capacitor | C11 | 1 | 47 nF MLCC, 0603; suggested 25 V X7R | Murata (candidate) | GRM188R71E473KA61D (candidate) | — | VERIFY |
| Capacitor | C33 | 1 | 560 pF C0G/NP0, 50 V, 0603 | Murata | GRM1885C1H561JA01D | — | VERIFY |
| Resistor | R4,R6,R7,R9-R11,R15,R16,R18,R19,R27 | 11 | 10 kΩ, 1%, 0805 | Yageo | RC0805FR-0710KL | 3.249 | BUY |
| Resistor | R17,R20-R26 | 8 | 20 kΩ, 1%, 0805 | TE Connectivity (candidate alternate) | CRG0805F20K | 9.56 | BUY |
| Resistor | R5,R8 | 2 | 260 Ω, 1%, 0805 | Yageo (candidate) | RC0805FR-07260RL | — | VERIFY |
| Resistor | R13,R14 | 2 | 4.7 kΩ, 1%, 0805 | Yageo (candidate) | RC0805FR-074K7L | — | VERIFY |
| Resistor | R1 | 1 | 100 kΩ, 1%, 0805 | Yageo (candidate) | RC0805FR-07100KL | — | VERIFY |
| Resistor | R2 | 1 | 27.4 kΩ, 1%, 0805 | Yageo (candidate) | RC0805FR-0727K4L | — | VERIFY |
| Resistor | R3 | 1 | 200 kΩ, 1%, 0805 | Yageo (candidate) | RC0805FR-07200KL | — | VERIFY |
| Resistor | R12 | 1 | 900 kΩ, 1%, 0805 | Yageo (candidate) | RC0805FR-07900KL | — | VERIFY |
| Rotary encoder | SW2,SW3 | 2 | Incremental rotary encoder with push switch; vertical, 20 mm body class | ALPS Alpine | EC11E family — exact suffix TBD | — | HOLD |
| Slide / push controls | SW1,SW2-BOOT1,SW4-SW6 | 5 | Control switches; symbol says SW_Push but assigned footprint is Copal CHS-01A slide switch | Copal (footprint family) | CHS-01A family — function/variant TBD | — | HOLD |
| Battery (off-board) | BAT1 (not a PCB footprint) | 1 | Single-cell 3.7 V Li-Po, 2500 mAh project candidate | WLY (project candidate) | WLY104050 — confirm exact supplier/lead configuration | — | HOLD |
| microSD card socket | J6 | 1 | microSD socket, SMT right-angle, push-push, top-mount; card-detect switch | Hirose Electric | DM3AT-SF-PEJM5 (CL0609-0031-0-00) | 339.2 | VERIFY |
| Storage media | SD1 (off-board) | 1 | 128 GB microSDXC UHS-I memory card, Class 10 / U1 / A1; up to 140 MB/s read | SanDisk | SDSQUAB-128G-GN6MN | 2578.0 | VERIFY |
