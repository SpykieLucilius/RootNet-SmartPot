# Build Guide: RootNet SmartPot

This guide explains how to assemble the SmartPot sensor module.

## Prerequisites
* Basic soldering skills (for the TCS34725 pins).
* Arduino IDE installed with the following libraries:
  * `Adafruit SSD1306` & `Adafruit GFX`
  * `Adafruit TCS34725`
  * `DHT sensor library` (by Adafruit)

## Step 1: Pin Soldering
If your **TCS34725** (Light Sensor) came without pins:
1. Place the header pins into a breadboard to keep them steady.
2. Place the sensor PCB on top of the pins.
3. Solder each pin carefully at ~350°C.

## Step 2: Breadboard Assembly
1. **Power Rails:** Connect the Lolin32 `3.3V` and `GND` to the breadboard power rails.
2. **I2C Bus:** Connect the **SSD1306 OLED** and the **TCS34725** in parallel to `GPIO 21` (SDA) and `GPIO 22` (SCL).
3. **Soil Sensor:** Connect the **Capacitive Soil Sensor** to `GPIO 34`.
4. **DHT11:** Connect the **DHT11** to `GPIO 4`. 
   * *Critical:* Place a **10kΩ resistor** between the VCC and DATA pins of the DHT11.

## Step 3: Firmware Upload
1. Open `SmartPot_Main.ino` from the `firmware/` folder.
2. Select Lolin32 in the Arduino IDE.
3. Upload the code and check the Serial Monitor (115200 baud).

## Step 4: Verification
* Ensure the OLED displays "Plant OK" or a specific status.
* Check if values update when you cover the light sensor or touch the soil probe.