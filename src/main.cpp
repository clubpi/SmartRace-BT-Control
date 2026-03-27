#include <Arduino.h>
#include <esp_system.h>
#include "app_log.h"
#include "app_config.h"
#include "ble_control.h"
#include "bond_manager.h"
#include "button_handler.h"
#include "web_ui.h"
#include "wifi_ap.h"

static const uint8_t ledPin = 2;
static unsigned long lastBlinkTime = 0;
static bool ledState = false;

static void statusLedLoop() {
    if (bleIsConnected()) {
        digitalWrite(ledPin, HIGH);
    } else {
        if (millis() - lastBlinkTime >= 500) {
            lastBlinkTime = millis();
            ledState = !ledState;
            digitalWrite(ledPin, ledState);
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println();
    appLog("SmartRace BT Control start");

    esp_reset_reason_t reason = esp_reset_reason();
    appLog(String("[INIT] reset reason: ") + String((int)reason));

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    appLog("[INIT] configLoad");
    configLoad();
    delay(1);

    appLog("[INIT] wifiApInit");
    wifiApInit();
    delay(1);

    bool skipBleInit = (reason == ESP_RST_WDT || reason == ESP_RST_TASK_WDT || reason == ESP_RST_INT_WDT);
    if (skipBleInit) {
        appLog("[INIT] BLE skipped after WDT reset (safe mode)");
    } else {
        appLog("[INIT] bleInit");
        bleInit();
    }
    delay(1);

    appLog("[INIT] buttonsInit");
    buttonsInit();

    appLog("[INIT] webUiInit");
    webUiInit();

    appLog("[INIT] setup done");
}

void loop() {
    wifiApLoop();
    webUiLoop();
    buttonsLoop();
    statusLedLoop();
    delay(5);
}