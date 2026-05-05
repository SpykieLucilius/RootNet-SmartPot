/*
 * SmartPot_Main.ino
 *
 * Sensor-module firmware for the smart modular pot project.
 * Targets the Wemos Lolin32 V1.0.0 (ESP32-based).
 *
 * Cycle:
 *   1. Wake from deep sleep (or initial boot)
 *   2. Read sensors: capacitive soil moisture, DHT11 temp/humidity,
 *      TCS34725 light intensity + colour
 *   3. Build a flat JSON payload
 *   4. Connect to Wi-Fi via WiFiMulti (multi-SSID, picks strongest)
 *   5. Resolve the server through mDNS (rootnet.local), fall back to a
 *      per-SSID IP table if mDNS is blocked on the network
 *   6. POST /readings
 *   7. Show readings on the OLED for a short window
 *   8. Power-down sequence (OLED off, Wi-Fi off, Serial.flush())
 *   9. esp_deep_sleep_start() for SLEEP_SECONDS
 *
 */

#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_TCS34725.h>
#include <DHT.h>

// ---------- Configuration ---------------------------------------------------

// Sleep duration between cycles (seconds). 600 = 10 minutes.
static const uint64_t SLEEP_SECONDS = 600ULL;

// How long to keep the OLED on after a successful cycle (milliseconds).
static const uint32_t OLED_AWAKE_MS = 15000UL;

// Wi-Fi connection timeout (milliseconds). Falls through to deep sleep on failure.
static const uint32_t WIFI_TIMEOUT_MS = 12000UL;

// HTTP timeout for the POST (milliseconds).
static const uint32_t HTTP_TIMEOUT_MS = 5000UL;

// Server discovery
static const char* SERVER_HOSTNAME = "rootnet";        // resolved as rootnet.local
static const uint16_t SERVER_PORT  = 8080;
static const char* SERVER_PATH     = "/readings";

// Status-line thresholds for the OLED
static const int   SOIL_DRY_THRESHOLD = 2500;
static const float TEMP_COLD          = 18.0f;
static const float TEMP_HOT           = 32.0f;
static const float LUX_LOW            = 200.0f;

// ---------- Pins ------------------------------------------------------------

static const int PIN_SDA      = 21;
static const int PIN_SCL      = 22;
static const int PIN_DHT      = 4;
static const int PIN_SOIL_ADC = 34;

// ---------- Peripherals -----------------------------------------------------

#define OLED_W 128
#define OLED_H 32
Adafruit_SSD1306 display(OLED_W, OLED_H, &Wire, -1);

