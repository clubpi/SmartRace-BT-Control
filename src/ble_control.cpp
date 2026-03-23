#include "ble_control.h"
#include "app_config.h"
#include <Arduino.h>
#include <BleKeyboard.h>

static BleKeyboard* bleKeyboard = nullptr;

void bleInit() {
    if (bleKeyboard != nullptr) {
        delete bleKeyboard;
        bleKeyboard = nullptr;
    }

    bleKeyboard = new BleKeyboard(g_config.btName.c_str(), "SmartRace", 100);
    bleKeyboard->begin();

    Serial.print("BLE name: ");
    Serial.println(g_config.btName);
    Serial.println("BLE Keyboard started");
}

bool bleIsConnected() {
    return (bleKeyboard != nullptr && bleKeyboard->isConnected());
}

void bleSendKey(char key) {
    if (bleKeyboard != nullptr && bleKeyboard->isConnected()) {
        bleKeyboard->write(key);
    }
}

const char* bleGetDeviceName() {
    return g_config.btName.c_str();
}