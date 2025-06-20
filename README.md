# GoControl - ESP32 GoPro remote control

Project start date: 19.10.2024

![Overview picture](https://github.com/sdebby/GoControl/blob/main/media/intro.jpg?raw=true)

## Overview

Building a cheap GoPro camera remote control, using Open GoPro API
 
## MRD (Marketing requirement)

 - Creating a simple -easy to use, BLE remote control for GoPro Cameras.
 - up to 2 buttons and LED for operation Indication.
 - Mobile and small size (similar to car remote)
 
### User indications

The remote will indicate the following using build in led:

* On connection - Green LED
* On disconnection - Red LED
* On sending Action Start command - Blue LED
* On sending Camera mode command - Yellow LED
 
## Skill set

 * BLE knowledge.
 * PCB editing.
 * 3D design.
 * 3D printing.
 * Arduino and C++ programming.
 
## BOM (Bill Of Materials):

1. ESP32 C6 dev kit
2. 2 X tact switches.
3. 2 X 330 Ohm resistors.
4. [Charger panel](https://www.aliexpress.com/item/1005005037876729.html?spm=a2g0o.order_list.order_list_main.386.6b881802CcDzO8)
5. LiPo battery 120 mAh.
   
**For R&D:**
1. Wiring.
2. breadboard.

**For production:**
1. PCB manufacturing.
2. 3D printing for enclosure.

## Software:

1. Arduino IDE for coding and uploading to ESP32 C6 board.
2. KiCad for PCB design.
3. Fusion360 for 3D enclosure design

## TSD (Technical Specification)

* Using a BLE to control GoPro camera
[OpenGoPro](https://gopro.github.io/OpenGoPro/ble/index.html)
* Running on ESP32 C6 dev board

## Scheme

1. FOR THE FIRST TIME, AND AFTER INSTALLING NEW FIRMWARE: Insert GoPro camera into [paring model](https://community.gopro.com/s/article/GoPro-Quik-How-To-Pair-Your-Camera?language=en_US#HERO2024)
2. Remote will scan for BLE devices.
3. Remote will connect to the first device that advertise GoPro `serviceUUID` (0xFEA6)
4. On fail connection / disconnection, remote will scan for BLE devices again.
5. Remote will stop scanning and go to deep sleep mode after 3 failed scans.
   1. Remote will wake up on button press.
OnConnection to GoPro camera:
1. Remote will turn BUILTIN LED to Green.
2. On pressing button 1, remote will send Action start command to `COMMAND_REQ_UUID`
3. On releasing button 2, remote will send Action stop command to `COMMAND_REQ_UUID`
4. On pressing button 2, remote will rotate between PhotoMode / VideoMode / TimelapsMode, by sending the correct request to  `COMMAND_REQ_UUID`
5.  The remote will listen to camera reply by registering a listener on  `COMMAND_RSP_UUID` service
OnDisconnection from GoPro camera:
1. Remote will turn BUILTIN LED to Red.
2. Remote will start scanning for GoPro devices again.
3. Remote will go to deep sleep mode after 3 failed scans.


## Schematics

![Scheme](https://github.com/sdebby/GoControl/blob/main/media/ESP32%20C6%20GoControl_bb.jpg?raw=true)


## PCB

* PCB design using KiCad
* 2 layers PCB
* PCB manufacturing using PCBWay
  
![PCB](https://github.com/sdebby/GoControl/blob/main/media/ESP32%20C6%20GoControl_pcb.png?raw=true)

## Hardware assembly:

![Welding components to PCB:](https://github.com/sdebby/GoControl/blob/main/media/IMG_9036.jpg?raw=true)

![Welding components to PCB](https://github.com/sdebby/GoControl/blob/main/media/IMG_9035.jpg?raw=true)

![Fixing PCB wrong wiring:](https://github.com/sdebby/GoControl/blob/main/media/IMG_9045.jpg?raw=true)

![Adding cut on PCB to fix ease assembly:](https://github.com/sdebby/GoControl/blob/main/media/IMG_9056.jpg?raw=true)

![Final PCB assembly:](https://github.com/sdebby/GoControl/blob/main/media/IMG_9057.jpg?raw=true)

### 3D enclosure design

* using Fusion360
* 3D print using Ender 5 plus with PLA filament.
![3D enclosure design](https://github.com/sdebby/GoControl/blob/main/media/Fusion360.jpg?raw=true)

## Video

[![](https://markdown-videos-api.jorgenkh.no/youtube/bjMAEX7DmnI)](https://youtu.be/bjMAEX7DmnI)


## R&D

Coin cell battery capacity:

| Battery | Capacity |
| ------- | -------- |
|  2016 | 100 mAh |
|  2025 | 170 mAh |
|  2032 | 235 mAh |
|  2450 | 620 mAh |

* Using 2032 battery did not work, as it was not able to power the ESP32 C6 board, I had to use LiPo battery. with 1200 mAh capacity.

### Code fixing

Fixing arduino not displaying BLE device names:
[Fixing library dont support short names](https://github.com/nkolban/esp32-snippets/issues/871)
the file `BLEAdvertisedDevice.cpp` in library :
c:\Users\user1\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.0.5\libraries\BLE\src\

### Deep sleep

1. deep sleep:
	1. [espressif doc](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/system/sleep_modes.html)
	2. Wake up on button can be use only on RTC pins
	3. On ESP32-C6: RTC pins are 0-7

## Feedback

If you have any feedback, please reach out to us at mailto:shmulik.debby@gmail.com