Adafruit_TCS34725 tcs(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

// ---------- Wi-Fi networks --------------------------------------------------
//
// Each SSID/password pair will be tried by WiFiMulti, which picks whichever
// one is in range with the strongest signal. Add more here as needed.

WiFiMulti wifiMulti;

struct WifiCred {
  const char* ssid;
  const char* pass;
  const char* fallback_ip;   
};

static const WifiCred WIFI_NETWORKS[] = {
  { "home_wifi1",       "REPLACE_WITH_PASSWORD", "192.168.8.100" },
  { "home_wifi2",      "REPLACE_WITH_PASSWORD", "192.168.1.50"  },
  { "phone_hotspot",  "REPLACE_WITH_PASSWORD", "172.20.10.2"   },
};
static const size_t WIFI_NETWORKS_COUNT =
    sizeof(WIFI_NETWORKS) / sizeof(WIFI_NETWORKS[0]);

// ---------- Persistent state ------------------------------------------------

RTC_DATA_ATTR uint32_t bootCount = 0;

// ---------- Utility ---------------------------------------------------------

static void goToDeepSleep() {
  Serial.printf("[step] entering deep sleep for %llu s\n",
                (unsigned long long)SLEEP_SECONDS);

  // Power-down sequence. 
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.flush();

  esp_sleep_enable_timer_wakeup(SLEEP_SECONDS * 1000000ULL);
  esp_deep_sleep_start();
}

static bool connectWifi() {
  Serial.println("[step] connecting Wi-Fi");

  WiFi.mode(WIFI_STA);
  for (size_t i = 0; i < WIFI_NETWORKS_COUNT; i++) {
    wifiMulti.addAP(WIFI_NETWORKS[i].ssid, WIFI_NETWORKS[i].pass);
  }

  uint32_t start = millis();
  while (wifiMulti.run() != WL_CONNECTED) {
    if (millis() - start > WIFI_TIMEOUT_MS) {
      Serial.println("[warn] Wi-Fi connection timed out");
      return false;
    }
    delay(200);
  }

  Serial.printf("[ok] Wi-Fi connected to %s (%s)\n",
                WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  return true;
}

// Resolve the server: try mDNS first, fall back to a per-SSID IP table.
static String resolveServerHost() {
  if (MDNS.begin("smartpot")) {
    IPAddress addr = MDNS.queryHost(SERVER_HOSTNAME);
    if (addr != IPAddress(0, 0, 0, 0)) {
      Serial.printf("[ok] mDNS resolved %s.local -> %s\n",
                    SERVER_HOSTNAME, addr.toString().c_str());
      return addr.toString();
    }
  }

  // Fallback: look up the current SSID in the network table
  String currentSsid = WiFi.SSID();
  for (size_t i = 0; i < WIFI_NETWORKS_COUNT; i++) {
    if (currentSsid == WIFI_NETWORKS[i].ssid) {
      Serial.printf("[warn] mDNS failed; using fallback IP %s\n",
                    WIFI_NETWORKS[i].fallback_ip);
      return String(WIFI_NETWORKS[i].fallback_ip);
    }
  }

  Serial.println("[err] no server address available");
  return String();
}

// ---------- Sensor reads ----------------------------------------------------

struct Reading {
  int   soil_raw;
  float temperature;
  float humidity;
  float lux;
  uint16_t r, g, b, c;
  bool dhtOk;
};

static Reading readAllSensors() {
  Reading rd = {};

  // Soil (analogue)
  rd.soil_raw = analogRead(PIN_SOIL_ADC);

  // Light (TCS34725)
  tcs.getRawData(&rd.r, &rd.g, &rd.b, &rd.c);
  rd.lux = tcs.calculateLux(rd.r, rd.g, rd.b);

  // Temperature / humidity (DHT11)
  rd.temperature = dht.readTemperature();
  rd.humidity    = dht.readHumidity();
  rd.dhtOk = !(isnan(rd.temperature) || isnan(rd.humidity));
  if (!rd.dhtOk) {
    rd.temperature = 0.0f;
    rd.humidity    = 0.0f;
  }

  return rd;
}

// ---------- Payload + POST --------------------------------------------------

static String buildPayload(const Reading& rd) {
  StaticJsonDocument<384> doc;
  doc["mac_address"]   = WiFi.macAddress();
  doc["soil_moisture"] = rd.soil_raw;
  doc["temperature"]   = rd.temperature;
  doc["humidity"]      = rd.humidity;
  doc["light_lux"]     = rd.lux;
  doc["light_red"]     = rd.r;
  doc["light_green"]   = rd.g;
  doc["light_blue"]    = rd.b;

  String out;
  serializeJson(doc, out);
  return out;
}

static bool postReading(const String& host, const String& payload) {
  WiFiClient client;
  HTTPClient http;

  String url = "http://" + host + ":" + String(SERVER_PORT) + SERVER_PATH;
  Serial.printf("[step] POST %s\n", url.c_str());

  http.setTimeout(HTTP_TIMEOUT_MS);
  if (!http.begin(client, url)) {
    Serial.println("[err] http.begin failed");
    return false;
  }
  http.addHeader("Content-Type", "application/json");

  int status = http.POST(payload);
  http.end();

  Serial.printf("[%s] POST %d\n", (status >= 200 && status < 300) ? "ok" : "err",
                status);
  return (status >= 200 && status < 300);
}

// ---------- OLED ------------------------------------------------------------

static const char* statusLabel(const Reading& rd) {
  if (!rd.dhtOk) return "DHT err";
  if (rd.soil_raw > SOIL_DRY_THRESHOLD) return "Water needed";
  if (rd.temperature < TEMP_COLD) return "Too cold!";
  if (rd.temperature > TEMP_HOT)  return "Too hot!";
  if (rd.lux < LUX_LOW)           return "Needs light";
  return "Plant OK";
}

static void drawReadings(const Reading& rd) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.printf("T=%.1fC H=%.0f%%\n", rd.temperature, rd.humidity);
  display.printf("Soil=%d Lux=%.0f\n", rd.soil_raw, rd.lux);
  display.printf("#%lu %s\n", (unsigned long)bootCount, statusLabel(rd));

  display.display();
}

// ---------- Setup / Loop ----------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(50);

  bootCount++;
  Serial.printf("\n=== Wake cycle #%lu ===\n", (unsigned long)bootCount);

  // 12-bit ADC for the soil sensor
  analogReadResolution(12);

  // I2C for OLED + TCS34725
  Wire.begin(PIN_SDA, PIN_SCL);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[err] OLED init failed");
    // continue anyway - the cycle should still report data
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.printf("Boot #%lu\nWaking...\n", (unsigned long)bootCount);
    display.display();
  }

  // Light sensor
  if (!tcs.begin()) {
    Serial.println("[warn] TCS34725 not found");
  }

  // DHT11
  dht.begin();

  // Read sensors
  Serial.println("[step] reading sensors");
  Reading rd = readAllSensors();
  Serial.printf("T=%.1fC H=%.0f%% Soil=%d Lux=%.0f -> %s\n",
                rd.temperature, rd.humidity, rd.soil_raw, rd.lux,
                statusLabel(rd));

  // Show readings on OLED while we deal with the network
  drawReadings(rd);

  // Wi-Fi
  if (!connectWifi()) {
    Serial.println("[warn] no Wi-Fi this cycle, sleeping");
    delay(OLED_AWAKE_MS); 
    goToDeepSleep();
  }

  // Resolve server and POST
  String host = resolveServerHost();
  if (host.length() > 0) {
    String payload = buildPayload(rd);
    Serial.printf("[step] payload (%u bytes)\n", payload.length());
    postReading(host, payload);
  }

  // Hold the OLED on for a moment so a passer-by can read the values
  Serial.printf("[step] holding OLED for %lu ms\n", (unsigned long)OLED_AWAKE_MS);
  delay(OLED_AWAKE_MS);

  // Sleep
  goToDeepSleep();
}

void loop() {
  // Never reached: setup() always ends in deep sleep.
}
