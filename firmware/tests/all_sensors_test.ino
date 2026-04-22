#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_TCS34725.h>
#include <DHT.h>

// --- Configuration & Pin Definitions ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_ADDR    0x3C

#define SOIL_PIN     34
#define DHTPIN       4
#define DHTTYPE      DHT11

#define I2C_SDA      21
#define I2C_SCL      22

// --- Thresholds for Plant Status ---
const int SOIL_DRY_THRESHOLD = 2500;
const float TEMP_COLD_THRESHOLD = 15.0;

// --- Object Initialization ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Initialize Serial for debugging at 115200 baud
  Serial.begin(115200);
  
  // Initialize I2C bus for the Lolin32
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Initialize OLED Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("OLED initialization failed"));
    for(;;);
  }

  // Initialize Sensors
  tcs.begin();
  dht.begin();
  
  // Configure ESP32 ADC resolution to 12-bit (0-4095 range)
  analogReadResolution(12);

  // Initial Screen Setup
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("RootNet Initialized"));
  display.display();
  delay(1000);
}

void loop() {
  // 1. Read Light Intensity (I2C)
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  uint16_t lux = tcs.calculateLux(r, g, b);

  // 2. Read Soil Moisture (Analog)
  int soil_value = analogRead(SOIL_PIN);

  // 3. Read Climate Data (Digital 1-wire)
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Handle DHT reading errors
  if (isnan(temp) || isnan(hum)) {
    temp = 0.0;
    hum = 0.0;
  }

  // 4. Print Data to Serial Monitor
  Serial.print(F("Temp: ")); Serial.print(temp);
  Serial.print(F("C | Hum: ")); Serial.print(hum);
  Serial.print(F("% | Soil: ")); Serial.print(soil_value);
  Serial.print(F(" | Lux: ")); Serial.println(lux);

  // 5. Update OLED Display
  display.clearDisplay();
  display.setCursor(0, 0);
  
  // Line 1: Climate data
  display.printf("T:%.1fC  H:%.0f%%\n", temp, hum);
  
  // Line 2: Soil and Light values
  display.printf("Soil:%d  Lux:%d\n", soil_value, lux);
  
  // Line 3: Decision Logic based on thresholds
  display.println(F("---------------------"));
  if (soil_value > SOIL_DRY_THRESHOLD) {
    display.print(F("STATUS: Water Needed"));
  } else if (temp < TEMP_COLD_THRESHOLD) {
    display.print(F("STATUS: Too Cold!"));
  } else {
    display.print(F("STATUS: Plant OK"));
  }

  // Push buffer to the physical screen
  display.display();

  // Wait 2 seconds before the next update
  delay(2000);
}