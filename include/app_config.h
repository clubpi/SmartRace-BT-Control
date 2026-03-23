#pragma once
#include <Arduino.h>

struct AppConfig {
    String btName;
    char shortPrefixKey;
    uint32_t shortDelayMs;
    bool sendPrefixOnShortPress;
    char longPressKey;
    uint32_t longPressMs;
    bool enableLongPress;
    char buttonKeys[6];
};

extern AppConfig g_config;

void configLoad();
void configSave();