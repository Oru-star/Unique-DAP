# Unique DAP

Unique DAP is an ESP32-S3-based portable audio player built around a dedicated high-performance audio DAC and headphone amplifier. It uses a Sharp reflective memory display rather than a conventional backlit LCD, removable microSD storage, and two rotary encoders for navigation. Charging is provided through a USB Type-C port.

<p align="center">
  <img src="RENDERS'n'Images/HOME4.png" alt="Three-quarter render of the Unique DAP enclosure" width="800">
</p>

## Project at a glance

- **Controller:** Espressif ESP32-S3 module
- **Audio:** ESS ES9038Q2M DAC and MAX97220A headphone amplifier
- **Display:** Sharp LS032B7DD02, 3.16-inch reflective memory LCD
- **Storage:** Removable microSDXC card; 128 GB target capacity
- **Controls:** Two rotary encoders and tactile/slide controls (final switch variants still under verification)
- **Design files:** KiCad schematic and PCB, Gerbers, and printable enclosure parts

## Repository contents

| Resource | Description |
|---|---|
| [KiCad PCB](<DAP'S PCB/DAP'S PCB.kicad_pcb>) | Main board layout |
| [KiCad schematic](<DAP'S PCB/DAP'S PCB.kicad_sch>) | Main circuit schematic |
| [Gerber files](<DAP'S PCB/Gerbers/>) | PCB manufacturing outputs |
| [Printable enclosure parts](<steps/Stl for prints/>) | 3D-printable enclosure parts |
| [Full BOM with INR/USD prices and links](./Bom.csv) | Complete component register with supplier links, manufacturer references, price class and engineering notes |

## Project images

### Enclosure

<p align="center">
  <img src="RENDERS'n'Images/TOP.png" alt="Top view of the DAP enclosure and display" width="700">
</p>
<p align="center"><em>Top view</em></p>

<p align="center">
  <img src="RENDERS'n'Images/SIDE-VIEW.PNG" alt="Front side view showing the USB-C port and controls" width="700">
</p>
<p align="center"><em>Front side view</em></p>

### PCB

<p align="center">
  <img src="RENDERS'n'Images/Pcb-Top.png" alt="Top view of the DAP PCB layout" width="600">
</p>
<p align="center"><em>PCB top view</em></p>

<p align="center">
  <img src="RENDERS'n'Images/Pcb-sideview.png" alt="Side view of the DAP PCB assembly" width="800">
</p>
<p align="center"><em>PCB side view</em></p>

## Bill of materials

**The complete BOM is [`Bom.csv`](./Bom.csv).** It includes all 38 BOM entries, MPNs/candidates, manufacturer references, purchase links, INR/USD unit prices, extended prices, price classification and engineering notes.

- **BOM lines with numeric prices or budget allowances:** 38 of 38
- **Budgetary estimates (not supplier quotes):** 4
- **Lines still requiring a quote:** 0
- **Current priced/estimated subtotal:** ₹12,892.44 / $137.32
- **FX reference:** 1 USD = ₹93.8516

> **Pricing caveat:** The subtotal includes supplier/list-price references and clearly marked budgetary estimates. It is not a final landed cost. Reconfirm all prices, stock, taxes, shipping, customs and reeling fees at checkout. PCB fabrication, assembly, stencil and tools are excluded.

### Detailed component pricing and purchase links

Links below were refreshed. Direct manufacturer/distributor product pages are used where available; otherwise the link opens an exact-MPN supplier search. Search results do not guarantee stock or orderability.

| Category | RefDes | Qty | Manufacturer / MPN | Manufacturer reference | Purchase link | Unit INR | Unit USD | Extended INR | Extended USD | Price class | Design status |
|---|---|---:|---|---|---|---:|---:|---:|---:|---|---|
| Display | DISP1 | 1 | `LS032B7DD02` | [Manufacturer / datasheet](https://global.sharp/products/device/lineup/selection/lcd/mobile/index.html) | [Supplier / lookup](https://www.digikey.in/en/products/detail/sharp-microelectronics/LS032B7DD02/23349498) | ₹3,739.83 | $39.85 | ₹3,739.83 | $39.85 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| DAC | IC1 | 1 | `ES9038Q2M` | [Manufacturer / datasheet](https://www.mouser.in/en/ProductDetail/ESS-Technology/ES9038Q2M?qs=sPbYRqrBIVmdeUedURuuOw%3D%3D) | [Supplier / lookup](https://www.mouser.in/en/ProductDetail/ESS-Technology/ES9038Q2M?qs=sPbYRqrBIVmdeUedURuuOw%3D%3D) | ₹1,967.96 | $20.97 | ₹1,967.96 | $20.97 | PRICE REFERENCE — reconfirm checkout | BUY |
| Power IC | U1 | 1 | `TPS61023DRLR` | [Manufacturer / datasheet](https://www.ti.com/product/TPS61023/part-details/TPS61023DRLR) | [Supplier / lookup](https://www.digikey.in/en/products/detail/texas-instruments/TPS61023DRLR/11310667) | ₹116.56 | $1.24 | ₹116.56 | $1.24 | PRICE REFERENCE — reconfirm checkout | BUY |
| Power IC | U2 | 1 | `TPS63021DSJR` | [Manufacturer / datasheet](https://www.ti.com/product/TPS63021/part-details/TPS63021DSJR) | [Supplier / lookup](https://www.digikey.in/en/products/detail/texas-instruments/TPS63021DSJR/2271632) | ₹384.11 | $4.09 | ₹384.11 | $4.09 | PRICE REFERENCE — reconfirm checkout | BUY |
| Charger IC | U3 | 1 | `BQ25895RTWR` | [Manufacturer / datasheet](https://www.ti.com/product/BQ25895) | [Supplier / lookup](https://www.digikey.in/en/products/detail/texas-instruments/BQ25895RTWR/5214018) | ₹321.05 | $3.42 | ₹321.05 | $3.42 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| USB-C controller | U4 | 1 | `TUSB320LAIRWBR` | [Manufacturer / datasheet](https://www.ti.com/product/TUSB320LAI/part-details/TUSB320LAIRWBR) | [Supplier / lookup](https://www.digikey.in/en/products/detail/texas-instruments/TUSB320LAIRWBR/5722618) | ₹174.05 | $1.85 | ₹174.05 | $1.85 | PRICE REFERENCE — reconfirm checkout | BUY |
| Headphone amplifier | U5 | 1 | `MAX97220AETE+T` | [Manufacturer / datasheet](https://www.analog.com/en/products/max97220a.html) | [Supplier / lookup](https://www.digikey.in/en/products/detail/analog-devices-inc-maxim-integrated/MAX97220AETE-T/2744332) | ₹245.56 | $2.62 | ₹245.56 | $2.62 | PRICE REFERENCE — reconfirm checkout | BUY |
| MCU module | U6 | 1 | `ESP32-S3-WROOM-1-N8R8` | [Manufacturer / datasheet](https://www.espressif.com/en/products/modules/esp32-s3-wroom-1) | [Supplier / lookup](https://www.digikey.in/en/products/detail/espressif-systems/ESP32-S3-WROOM-1-N8R8/16705630) | ₹603.88 | $6.43 | ₹603.88 | $6.43 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| USB-C receptacle | J1 | 1 | `USB4105-GF-A` | [Manufacturer / datasheet](https://gct.co/connector/usb4105) | [Supplier / lookup](https://www.digikey.in/en/products/detail/gct/USB4105-GF-A/11198510) | ₹76.44 | $0.81 | ₹76.44 | $0.81 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Battery header | J2 | 1 | `TSW-102-07-G-S` | [Manufacturer / datasheet](https://www.samtec.com/products/tsw-102-07-g-s) | [Supplier / lookup](https://www.digikey.in/en/products/detail/samtec-inc/TSW-102-07-G-S/1101325) | ₹20.07 | $0.21 | ₹20.07 | $0.21 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Programming header | J3 | 1 | `TSW-105-07-G-S` | [Manufacturer / datasheet](https://www.samtec.com/products/tsw-105-07-g-s) | [Supplier / lookup](https://www.digikey.in/en/products/detail/samtec-inc/TSW-105-07-G-S/1101322) | ₹40.66 | $0.43 | ₹40.66 | $0.43 | PRICE REFERENCE — reconfirm checkout | BUY |
| Display FPC connector | J4 | 1 | `FH34SRJ-10S-0.5SH(50)` | [Manufacturer / datasheet](https://www.hirose.com/en/product/p/CL0580-1251-5-50) | [Supplier / lookup](https://www.digikey.in/en/products/detail/hirose-electric-co-ltd/FH34SRJ-10S-0-5SH-50/3880296) | ₹116.57 | $1.24 | ₹116.57 | $1.24 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Headphone jack | J5 | 1 | `SJ1-3533NG` | [Manufacturer / datasheet](https://www.digikey.in/en/products/detail/same-sky-formerly-cui-devices/SJ1-3533NG/738701) | [Supplier / lookup](https://www.digikey.in/en/products/detail/same-sky-formerly-cui-devices/SJ1-3533NG/738701) | ₹160.74 | $1.71 | ₹160.74 | $1.71 | PRICE REFERENCE — reconfirm checkout | BUY |
| Ferrite bead | FB1 | 1 | `BLM21PG601SN1D` | [Manufacturer / datasheet](https://www.murata.com/en-global/products/productdetail?partno=BLM21PG601SN1D) | [Supplier / lookup](https://www.digikey.in/en/products/detail/murata-electronics/BLM21PG601SN1D/24763118) | ₹11.47 | $0.12 | ₹11.47 | $0.12 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Power inductor | L1 | 1 | `74438357010` | [Manufacturer / datasheet](https://www.we-online.com/en/components/products/WE-MAPI) | [Supplier / lookup](https://in.element14.com/wurth-elektronik/74438357010/inductor-1uh-7-4a-20-shielded/dp/2842181) | ₹161.46 | $1.72 | ₹161.46 | $1.72 | PRICE REFERENCE — reconfirm checkout | HOLD |
| Power inductor | L2 | 1 | `XFL4020-152ME` | [Manufacturer / datasheet](https://www.coilcraft.com/en-us/products/power/high-voltage-inductors/xfl/xfl4020/) | [Supplier / lookup](https://www.coilcraft.com/en-us/products/power/high-voltage-inductors/xfl/xfl4020/) | ₹231.81 | $2.47 | ₹231.81 | $2.47 | LIST PRICE REFERENCE — confirm configuration | HOLD |
| Power inductor | L3 | 1 | `DFE322512F-2R2M=P2` | [Manufacturer / datasheet](https://www.murata.com/en-global/products/productdetail?partno=DFE322512F-2R2M%3DP2) | [Supplier / lookup](https://www.digikey.in/en/products/detail/murata-electronics/DFE322512F-2R2M-P2/9815914) | ₹46.82 | $0.50 | ₹46.82 | $0.50 | PRICE REFERENCE — reconfirm checkout | HOLD |
| Oscillator | Y1 | 1 | `ASE-40.000MHZ-L-C-T` | [Manufacturer / datasheet](https://abracon.com/Oscillators/ASE.pdf) | [Supplier / lookup](https://www.digikey.in/en/products/detail/abracon-llc/ASE-40-000MHZ-L-C-T/5980161) | ₹99.37 | $1.06 | ₹99.37 | $1.06 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Capacitor | C1,C8-C10,C13,C14,C30 | 7 | `CL10A106KP8NNND` | [Manufacturer / datasheet](https://www.samsungsem.com/global/product/passive-component/mlcc.do) | [Supplier / lookup](https://www.digikey.in/en/products/detail/samsung-electro-mechanics/CL10A106KP8NNND/3894249) | ₹19.11 | $0.20 | ₹133.77 | $1.43 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Capacitor | C2,C16-C21,C23,C28,C31 | 10 | `GRM188R71H104KA93D` | [Manufacturer / datasheet](https://www.murata.com/en-global/products/productdetail?partno=GRM188R71H104KA93D) | [Supplier / lookup](https://www.digikey.in/en/products/detail/murata-electronics/GRM188R71H104KA93D/587143) | ₹15.29 | $0.16 | ₹152.90 | $1.63 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Capacitor | C3-C6 | 4 | `CC0603MRX5R5BB226` | [Manufacturer / datasheet](https://www.yageo.com/en/ProductSearch/Index) | [Supplier / lookup](https://www.digikey.in/en/products/detail/yageo/CC0603MRX5R5BB226/5195222) | ₹54.46 | $0.58 | ₹217.84 | $2.32 | PRICE REFERENCE — reconfirm checkout | HOLD |
| Capacitor | C7,C12,C24-C27,C29,C32,C34,C35 | 10 | `C1608X7R1C105K080AC` | [Manufacturer / datasheet](https://product.tdk.com/en/search/capacitor/ceramic/mlcc/info?part_no=C1608X7R1C105K080AC) | [Supplier / lookup](https://www.digikey.in/en/products/result?keywords=C1608X7R1C105K080AC) | ₹11.47 | $0.12 | ₹114.70 | $1.22 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Capacitor | C15,C22,C36-C39 | 6 | `GRM188R71A475KE15D` | [Manufacturer / datasheet](https://www.murata.com/en-global/products/productdetail?partno=GRM188R71A475KE15D) | [Supplier / lookup](https://www.digikey.in/en/products/result?keywords=GRM188R71A475KE15D) | ₹5.00 | $0.05 | ₹30.00 | $0.32 | ESTIMATE — not a supplier quote | VERIFY |
| Capacitor | C11 | 1 | `GRM188R71E473KA61D` | [Manufacturer / datasheet](https://www.murata.com/en-global/products/productdetail?partno=GRM188R71E473KA61D) | [Supplier / lookup](https://www.digikey.in/en/products/result?keywords=GRM188R71E473KA61D) | ₹3.00 | $0.03 | ₹3.00 | $0.03 | ESTIMATE — not a supplier quote | VERIFY |
| Capacitor | C33 | 1 | `GRM1885C1H561JA01D` | [Manufacturer / datasheet](https://www.murata.com/en-global/products/productdetail?partno=GRM1885C1H561JA01D) | [Supplier / lookup](https://www.digikey.in/en/products/detail/murata-electronics/GRM1885C1H561JA01D/586954) | ₹15.29 | $0.16 | ₹15.29 | $0.16 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Resistor | R4,R6,R7,R9-R11,R15,R16,R18,R19,R27 | 11 | `RC0805FR-0710KL` | [Manufacturer / datasheet](https://www.yageo.com/en/ProductSearch/Index) | [Supplier / lookup](https://www.digikey.in/en/products/detail/yageo/RC0805FR-0710KL/730482) | ₹3.25 | $0.03 | ₹35.74 | $0.38 | PRICE REFERENCE — reconfirm checkout | BUY |
| Resistor | R17,R20-R26 | 8 | `CRG0805F20K` | [Manufacturer / datasheet](https://www.te.com/en/product-1622825-1.html) | [Supplier / lookup](https://www.digikey.in/en/products/detail/te-connectivity-passive-product/CRG0805F20K/2380867) | ₹9.51 | $0.10 | ₹76.08 | $0.81 | PRICE REFERENCE — reconfirm checkout | BUY |
| Resistor | R5,R8 | 2 | `RC0805FR-07260RL` | [Manufacturer / datasheet](https://www.yageo.com/en/ProductSearch/Index) | [Supplier / lookup](https://www.digikey.in/en/products/result?keywords=RC0805FR-07260RL) | ₹3.00 | $0.03 | ₹6.00 | $0.06 | ESTIMATE — not a supplier quote | VERIFY |
| Resistor | R13,R14 | 2 | `RC0805FR-074K7L` | [Manufacturer / datasheet](https://www.yageo.com/en/ProductSearch/Index) | [Supplier / lookup](https://www.digikey.in/en/products/detail/yageo/RC0805FR-074K7L/727929) | ₹9.56 | $0.10 | ₹19.12 | $0.20 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Resistor | R1 | 1 | `RC0805FR-07100KL` | [Manufacturer / datasheet](https://www.yageo.com/en/ProductSearch/Index) | [Supplier / lookup](https://www.digikey.in/en/products/detail/yageo/RC0805FR-07100KL/730491) | ₹10.51 | $0.11 | ₹10.51 | $0.11 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Resistor | R2 | 1 | `RC0805FR-0727K4L` | [Manufacturer / datasheet](https://www.yageo.com/en/ProductSearch/Index) | [Supplier / lookup](https://www.digikey.in/en/products/detail/yageo/RC0805FR-0727K4L/727782) | ₹9.56 | $0.10 | ₹9.56 | $0.10 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Resistor | R3 | 1 | `RC0805FR-07200KL` | [Manufacturer / datasheet](https://www.yageo.com/en/ProductSearch/Index) | [Supplier / lookup](https://www.digikey.in/en/products/detail/yageo/RC0805FR-07200KL/730671) | ₹9.56 | $0.10 | ₹9.56 | $0.10 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Resistor | R12 | 1 | `RC0805FR-07900KL` | [Manufacturer / datasheet](https://www.yageo.com/en/ProductSearch/Index) | [Supplier / lookup](https://www.digikey.in/en/products/result?keywords=RC0805FR-07900KL) | ₹3.00 | $0.03 | ₹3.00 | $0.03 | ESTIMATE — not a supplier quote | VERIFY |
| Rotary encoder | SW2,SW3 | 2 | `EC11E family — EC11E15204A3 price candidate` | [Manufacturer / datasheet](https://tech.alpsalpine.com/e/products/category/enc/) | [Supplier / lookup](https://www.digikey.in/en/products/detail/alps-alpine/EC11E15204A3/21721644) | ₹421.38 | $4.49 | ₹842.76 | $8.98 | PRICE REFERENCE — reconfirm checkout | HOLD |
| Slide / push controls | SW1,SW2-BOOT1,SW4-SW6 | 5 | `CHS-01A` | [Manufacturer / datasheet](https://www.nidec.com/en/product/search/category/B102/M101/S100/) | [Supplier / lookup](https://www.tanotis.com/products/nidec-copal-electronics-chs-01a-dip-switch-1pos-spst-slide-smd) | ₹169.20 | $1.80 | ₹846.00 | $9.01 | PRICE REFERENCE — reconfirm checkout | HOLD |
| Battery (off-board) | BAT1 (not a PCB footprint) | 1 | `WLY104050 — exact supplier/lead configuration TBD` | [Manufacturer / datasheet](https://robu.in/product-category/single-cell-micro-lipo-battery/) | [Supplier / lookup](https://robu.in/product-category/single-cell-micro-lipo-battery/) | ₹600.00 | $6.39 | ₹600.00 | $6.39 | INDIA LISTING — reconfirm checkout | HOLD |
| microSD card socket | J6 | 1 | `DM3AT-SF-PEJM5` | [Manufacturer / datasheet](https://www.hirose.com/product/document?clcode=CL0609-0031-0-00&documenttype=Drawing&lang=en) | [Supplier / lookup](https://www.digikey.in/en/products/detail/hirose-electric-co-ltd/DM3AT-SF-PEJM5/2533565) | ₹339.20 | $3.61 | ₹339.20 | $3.61 | PRICE REFERENCE — reconfirm checkout | VERIFY |
| Storage media | SD1 (off-board) | 1 | `SDSQUAB-128G-GN6MN` | [Manufacturer / datasheet](https://www.westerndigital.com/products/memory-cards/sandisk-ultra-uhs-i-microsd) | [Supplier / lookup](https://indiastore-stage.westerndigital.com/store/sdsquab-128g-gn6mn.html) | ₹909.00 | $9.69 | ₹909.00 | $9.69 | PRICE REFERENCE — reconfirm checkout | VERIFY |

### Link verification note

Direct DigiKey and Mouser product pages were substituted for several generic or stale search links. For parts without a confirmed exact product page, the purchase link opens a supplier search for the exact MPN; verify the result and suffix before ordering.

## Engineering review — unresolved approvals

**A numeric price, supplier link or footprint match does not constitute engineering approval.** The following checks remain mandatory before fabrication:

- **Power inductors L1/L2/L3:** Match each inductor to the actual converter topology, switching frequency, inductance range, peak and RMS current, saturation criterion, DCR loss, thermal rise, footprint and layout. For L2, the Coilcraft XFL4020 1.5 µH series data gives 15.8 mΩ maximum DCR, 4.1 A Isat at 10% inductance drop and 6.7 A Irms at 20°C rise. Compare against the calculated worst-case peak current, not just the converter's headline output current.
- **Battery BAT1:** Robu lists WLY104050 3.7 V, 2500 mAh at ₹600. This is a price reference only. Obtain the cell datasheet and verify dimensions, protection PCM, maximum charge voltage/current, continuous discharge current, NTC/temperature-sense requirements, lead connector and polarity. Keep HOLD until these are confirmed.
- **Battery connector J2:** Samtec TSW-102 is an unpolarized header. Prefer a keyed, locking connector matched to the battery harness; verify polarity and contact current. Do not approve an unkeyed battery connector without a deliberate safety review.
- **Capacitors C15,C22,C36-C39:** The ₹5/pc allowance for the 4.7 µF Murata candidate is a budget estimate, not a quote. Verify exact MPN and lifecycle, voltage rating, dielectric and effective capacitance at the actual DC bias.
- **Capacitor C11:** The ₹3/pc allowance for the 47 nF candidate is a budget estimate. Verify exact MPN, voltage rating, dielectric, tolerance and package against the circuit requirement.
- **Resistors R5/R8:** The ₹3/pc allowance is a budget estimate. Exact Yageo MPN RC0805FR-07260RL was not confirmed in distributor search. Verify the intended 260 Ω value and exact orderable code.
- **Resistor R12:** The ₹3/pc allowance is a budget estimate. Exact Yageo MPN RC0805FR-07900KL was not confirmed. Verify that 900 kΩ is intentional and confirm the exact orderable code; do not silently change it to 909 kΩ or 910 kΩ.
- **Oscillator Y1:** Confirm the 40 MHz oscillator against ES9038Q2M MCLK requirements and the full 44.1 kHz / 48 kHz audio clock plan before ordering.
- **Rotary encoders SW2/SW3:** EC11E15204A3 is a candidate without a push switch. Select a correct push-switch variant or add separate tact switches.
- **Control switches:** Copal CHS-01A is a slide switch, not a momentary pushbutton. Resolve symbol, footprint and actual user-interface function.
- **Capacitors C3-C6:** 22 µF, 6.3 V, 0603 MLCCs need effective-capacitance/DC-bias derating checks, especially on a 5 V rail.
- **Capacitor C2 group:** Confirm lifecycle status of the proposed 100 nF Murata part and choose an active replacement if obsolete.
- **Display DISP1:** Verify Sharp LS032B7DD02 full power rails, logic levels, FPC pinout, flex thickness and contact orientation against the exact panel documentation.
- **microSD J6:** Verify Hirose DM3AT-SF-PEJM5 pad mapping, shield pads and mechanical dimensions against the manufacturer drawing. Card-detect contacts may remain unconnected if unused.
- **USB-C J1:** Verify receptacle footprint, shell stakes, CC configuration, VBUS path, ESD protection and port role against the intended charge/data architecture.

## Assembly and bring-up

1. **Prepare parts:** Check exact MPN, package, footprint, pinout, ratings, lifecycle and availability. Do not substitute solely on nominal value.
2. **Review fabrication outputs:** Inspect the schematic, PCB, design-rule results, drill files, copper, solder mask, silkscreen and board outline. Independently inspect Gerbers before ordering.
3. **Inspect the bare PCB:** Check board damage, opens, shorts, solder-mask defects, drill sizes and connector alignment.
4. **Populate the PCB:** Use the assembly drawing and BOM. Apply solder paste with a stencil, place SMD parts, reflow to an appropriate profile, then install through-hole parts. Inspect fine-pitch ICs and connectors under magnification.
5. **Unpowered checks:** Measure resistance from every power rail to ground, verify polarity, inspect for solder bridges and confirm connector pin mapping.
6. **First power-up:** Use a current-limited bench supply rather than the Li-Po battery. Verify regulator outputs, startup sequencing, ripple, current draw and thermal behaviour before connecting the battery.
7. **Install display:** Confirm Sharp LS032B7DD02 flex orientation, pinout and supply rails. Never hot-plug the FPC.
8. **Install storage:** Use a compatible 128 GB microSDXC card. Firmware must support the selected filesystem; many 128 GB cards ship exFAT-formatted.
9. **Connect battery:** Only after battery polarity, protection, charger configuration, charge current and temperature sensing are verified against the cell datasheet.
10. **Functional validation:** Test display refresh, encoder direction/debounce, buttons, microSD read/write, USB-C operation, audio sample rates, headphone output and thermal performance.

## Firmware and storage notes

- The ESP32-S3 firmware must initialize the display, controls, microSD interface, audio pipeline, DAC and headphone amplifier.
- A 128 GB microSDXC card is commonly supplied as exFAT. Firmware must include compatible filesystem support or a documented reformat procedure.
- The microSD card is removable music/data storage; do not claim it is pre-flashed with firmware unless a specific image and flashing procedure are supplied. ESP32-S3 boot firmware normally resides in module flash.

## Project status

This is an actively developed hardware project. Prices and links are procurement references, not guarantees of stock, compatibility or production readiness. Electrical performance, power integrity, audio performance, thermal behaviour and enclosure fit require validation on the assembled hardware.

**Fabrication status: NOT APPROVED by this README/BOM alone.** Resolve all HOLD/VERIFY items, complete schematic and PCB review, and validate the Gerbers before ordering.

## License

Add the project license and any required third-party hardware/documentation attribution.