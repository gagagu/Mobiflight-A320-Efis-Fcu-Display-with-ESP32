# Mobiflight-A320-Efis-Fcu-Display-with-ESP32
A320 Efis/Fcu Display for Mobiflight with Esp32

## This project ist still in development!

The goal of this project is to create an A320 Efis and Fcu controlled by Mobiflight. I want to use it with Fexis A320 for MSFS 2020 but for make it easier on development i have used the A320 from FlyByWire first.

## old The Idea
Mobiflight (today) is not able to control Oled Displays so it's not possible to create Displays like neede for FCU or EFIS with different graphic content. But Mobiflight is able to use Text LCD Display. May idea was/is to use an Arduino to emulate an LCD Display but instead of sending text from Mobiflight to the display i will send commands. The Arduino receives the commands and sets the content of the Oled Display corresponding to the command.

For Example:
In Flightsim we pull the baro knob. Mobiflight detect this and send an command "setSTD" to the LCD Display by I2c bus. The Arduino receives the command and print "STD" to the Oled Display.

### In the project directories are always two subdirectories, "EfisOnly" and "EfisAndFcu". The first one is used the "old" idea. The second one is used the "new" idea (see below).

## the new Idea
With the new (in beta yet) Mobiflight feature for generic I2C Devices the emulation of the LCD Display is not needed anyore. There is now a direct i2c communication between the ESP32 and the Arduino Mega 2560 (Mobliblight).
This makes the code and Mobiflight handling easier.

## The Reality
In reality a Ardunio cannot be i2c bus master and slave, so it wasn't possible to communicate with Mobiflight and the Oled Display on one bus. I have made some tests to use the hardware i2c bus for Mobiflight communication and a Software emulated i2c bus for the Oled Display. It work.... (see: https://github.com/gagagu/Mobiflight-A320-Efis-Display-with-Arduino)
But not really good. It's too slow.

Thats the reason why I switched to an ESP32 board, which has two hardware i2c interfaces.
I've made my first tests only with one Oled Display for the left Efis and everything works fine.

Now, I am working on an extension to realize left and right Efis and the Fcu with one Esp32, for the Displays. Because the displays used all the same i2c address i have to add an i2c multiplexer to controll all displays.
You will need nearly three Arduino Mega 2560 for all Efis and Fcu Buttons and LED's to control. Yes, you can use Portexpander to use one Arduino Mega, but why. The expander is not much cheaper than an additionally used Arduino Mega....

## Current State
The code is nearly finished and the displays are working. For easier testing i have used the Fly By Wire A320 with Mobiflight and i am using the Mobiflight 10

## Installation
- Upload the EfisAndFcu.ino to yout ESP32 with Arduino Studio
- download the latest Mobiflight from GitHub (https://github.com/MobiFlight/MobiFlight-Connector/releases)
- download the lastest mobiflight_genericI2C_mega_<version>.hex from Mobiflight Firmware Source (https://github.com/MobiFlight/MobiFlight-FirmwareSource/releases) and copy it to the Mobniflight/Firmware directory
- download the mobiflight.gagagu_efis_fcu.device.json from my Repository (https://github.com/gagagu/Mobiflight-A320-Efis-Fcu-Display-with-ESP32/tree/main/Mobiflight/EfisAndFcu/Devices) and copy it to the Mobiflight/Devices directory.
- mobiflight_genericI2C_mega.board.json
- Start Mobiflight
- Upload the Mobiflight GenerisI2C Mega Firmware für the FCU Arduino
- Upload the FCU.mfmc (https://github.com/gagagu/Mobiflight-A320-Efis-Fcu-Display-with-ESP32/tree/main/Mobiflight/EfisAndFcu) Config to the FCU Arduino
- I am using a second Arduino Mega for the EFIS Knops and buttons (not display) and upload the "normal" Mobiflight firmware to ut and upload the EFIS_LEFT.mfmc to it.
- Open the  FBW_A320_EfisAndFcu.mcc (https://github.com/gagagu/Mobiflight-A320-Efis-Fcu-Display-with-ESP32/tree/main/Mobiflight/EfisAndFcu) in Mobiflight and push "run" (MSFS 2020 with FlyByWire A320 shoudl be running).


## Where are the stl's
I will create a folder with all stl files, soon. Or i will release them on Thingiverse and post the link here.

![alt_text](https://github.com/gagagu/Mobiflight-A320-Efis-Fcu-Display-with-ESP32/blob/main/Fritzing/Fritzing-Schematic.png)

## Used designs
https://www.printables.com/de/model/494654-airbus-a320-efis-baro-knob

https://cults3d.com/en/3d-model/game/rotary-switch-5-x-45

https://www.thingiverse.com/thing:5253832

