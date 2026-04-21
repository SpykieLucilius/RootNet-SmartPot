# List of Devices: RootNet-SmartPot Project

## 1. Lolin32 V1.0.0 (ESP32-based Microcontroller)
* **Role:** Central processing unit for sensor telemetry collection, local display control, and Wi-Fi transmission.
* **Key Features:** Dual-core processor, integrated Wi-Fi/Bluetooth, and a built-in LiPo charging circuit with battery monitoring on GPIO 35.

## 2. SSD1306 OLED Display (0.91")
* **Role:** Local interface for real-time status updates and sensor readings.
* **Key Features:** 128x32 pixel resolution (monochrome), I2C interface, and ultra-low power consumption.

## 3. TCS34725 RGB Color Sensor
* **Role:** Measures ambient light intensity (Lux) and color temperature to assess environmental lighting.
* **Key Features:** High dynamic range, integrated IR-blocking filter, and I2C communication sharing the same bus as the display.

## 4. Capacitive Soil Moisture Sensor V2.0
* **Role:** Measures the dielectric constant of the soil to estimate moisture levels.
* **Key Features:** Capacitive sensing technology (prevents electrode corrosion), 3.3V compatible, and analogue output to GPIO 34.

## 5. DHT11 Temperature and Humidity Sensor
* **Role:** Monitors ambient climate conditions (temperature and humidity) surrounding the plant.
* **Key Features:** Digital sensing via a proprietary single-wire protocol on GPIO 4.

## 6. 10kΩ Resistor
* **Role:** Pull-up resistor for the DHT11 data line.
* **Key Features:** Stabilizes digital communication by preventing the signal line from floating.

## 7. Breadboard and Jumper Wires
* **Role:** Main prototyping platform for circuit assembly and electrical verification.
* **Key Features:** Facilitates solderless component testing and easy hardware iterations.

## 8. Power & Solar Energy System
* **Role:** Provides autonomous power management for long-term remote monitoring.
* **Key Features:** * **Solar Panel:** 5V / 0.5W panel used for energy harvesting.
    * **Battery:** ~1000 mAh LiPo battery for power buffering and overnight operation.
    * **Charging:** The solar panel feeds the Lolin32's onboard TP4054-based charging circuit, which automatically manages the LiPo charging cycle.