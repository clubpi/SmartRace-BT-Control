#pragma once

#include <Arduino.h>

void wifiApInit();
void wifiApLoop();
void wifiApEnable();
void wifiApDisable();
void wifiApSetBatteryMode(bool enabled);
bool wifiApIsBatteryModeEnabled();
bool wifiApIsActive();
uint32_t wifiApRemainingMs();
const char* wifiApSsid();