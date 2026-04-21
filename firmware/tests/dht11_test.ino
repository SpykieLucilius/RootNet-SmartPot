#include <DHT.h>

// Define the digital pin connected to the DHT11 data pin
#define DHTPIN 4

// Define the exact type of DHT sensor being used
#define DHTTYPE DHT11

// Initialize the DHT sensor object
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Start serial communication at 115200 baud rate for debugging output
  Serial.begin(115200);

  Serial.println("\n--- DHT11 Temperature & Humidity Sensor Test ---");

  // Initialize the sensor to start taking readings
  dht.begin();
}

void loop() {
  // Wait 2 seconds between measurements. 
  // This is required because the DHT11 is a slow sensor.
  delay(2000);

  // Read temperature in Celsius (default)
  float temp = dht.readTemperature();
  
  // Read relative humidity as a percentage
  float hum = dht.readHumidity();

  // Check if any reads failed (returns NaN: Not a Number)
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Error: Failed to read from DHT sensor! Check wiring.");
    return; // Skip the rest of the loop and try again
  }

  // Print the valid results to the Serial Monitor
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print("%  |  Temperature: ");
  Serial.print(temp);
  Serial.println("°C");
}