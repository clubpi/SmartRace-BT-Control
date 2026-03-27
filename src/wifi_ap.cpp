#include "wifi_ap.h"
#include "app_config.h"
#include "app_log.h"
#include <Arduino.h>
#include <WiFi.h>

static const char* AP_SSID = "SmartRace-Setup";
static const char* AP_PASSWORD = "12345678";
static const uint32_t AP_BATTERY_TIMEOUT_MS = 60000;

static bool apActive = false;
static bool batteryModeEnabled = false;
static unsigned long apStartMs = 0;

static void updateWifiMode() {
    wl_status_t staStatus = WiFi.status();
    bool staConnected = (staStatus == WL_CONNECTED);
    if (apActive) {
        WiFi.mode(staConnected ? WIFI_AP_STA : WIFI_AP);
    } else {
        WiFi.mode(staConnected ? WIFI_STA : WIFI_OFF);
    }
}

void wifiApEnable() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    apActive = true;
    apStartMs = millis();

    appLog(String("AP SSID: ") + AP_SSID);
    appLog("AP Security: WPA2-PSK");
    appLog(String("AP IP: ") + WiFi.softAPIP().toString());
}

void wifiApDisable() {
    if (!apActive) {
        return;
    }

    WiFi.softAPdisconnect(true);
    apActive = false;
    updateWifiMode();
    appLog("AP disabled");
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
    if (g_config.staAutoConnect && g_config.staSsid.length() > 0) {
        appLog("STA auto connect requested");
        wifiStaConnect();
    }
}

void wifiApLoop() {
    if (!apActive || !batteryModeEnabled) {
        return;
    }

    if (millis() - apStartMs >= AP_BATTERY_TIMEOUT_MS) {
        appLog("Battery mode timeout reached - disabling AP");
        wifiApDisable();
    }
}

const char* wifiApSsid() {
    return AP_SSID;
}

bool wifiStaConnect() {
    if (g_config.staSsid.length() == 0) {
        appLog("STA connect skipped: SSID empty");
        return false;
    }

    WiFi.mode(apActive ? WIFI_AP_STA : WIFI_STA);
    WiFi.begin(g_config.staSsid.c_str(), g_config.staPassword.c_str());
    appLog(String("STA connect start: ") + g_config.staSsid);

    unsigned long startMs = millis();
    while (millis() - startMs < 12000) {
        if (WiFi.status() == WL_CONNECTED) {
            appLog(String("STA connected: ") + WiFi.localIP().toString());
            return true;
        }
        delay(100);
    }

    appLog(String("STA connect failed, status=") + String((int)WiFi.status()));
    updateWifiMode();
    return false;
}

void wifiStaDisconnect() {
    if (WiFi.status() == WL_CONNECTED || WiFi.SSID().length() > 0) {
        WiFi.disconnect(false, false);
        appLog("STA disconnected");
    }
    updateWifiMode();
}

bool wifiStaIsConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifiStaSsid() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.SSID();
    }
    return g_config.staSsid;
}

String wifiStaIp() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "-";
}

String wifiStaStatusText() {
    wl_status_t status = WiFi.status();
    switch (status) {
        case WL_CONNECTED: return "Verbunden";
        case WL_NO_SSID_AVAIL: return "SSID nicht gefunden";
        case WL_CONNECT_FAILED: return "Verbindung fehlgeschlagen";
        case WL_CONNECTION_LOST: return "Verbindung verloren";
        case WL_DISCONNECTED: return "Getrennt";
        case WL_IDLE_STATUS: return "Idle";
        default: return String("Status ") + String((int)status);
    }
}

int wifiStaRssi() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}