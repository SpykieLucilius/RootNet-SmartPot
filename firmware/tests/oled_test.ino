#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define OLED dimensions and I2C pins for the ESP32
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define I2C_SDA 21
#define I2C_SCL 22
#define OLED_ADDR 0x3C

// Initialize the display object (-1 means no dedicated reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

  // Initialize the I2C bus with the specific SDA and SCL pins
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize the OLED display
  // SSD1306_SWITCHCAPVCC tells it to generate display voltage internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("Error: SSD1306 display not found"));
    for(;;); // Infinite loop to halt execution if initialization fails
  }

  // Clear any existing data in the display buffer
  display.clearDisplay();

  // Configure text properties
  display.setTextSize(1);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text on black background
  display.setCursor(0, 0);             // Start drawing at the top-left corner

  // Write text to the display's memory buffer
  // The F() macro saves RAM by storing strings in flash memory
  display.println(F("Hello world!"));

  // Push the memory buffer to the physical screen to make it visible
  display.display();
}

void loop() {
  // Nothing to do here for a static text test
}