#include "button_handler.h"
#include "app_config.h"
#include "ble_control.h"
#include <Arduino.h>

static const uint8_t buttonPins[6] = {14, 27, 26, 25, 33, 32};
static const unsigned long debounceDelay = 30;

static bool lastReading[6];
static bool buttonState[6];
static unsigned long lastDebounceTime[6];

static bool pressedFlag[6];
static unsigned long pressTime[6];
static bool longPressHandled[6];
static bool prefixReady = true;

void buttonsInit() {
    for (int i = 0; i < 6; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        lastReading[i] = digitalRead(buttonPins[i]);
        buttonState[i] = lastReading[i];
        lastDebounceTime[i] = 0;
        pressedFlag[i] = false;
        pressTime[i] = 0;
        longPressHandled[i] = false;
    }
}

void buttonsLoop() {
    if (!bleIsConnected()) {
        return;
    }

    for (int i = 0; i < 6; i++) {
        bool reading = digitalRead(buttonPins[i]);

        if (reading != lastReading[i]) {
            lastDebounceTime[i] = millis();
        }

        if ((millis() - lastDebounceTime[i]) > debounceDelay) {
            if (reading != buttonState[i]) {
                buttonState[i] = reading;

                if (buttonState[i] == LOW) {
                    pressedFlag[i] = true;
                    pressTime[i] = millis();
                    longPressHandled[i] = false;
                } else {
                    if (pressedFlag[i]) {
                        unsigned long duration = millis() - pressTime[i];
                        bool shortPressTriggered = !longPressHandled[i] && duration < g_config.longPressMs;

                        if (shortPressTriggered) {
                            Serial.print("Short press button ");
                            Serial.println(i + 1);

                            if (g_config.sendPrefixOnShortPress) {
                                bool shouldSendPrefix = true;
                                if (g_config.sendPrefixOnlyOnceUntilLongPress) {
                                    shouldSendPrefix = prefixReady;
                                }

                                if (shouldSendPrefix) {
                                    bleSendKey(g_config.shortPrefixKey);
                                    delay(g_config.shortDelayMs);
                                    if (g_config.sendPrefixOnlyOnceUntilLongPress) {
                                        prefixReady = false;
                                    }
                                }
                            }
                            bleSendKey(g_config.buttonKeys[i]);
                        }

                        pressedFlag[i] = false;
                        longPressHandled[i] = false;
                    }
                }
            }
        }

        if (pressedFlag[i] && !longPressHandled[i]) {
            if (millis() - pressTime[i] >= g_config.longPressMs) {
                Serial.print("Long press button ");
                Serial.println(i + 1);

                bleSendKey(g_config.longPressKey);
                prefixReady = true;
                longPressHandled[i] = true;
            }
        }

        lastReading[i] = reading;
    }
}