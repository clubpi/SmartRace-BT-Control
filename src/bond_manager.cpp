#include "bond_manager.h"
#include <Arduino.h>
#include <esp_gap_ble_api.h>

static const uint8_t ledPin = 2;

void deleteBondsNow(bool restartAfter) {
    Serial.println("Delete bonds requested");

    int bondedDeviceCount = esp_ble_get_bond_device_num();
    Serial.print("Bonded devices found: ");
    Serial.println(bondedDeviceCount);

    if (bondedDeviceCount > 0) {
        esp_ble_bond_dev_t* bondedDevices =
            (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * bondedDeviceCount);

        if (bondedDevices != nullptr) {
            esp_ble_get_bond_device_list(&bondedDeviceCount, bondedDevices);

            for (int i = 0; i < bondedDeviceCount; i++) {
                esp_err_t result = esp_ble_remove_bond_device(bondedDevices[i].bd_addr);

                Serial.print("Removing bond ");
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.println(result == ESP_OK ? "OK" : "FAILED");
            }

            free(bondedDevices);
        } else {
            Serial.println("Memory allocation failed");
        }
    } else {
        Serial.println("No bonded devices stored");
    }

    for (int i = 0; i < 10; i++) {
        digitalWrite(ledPin, HIGH);
        delay(80);
        digitalWrite(ledPin, LOW);
        delay(80);
    }

    if (restartAfter) {
        Serial.println("Restart ESP...");
        delay(300);
        ESP.restart();
    }
}