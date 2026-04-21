# RootNet-SmartPot

This repository contains the C++ (Arduino) firmware and hardware specifications for the RootNet SmartPot, an ESP32-based modular edge device designed for optimized indoor plant monitoring. The SmartPot integrates multiple sensors to provide real-time data on soil moisture, ambient light, temperature, and humidity, all displayed on an OLED screen. The device is designed for easy assembly and maintenance, making it ideal for plant enthusiasts and smart gardening applications.

## Wiring 

### 1. OLED Display (SSD1306 - I2C)
* **VCC** ➔ **3.3V** pin
* **GND** ➔ **GND** pin
* **SDA** ➔ **GPIO 21**
* **SCL** ➔ **GPIO 22**

### 2. Light Sensor (TCS34725 - I2C)
* **VIN** ➔ **3.3V** pin
* **GND** ➔ **GND** pin
* **SDA** ➔ **GPIO 21** *(Shared with OLED)*
* **SCL** ➔ **GPIO 22** *(Shared with OLED)*

### 3. Soil Moisture Sensor (Capacitive V2.0)
* **VCC** ➔ **3.3V** pin
* **GND** ➔ **GND** pin
* **AOUT** ➔ **GPIO 34**

### 4. Temperature & Humidity Sensor (4-pin DHT11)
* **VCC** ➔ **3.3V** pin
* **DATA** ➔ **GPIO 4**
* **GND** ➔ **GND** pin
* Place a 10kΩ pull-up resistor connecting VCC and DATA together.