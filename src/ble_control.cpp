#include "ble_control.h"
#include "app_config.h"
#include "app_log.h"
#include <Arduino.h>
#include <BleKeyboard.h>
#include <BLESecurity.h>

static BleKeyboard* bleKeyboard = nullptr;
static const uint16_t keyPressHoldMs = 18;
static const uint16_t keyReleaseGapMs = 12;

// BleKeyboard treats values >= 136 as HID non-printing keys and subtracts 136 internally.
static const uint8_t hidRawOffset = 136;

static uint8_t mapDigitToHidKeycode(char key) {
    switch (key) {
        case '1': return hidRawOffset + 0x1E;
        case '2': return hidRawOffset + 0x1F;
        case '3': return hidRawOffset + 0x20;
        case '4': return hidRawOffset + 0x21;
        case '5': return hidRawOffset + 0x22;
        case '6': return hidRawOffset + 0x23;
        case '7': return hidRawOffset + 0x24;
        case '8': return hidRawOffset + 0x25;
        case '9': return hidRawOffset + 0x26;
        case '0': return hidRawOffset + 0x27;
        default: return (uint8_t)key;
    }
}

static void configureBleSecurityForCompatibility() {
    // Keep secure connections and bonding, but drop MITM requirement for phone compatibility.
    BLESecurity security;
    security.setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    security.setCapability(ESP_IO_CAP_NONE);
    security.setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
    security.setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
}

void bleInit() {
    if (bleKeyboard != nullptr) {
        delete bleKeyboard;
        bleKeyboard = nullptr;
    }

    configureBleSecurityForCompatibility();
    bleKeyboard = new BleKeyboard(g_config.btName.c_str(), "SmartRace", 100);
    bleKeyboard->begin();
    configureBleSecurityForCompatibility();

    appLog(String("BLE name: ") + g_config.btName);
    appLog("BLE Keyboard started (SC_BOND, no MITM)");
}

bool bleIsConnected() {
    return (bleKeyboard != nullptr && bleKeyboard->isConnected());
}

void bleSendKey(char key) {
    if (bleKeyboard != nullptr && bleKeyboard->isConnected()) {
        uint8_t sendKey = mapDigitToHidKeycode(key);

        // Explicit press/release improves compatibility with some HID hosts on ESP32-S3.
        bool pressed = bleKeyboard->press(sendKey);
        if (!pressed) {
            appLog(String("[BLE] press failed for key: ") + String(key));
            return;
        }

        delay(keyPressHoldMs);
        bleKeyboard->releaseAll();
        delay(keyReleaseGapMs);
    }
}

const char* bleGetDeviceName() {
    return g_config.btName.c_str();
}