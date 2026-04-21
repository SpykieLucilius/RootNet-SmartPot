// Define the analog pin connected to the soil moisture sensor
#define SOIL_PIN 34

void setup() {
  // Start serial communication at 115200 baud rate for debugging
  Serial.begin(115200);

  // Configure the ESP32 Analog-to-Digital Converter (ADC) to 12 bits
  // This means analog readings will range from 0 (0V) to 4095 (3.3V)
  analogReadResolution(12);

  Serial.println("\n--- Capacitive Soil Moisture Sensor Test ---");
}

void loop() {
  // Read the raw analog value from the sensor
  int soil_value = analogRead(SOIL_PIN);

  // Print the value to the Serial Monitor
  Serial.print("Soil Moisture Value: ");
  Serial.print(soil_value);

  // Basic threshold testing (tune these values based on your specific plant and soil)
  if (soil_value > 2500) {
    Serial.println("  -> DIRT DRY");
  } else if (soil_value < 1200) {
    Serial.println("  -> DIRT WET");
  } else {
    Serial.println("  -> DIRT OK");
  }

  // Wait 1 second before taking the next reading
  delay(1000);
}