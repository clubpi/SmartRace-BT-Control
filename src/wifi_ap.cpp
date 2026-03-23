#include "wifi_ap.h"
#include <Arduino.h>
#include <WiFi.h>

static const char* AP_SSID = "SmartRace-Setup";
static const char* AP_PASSWORD = "12345678";

void wifiApInit() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    Serial.print("AP SSID: ");
    Serial.println(AP_SSID);
    Serial.print("AP Security: ");
    Serial.println("WPA2-PSK");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

const char* wifiApSsid() {
    return AP_SSID;
}