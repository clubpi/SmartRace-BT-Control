#include <Arduino.h>
#include <esp_system.h>
#include <esp32-hal-rgb-led.h>
#include "app_log.h"
#include "app_config.h"
#include "ble_control.h"
#include "bond_manager.h"
#include "button_handler.h"
#include "web_ui.h"
#include "wifi_ap.h"

static const uint8_t ledPin = 2;
#if CONFIG_IDF_TARGET_ESP32S3
static const uint8_t recoveryButtonPin = 4;  // Taste 1
// Some DevKitC-1 revisions expose the RGB LED on GPIO38, while the Arduino
// variant still advertises the built-in NeoPixel path via RGB_BUILTIN/PIN_NEOPIXEL.
static const uint8_t rgbLedPinFallback = 38;
#else
static const uint8_t recoveryButtonPin = 14; // Taste 1
#endif
static unsigned long lastBlinkTime = 0;
static bool ledState = false;

#if CONFIG_IDF_TARGET_ESP32S3
static void setRgbStatusColor(uint8_t red, uint8_t green, uint8_t blue) {
    neopixelWrite(RGB_BUILTIN, red, green, blue);
    neopixelWrite(rgbLedPinFallback, red, green, blue);
}
#endif

static bool isBondRecoveryRequestedOnBoot() {
    pinMode(recoveryButtonPin, INPUT_PULLUP);
    unsigned long startMs = millis();
    while (millis() - startMs < 1200) {
        if (digitalRead(recoveryButtonPin) != LOW) {
            return false;
        }
        delay(10);
    }
    return true;
}

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

#if CONFIG_IDF_TARGET_ESP32S3
    if (bleIsConnected()) {
        setRgbStatusColor(0, 28, 0);
    } else {
        if (ledState) {
            setRgbStatusColor(0, 0, 20);
        } else {
            setRgbStatusColor(0, 0, 0);
        }
    }
#endif
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

#if CONFIG_IDF_TARGET_ESP32S3
    setRgbStatusColor(0, 0, 0);
#endif

    appLog("[INIT] configLoad");
    configLoad();
    delay(1);

    appLog("[INIT] wifiApInit");
    wifiApInit();
    delay(1);

    if (isBondRecoveryRequestedOnBoot()) {
        appLog("[INIT] recovery: button 1 held, clearing BLE bonds");
        deleteBondsNow(false);
        delay(150);
    }

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