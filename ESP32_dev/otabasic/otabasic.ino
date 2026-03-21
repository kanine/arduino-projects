#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

void setup() {
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
}