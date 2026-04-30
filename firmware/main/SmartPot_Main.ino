#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_TCS34725.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "esp_sleep.h"

// ---------- Wi-Fi config ----------
const char* WIFI_SSID_1 = "Nothing Phone(3)_1513";
const char* WIFI_PASS_1 = "HIDDEN_FOR_SECURITY_REASONS";
const char* WIFI_SSID_2 = "moaiwlan";
const char* WIFI_PASS_2 = "HIDDEN_FOR_SECURITY_REASONS";

// ---------- Server config ----------
const uint16_t SERVER_PORT = 8080;

struct ServerOverride { const char* ssid; const char* ip; };
const ServerOverride SERVER_OVERRIDES[] = {
  { "Nothing Phone(3)_1513", "10.117.201.144" },
  { "moaiwlan",                "10.3.119.21"     },
};
const int SERVER_OVERRIDES_COUNT = sizeof(SERVER_OVERRIDES) / sizeof(SERVER_OVERRIDES[0]);

// ---------- Sleep / awake timing ----------
#define SLEEP_SECONDS         600
#define OLED_DISPLAY_SECONDS   15 

// ---------- Sensor / display setup ----------
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   32
#define OLED_I2C_ADDR   0x3C
#define DHT_PIN         4
#define DHT_TYPE        DHT11
#define SOIL_PIN        34
#define BATTERY_PIN     35

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS,
                                          TCS34725_GAIN_4X);
DHT dht(DHT_PIN, DHT_TYPE);
WiFiMulti wifiMulti;

// ---------- RTC memory ----------
RTC_DATA_ATTR uint32_t bootCount = 0;

// ---------- Moruga Yellow thresholds ----------
const int   SOIL_DRY      = 2500;
const float TEMP_COLD     = 18.0;
const float TEMP_HOT      = 32.0;
const float LUX_LOW       = 200.0;

// ---------- Helpers ----------
String resolveServerURL() {
  IPAddress ip = MDNS.queryHost("rootnet");
  if (ip != IPAddress(0, 0, 0, 0)) {
    return "http://" + ip.toString() + ":" + String(SERVER_PORT);
  }
  String ssid = WiFi.SSID();
  for (int i = 0; i < SERVER_OVERRIDES_COUNT; i++) {
    if (ssid == SERVER_OVERRIDES[i].ssid) {
      return "http://" + String(SERVER_OVERRIDES[i].ip) + ":" + String(SERVER_PORT);
    }
  }
  return "";
}

float readBatteryVoltage() {
  int raw = analogRead(BATTERY_PIN);
  return (raw / 4095.0f) * 3.3f * 2.0f;
}

const char* statusFromReadings(int soil, float temp, float lux) {
  if (soil > SOIL_DRY)         return "Water needed";
  if (temp < TEMP_COLD)        return "Too cold!";
  if (temp > TEMP_HOT)         return "Too hot!";
  if (lux < LUX_LOW)           return "Needs light";
  return "Plant OK";
}

void drawOLED(uint32_t cycle, float t, float h, int soil, float lux,
              float bat, const char* status) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.printf("T:%.1fC H:%.0f%% #%lu", t, h, (unsigned long)cycle);
  display.setCursor(0, 11);
  display.printf("Soil:%d Lux:%.0f", soil, lux);
  display.setCursor(0, 22);
  display.printf("%-12s %.2fV", status, bat);

  display.display();
}

void oledOff() {
  display.ssd1306_command(SSD1306_DISPLAYOFF);
}

void goToSleep() {
  Serial.printf("Sleeping for %d s\n", SLEEP_SECONDS);
  Serial.flush();
  oledOff();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_SECONDS * 1000000ULL);
  esp_deep_sleep_start();
  // execution never reaches here
}

// ---------- Single-shot main ----------
void setup() {
  Serial.begin(115200);
  delay(200);

  bootCount++;
  Serial.printf("\n=== Wake cycle #%u ===\n", bootCount);

  // I2C + ADC
  Wire.begin(21, 22);
  analogReadResolution(12);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("OLED init failed");
    goToSleep();
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("RootNet SmartPot");
  display.printf("Cycle #%u\n", bootCount);
  display.display();

  // Sensors
  if (!tcs.begin()) Serial.println("TCS34725 not found");
  dht.begin();
  delay(1500);

  // ---- 1. Read sensors ----
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  float lux = tcs.calculateLux(r, g, b);

  int soil = analogRead(SOIL_PIN);

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  if (isnan(temp)) temp = 0;
  if (isnan(hum))  hum  = 0;

  float battery = readBatteryVoltage();
  const char* status = statusFromReadings(soil, temp, lux);

  Serial.printf("T=%.1fC H=%.0f%% Soil=%d Lux=%.0f Bat=%.2fV  -> %s\n",
                temp, hum, soil, lux, battery, status);

  // ---- 2. Update OLED with real readings ----
  drawOLED(bootCount, temp, hum, soil, lux, battery, status);

  // ---- 3. Wi-Fi + POST ----
  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);

  Serial.print("Wi-Fi");
  unsigned long start = millis();
  while (wifiMulti.run() != WL_CONNECTED && millis() - start < 10000) {
    Serial.print(".");
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf(" connected to %s (%s)\n",
                  WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    MDNS.begin("smartpot");
    delay(200);  // give mDNS a moment

    String url = resolveServerURL();
    if (url.length() > 0) {
      url += "/readings";

      StaticJsonDocument<512> doc;
      doc["mac_address"]     = WiFi.macAddress();
      doc["soil_moisture"]   = soil;
      doc["temperature"]     = temp;
      doc["humidity"]        = hum;
      doc["light_lux"]       = lux;
      doc["light_red"]       = r;
      doc["light_green"]     = g;
      doc["light_blue"]      = b;
      doc["battery_voltage"] = battery;

      String body;
      serializeJson(doc, body);

      HTTPClient http;
      http.begin(url);
      http.addHeader("Content-Type", "application/json");
      http.setTimeout(5000);

      int code = http.POST(body);
      Serial.printf("POST %d\n", code);
      http.end();
    } else {
      Serial.println("Server not resolvable on this network");
    }
  } else {
    Serial.println(" no network");
  }

  // ---- 4. Keep the OLED on for the rest of the awake window ----
  unsigned long awakeMs = millis();
  while (millis() - awakeMs < (unsigned long)OLED_DISPLAY_SECONDS * 1000UL) {
    delay(100);
  }

  // ---- 5. Sleep ----
  goToSleep();
}

void loop() {
   // never runs
}