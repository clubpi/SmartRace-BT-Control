#include "wifi_ap.h"
#include "app_config.h"
#include <Arduino.h>
#include <WiFi.h>

static const char* AP_SSID = "SmartRace-Setup";
static const char* AP_PASSWORD = "12345678";
static const uint32_t AP_BATTERY_TIMEOUT_MS = 60000;

static bool apActive = false;
static bool batteryModeEnabled = false;
static unsigned long apStartMs = 0;

void wifiApEnable() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    apActive = true;
    apStartMs = millis();

    Serial.print("AP SSID: ");
    Serial.println(AP_SSID);
    Serial.print("AP Security: ");
    Serial.println("WPA2-PSK");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void wifiApDisable() {
    if (!apActive) {
        return;
    }

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    apActive = false;
    Serial.println("AP disabled");
}

void wifiApSetBatteryMode(bool enabled) {
    batteryModeEnabled = enabled;
    g_config.batteryModeEnabled = enabled;

    if (!apActive) {
        return;
    }

    if (batteryModeEnabled) {
        apStartMs = millis();
    }
}

bool wifiApIsBatteryModeEnabled() {
    return batteryModeEnabled;
}

bool wifiApIsActive() {
    return apActive;
}

uint32_t wifiApRemainingMs() {
    if (!apActive || !batteryModeEnabled) {
        return 0;
    }

    unsigned long elapsed = millis() - apStartMs;
    if (elapsed >= AP_BATTERY_TIMEOUT_MS) {
        return 0;
    }
    return AP_BATTERY_TIMEOUT_MS - elapsed;
}

void wifiApInit() {
    batteryModeEnabled = g_config.batteryModeEnabled;
    wifiApEnable();
}

void wifiApLoop() {
    if (!apActive || !batteryModeEnabled) {
        return;
    }

    if (millis() - apStartMs >= AP_BATTERY_TIMEOUT_MS) {
        Serial.println("Battery mode timeout reached - disabling AP");
        wifiApDisable();
    }
}

const char* wifiApSsid() {
    return AP_SSID;
}