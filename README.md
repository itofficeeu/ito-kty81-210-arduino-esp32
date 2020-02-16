# ito-kty81-210-arduino-esp32
Example program that continous reads the temperature determined by a KTY81-210, 
and outputs it in the Arduino monitor, alias terminal.

The software is written with adaptability in mind,
and shows the calculations behind the temperature output
in a way where it is possible to understand and modify these.

The precision is dependent on your own calibration.


## Software

C/C++ in the Arduino IDE

Author: Andreas Chr. Dyhrberg @ itoffice.eu

Licens: This software may be distributed and modified under the terms of the 
GNU General Public License version 2 (GPL2) as published by the Free Software 
Foundation and appearing in the file GPL2.TXT included in the packaging of 
this file. Please note that GPL2 Section 2[b] requires that all works based 
on this software must also be made publicly available under the terms of the 
GPL2 ("Copyleft").


## Hardware

A KTY81-210 mounted in the Sensor Adapter Switchboard from https://itoffice.eu/ 

The Sensor Adapter Switchboard is mounted on the Minimal-X Extension Board from https://itoffice.eu/

ESP32 in a Heltec WiFi Lora 32 that runs on top of the Minimal-X Extension Board.

See the foto for an overview of the hardware mentioned, or visit the web site https://itoffice.eu/ for further inspiration.


## The sensor module "Sensor Adapter Switchboard"
alias the circuit around the KTY81. Setup as a voltage divider like this:

 (Vcc 5+)--+-->3k--+-->kty81-210---->(GND)

                   |

                   +-----> ADC (Analog Port of the microcontroller)
