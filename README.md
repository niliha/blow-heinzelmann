# Blow Heinzelmann

An interactive sound installation to play a random selection of sound samples.

<img src="https://github.com/niliha/blow-heinzelmann/assets/75397148/26e695e9-e464-467c-b0fb-a08f7f267a61" width="33%">

# Hardware Details

* [FireBeetle 2 ESP32 C6](https://wiki.dfrobot.com/SKU_DFR1075_FireBeetle_2_Board_ESP32_C6)
* [DFPlayer Mini MP3 Player](https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299)
* 3.7V 1000mAh Li-Po rechargeable battery
* [IXDD604PI MOSFET Gate Driver](https://www.littelfuse.com/products/integrated-circuits/gate-driver-ics/igbt-and-mosfet-driver-ics/low-side-gate-driver-ics/ixd_604.aspx)
* 3W 8 Ohm speaker

# Getting Started

* Clone this repository ``--recursive``ly.
* Install ESP-IDF extension for Visual Studio Code and install ESP-IDF 5.14. See [here](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/install.md) for further instructions.

# Operating Instructions

## Usage

Press the button to play a combination of a "blow" and "heinzelmann" sound sample.

## Rechargeable Battery

The included 3.7V 1000mAh Li-Po battery should suffice for several years of operation by leveraging the deep sleep mode of the ESP32 und controlling the power supply of the DFPlayer Mini using a gate driver as low-side switch.
It can be charged by supplying the FireBeetle 2 ESP32 C6 Development Board via USB-C or through the `VIN` pin, e.g. using a solar panel.

# Known Issues

The DFPlayer Mini is very sensitive when it comes to its power supply (3.2V - 5V).
Due to the use of the Gate Driver as low-side switch, the supply voltage coming from the ESP32 Dev Board (3.3V) drops a little bit.
This leads to the side effect that the DFPlayer Mini struggles to operate at full volume.
A workaround for this would be to use a 5V boost converter.
