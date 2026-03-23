#include <Arduino.h>
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
    delay(1000);
    Serial.println();
    Serial.println("SmartRace BT Control start");

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    configLoad();
    wifiApInit();
    bleInit();
    buttonsInit();
    webUiInit();
}

void loop() {
    wifiApLoop();
    webUiLoop();
    buttonsLoop();
    statusLedLoop();
    delay(5);
}