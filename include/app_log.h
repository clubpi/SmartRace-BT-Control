#pragma once

#include <Arduino.h>

void appLog(const String& message);
int appLogCount();
String appLogGet(int index);
