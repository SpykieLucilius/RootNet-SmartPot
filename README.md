# RootNet-SmartPot

The RootNet-SmartPot is an ESP32-based modular sensing unit designed for intelligent indoor plant monitoring. It serves as the perception layer for the RootNet ecosystem, collecting environmental data and providing immediate local feedback.

## Features
- **Comprehensive Sensing**: Monitors soil moisture, ambient light (Lux/RGB), temperature, and humidity.
- **Local Feedback**: Features an SSD1306 OLED display for real-time status updates (e.g., "Needs Water", "Too Cold", "OK") without network dependency.
- **Power Efficiency**: Optimized for long-term battery operation using ESP32 Deep Sleep cycles.
- **Energy Harvesting**: Supports autonomous operation via a 0.5W solar panel and an integrated LiPo charging circuit.
- **Connectivity**: Transmits telemetry via Wi-Fi to a central RootNet server for data logging and remote monitoring.

## Hardware Components
- **Microcontroller**: Lolin32 V1.0.0 (ESP32)
- **Display**: SSD1306 OLED (0.91", 128x32)
- **Light Sensor**: TCS34725 RGB Color Sensor
- **Moisture Sensor**: Capacitive Soil Moisture Sensor V2.0
- **Climate Sensor**: DHT11 Temperature and Humidity Sensor
- **Energy**: 1000mAh LiPo Battery & 5V / 0.5W Solar Panel
- **Accessories**: 10kΩ Pull-up Resistor, Breadboard, and Jumper Wires

## Repository Structure
```text
RootNet-SmartPot/
├── docs/
│   ├── wiring_diagram.md
│   ├── build_guide.md
│   └── images/
├── firmware/
│   ├── SmartPot_Main/
│   │   └── SmartPot_Main.ino
│   └── tests/
│       ├── oled_test.ino
│       ├── light_test.ino
│       ├── soil_test.ino
│       ├── dht11_test.ino
│       └── all_sensor_test.ino
├── hardware/
│   └── device_list.md
└── README.md