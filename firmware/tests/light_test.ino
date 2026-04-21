#include <Wire.h>
#include <Adafruit_TCS34725.h>

// Initialize the color sensor with specific integration time and gain
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() {
  // Start serial communication at 115200 baud rate for debugging output
  Serial.begin(115200);
  
  // Initialize the I2C bus with SDA = 21 and SCL = 22 for the Lolin32 (ESP32)
  Wire.begin(21, 22);

  Serial.println("\n--- TCS34725 Light Sensor Test ---");

  // Check if the sensor is connected and responding on the I2C bus (Address 0x29)
  if(!tcs.begin()) {
    Serial.println("Error: TCS34725 sensor not detected. Please check your wiring.");
    while(1); // Infinite loop to halt execution if the sensor is missing
  }
  
  Serial.println("Sensor successfully initialized.");
}

void loop() {
  // Variables to store red, green, blue, and clear (unfiltered) light values
  uint16_t r, g, b, c;
  
  // Read the raw sensor data
  tcs.getRawData(&r, &g, &b, &c);
  
  // Calculate the illuminance in Lux based on the RGB values
  uint16_t lux = tcs.calculateLux(r, g, b);

  // Print the results to the Serial Monitor
  Serial.print("Illuminance (Lux): "); 
  Serial.print(lux);
  Serial.print("  |  Raw Colors -> R: "); Serial.print(r);
  Serial.print(" G: "); Serial.print(g);
  Serial.print(" B: "); Serial.println(b);
  
  // Wait for 500 milliseconds before taking the next reading
  delay(500);
}