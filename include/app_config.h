#pragma once
#include <Arduino.h>

struct AppConfig {
    String btName;
    char shortPrefixKey;
    uint32_t shortDelayMs;
    char longPressKey;
    uint32_t longPressMs;
    char buttonKeys[6];
};

extern AppConfig g_config;

void configLoad();
void configSave();