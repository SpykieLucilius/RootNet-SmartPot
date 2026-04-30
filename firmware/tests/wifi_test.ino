#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>

WiFiMulti wifiMulti;

const char* WIFI_SSID_1 = "Nothing Phone(3)_1513";
const char* WIFI_PASS_1 = "HIDDEN_FOR_SECURITY_REASONS";

const char* WIFI_SSID_2 = "moaiwlan";
const char* WIFI_PASS_2 = "HIDDEN_FOR_SECURITY_REASONS";

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== Wi-Fi test ===");

  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);

  Serial.print("Connecting");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" connected!");
  Serial.print("SSID:        ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address:  ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("RSSI:        ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.println();
  Serial.print("Resolving rootnet.local... ");
  if (!MDNS.begin("smartpot")) {
    Serial.println("mDNS failed to start");
  } else {
    IPAddress serverIP = MDNS.queryHost("rootnet");
    if (serverIP == IPAddress(0, 0, 0, 0)) {
      Serial.println("not found");
    } else {
      Serial.print("found at ");
      Serial.println(serverIP);
    }
  }
}

void loop() {
}