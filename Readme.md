# DAP — Procurement BOM

**BOM revision:** 11 November 2026  
**Status:** Engineering-review procurement list — **not yet released for fabrication or bulk purchasing**.

This repository BOM records the parts identified from the current KiCad schematic and footprint-assignment inventory. It includes the Sharp display, audio DAC and headphone amplifier, power-management ICs, ESP32-S3 module, connectors, passives, oscillator, and the newly assigned microSD socket and 128 GB card.

## BOM files

Keep these files together in the repository root (or update the links below if you place them in another folder):

- [`DAP_Procurement_BOM_2026-11-11_v2.xlsx`](./DAP_Procurement_BOM_2026-11-11_v2.xlsx) — formatted workbook with Summary, BOM and Design Review sheets.
- [`DAP_Procurement_BOM_2026-11-11_v2.csv`](./DAP_Procurement_BOM_2026-11-11_v2.csv) — flat BOM for scripts, procurement imports and version control.

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
