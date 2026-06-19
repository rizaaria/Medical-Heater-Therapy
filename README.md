# Medical Heater Therapy

## Overview

Medical Heater Therapy is a wearable heat therapy device designed to provide controlled thermal treatment for muscle relaxation and pain relief.

The project was developed in two versions:

- **V1:** Arduino Uno + 2.4" TFT Touchscreen Shield + Dual DS18B20
- **V2:** ESP32 + 2.8" Touchscreen LCD + DS18B20

The system utilizes a nichrome-wire heater, battery-powered operation, temperature monitoring, and touchscreen control.

---

## Features

- Real-time temperature monitoring
- Touchscreen user interface
- Adjustable therapy duration
- Automatic heater control
- Overheat protection
- Battery-powered operation
- Wearable design

---

## Hardware

### Version 1

- Arduino Uno
- 2.4" TFT Touchscreen Shield
- 2 × DS18B20
- Nichrome Wire Heater
- MOSFET Driver
- Buzzer

### Version 2

- ESP32
- 2.8" Touchscreen LCD
- DS18B20
- Nichrome Wire Heater
- MOSFET Driver
- Buzzer

### Power System

- 3 × Li-ion Battery
- BMS 3S
- LM2596 Buck Converter

---

## Working Principle

The nichrome-wire heater generates heat while the DS18B20 continuously monitors temperature. The microcontroller automatically controls the heater and displays system information on the touchscreen interface.

---

## Technologies Used

- ESP32
- Arduino Uno
- Embedded C/C++
- DS18B20
- TFT Touchscreen LCD
- Power Electronics
- Wearable Device Design

---

## Disclaimer

This project was developed for educational, research, and prototype purposes. It is not intended to replace certified medical devices or professional medical diagnosis.
