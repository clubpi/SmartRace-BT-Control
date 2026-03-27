#pragma once
#include <Arduino.h>

struct AppConfig {
    String btName;
    bool batteryModeEnabled;
    String staSsid;
    String staPassword;
    bool staAutoConnect;
    char shortPrefixKey;
    uint32_t shortDelayMs;
    bool sendPrefixOnShortPress;
    bool sendPrefixOnlyOnceUntilLongPress;
    char longPressKey;
    uint32_t longPressMs;
    char buttonKeys[6];
};

extern AppConfig g_config;

void configLoad();
void configSave();