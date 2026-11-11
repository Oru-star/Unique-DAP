# Unique DAP

Unique Dap is An ESP32-S3-based audio player , which uses a unique Sharp reflective memory display instead of normal lcd pannel ,it has a dedicated audio DAC and headphone amplifier built in itself , and removable microSD storage and it can be navigated using the two rotary encoders at the bottom and charges using the type c port below 



<p align="center">
  <img src="RENDERS'n'Images/HOME4.png" alt="Three-quarter render of the Unique DAP enclosure" width="800">
</p>

## Project at a glance

- **Controller:** ESP32-S3 module
- **Audio:** ES9038Q2M DAC and MAX97220A headphone amplifier
- **Display:** Sharp 3.16-inch reflective memory LCD
- **Storage:** microSD socket with a 128 GB card 
- **Design files:** KiCad PCB and schematics, Gerbers, and printable enclosure parts

## Repository contents

| Resource | Description |
|---|---|
| [KiCad PCB](<DAP'S PCB/DAP'S PCB.kicad_pcb>) | Main board layout |
| [KiCad schematic](<DAP'S PCB/DAP'S PCB.kicad_sch>) | Main circuit schematic |
| [Gerber files](./Gerbers/) | For pcb manufacturing  |
| [Printable enclosure parts](<steps/Stl for prints/>) | For printing the project |



## DAP's images 

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

## Assembly steps

> **Prototype status:** The BOM still has HOLD and VERIFY items, including incorrectly sized power-inductor footprints. Resolve these design and footprint issues, then rerun KiCad ERC/DRC and update the manufacturing files before ordering or powering the board.

1. **Prepare the parts** —  check exact part numbers and footprints from my repo,the USB-C receptacle port , switches/encoders and display connector. The BOM contains a single PCB design.

2. **Fabricate and inspect the PCB** —  Confirm Gerbers in [`DAP'S PCB/Gerbers`](<DAP'S PCB/Gerbers/>) Check if everything is correct or not and then send it for the fab. Check delivered board for damage, solder-mask defects or any shorts and correct connector openings before assemblling the board .
3. **Populate the board** — Place the SMD parts and ICs solder them using the hotplate or any similar thing (stencil is recommended), then the through-hole connectors and components , following the board layout . check the solder joints if everything looks correct
4. **Inspect before anything** — Check for solder bridges and shorts
5. **check the enclosure** — test if everything is fitting in the enclosure and if anything is not fitting you can even use the tools to make it fit 
6. **Install the display** — Verify the Sharp LS032B7DD02 flex cable pinout and  orientation, and then carefully connect it with the main pcb 
7. **Connect storage ** — Insert the microSD card into the sdcard slot we just added and make sure that the sdcard has the firmware flashed already i gave .
8. **Connect the battery and test power** — turn on the dap and check if everything is working properly as mentioned!!


## Bill of materials

Full parts list: [Bom.csv](./Bom.csv).

**Known-priced subtotal: ₹11,412.70 INR** (22 of 38 items priced; excludes unpriced parts, tax, shipping, customs, PCB fabrication and assembly).

<details>
<summary>Show BOM (38 items)</summary>

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

</details>
