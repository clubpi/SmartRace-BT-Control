#pragma once

void bleInit();
bool bleIsConnected();
void bleSendKey(char key);
const char* bleGetDeviceName();