# Wiring Diagram: RootNet SmartPot

The RootNet SmartPot uses a combination of I2C, Analogue, and One-Wire protocols. All components are powered by the Lolin32 3.3V output.

## Pin Mapping Table

| Component | Sensor Pin | Lolin32 Pin | Protocol |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | VCC / GND | 3.3V / GND | Power |
| | SDA / SCL | GPIO 21 / 22 | I2C |
| **TCS34725** | VIN / GND | 3.3V / GND | Power |
| | SDA / SCL | GPIO 21 / 22 | I2C (Shared) |
| **Soil Sensor** | VCC / GND | 3.3V / GND | Power |
| | AOUT | GPIO 34 | Analogue |
| **DHT11** | VCC (Pin 1) | 3.3V | Power |
| | DATA (Pin 2)| GPIO 4 | One-Wire |
| | GND (Pin 4) | GND | Power |

## Special Wiring Notes

### I2C Shared Bus
Both the OLED display and the Light Sensor share the same physical wires for SDA and SCL. The ESP32 distinguishes them via their hardware addresses:
* **OLED:** 0x3C
* **TCS34725:** 0x29

### DHT11 Pull-up Resistor
A 10kΩ resistor must be connected between VCC (Pin 1) and DATA (Pin 2) of the DHT11 to ensure stable digital communication.

### Internal Battery Monitoring
The Lolin32 V1.0.0 monitors battery voltage internally via a voltage divider connected to GPIO 35. No external wiring is required for this feature.