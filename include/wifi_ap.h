#pragma once

#include <Arduino.h>

void wifiApInit();
void wifiApLoop();
void wifiApEnable();
void wifiApDisable();
bool wifiApIsActive();
const char* wifiApSsid();

bool wifiStaConnect();
void wifiStaDisconnect();
bool wifiStaIsConnected();
String wifiStaSsid();
String wifiStaIp();
String wifiStaStatusText();
int wifiStaRssi();