#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>

WiFiMulti wifiMulti;

const char* WIFI_SSID_1 = "Nothing Phone(3)_1513";
const char* WIFI_PASS_1 = "HIDDEN_FOR_SECURITY_REASONS";
const char* WIFI_SSID_2 = "moaiwlan";
const char* WIFI_PASS_2 = "HIDDEN_FOR_SECURITY_REASONS";

const uint16_t SERVER_PORT = 8080;
const uint32_t POST_INTERVAL_MS = 10000; 

String resolveServerURL() {
  IPAddress ip = MDNS.queryHost("rootnet");
  if (ip != IPAddress(0, 0, 0, 0)) {
    return "http://" + ip.toString() + ":" + String(SERVER_PORT);
  }
  return "";
}

void setup() {
  Serial.begin(115200);
  delay(500);

  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);

  Serial.print("Connecting Wi-Fi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print(" connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  if (!MDNS.begin("smartpot")) {
    Serial.println("mDNS init failed");
  }
}

void loop() {
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost, will retry on next cycle");
    delay(POST_INTERVAL_MS);
    return;
  }

  String url = resolveServerURL();
  if (url.length() == 0) {
    Serial.println("Server not found via mDNS, skipping cycle");
    delay(POST_INTERVAL_MS);
    return;
  }
  url += "/readings";

  // Build a hardcoded JSON payload
  StaticJsonDocument<512> doc;
  doc["mac_address"]      = WiFi.macAddress();
  doc["soil_moisture"]    = 1850;
  doc["temperature"]      = 23.4;
  doc["humidity"]         = 52.0;
  doc["light_lux"]        = 380.5;
  doc["light_red"]        = 95;
  doc["light_green"]      = 130;
  doc["light_blue"]       = 80;
  doc["battery_voltage"]  = 3.95;

  String body;
  serializeJson(doc, body);

  Serial.print("POST ");
  Serial.println(url);
  Serial.print("Body: ");
  Serial.println(body);

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  int code = http.POST(body);
  if (code > 0) {
    Serial.printf("HTTP %d: %s\n", code, http.getString().c_str());
  } else {
    Serial.printf("POST failed: %s\n", http.errorToString(code).c_str());
  }
  http.end();

  delay(POST_INTERVAL_MS);
}