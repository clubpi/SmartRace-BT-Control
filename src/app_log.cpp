#include "app_log.h"

static const int LOG_CAPACITY = 20;
static String s_logs[LOG_CAPACITY];
static int s_start = 0;
static int s_count = 0;

void appLog(const String& message) {
    String line = String("[") + String(millis()) + String(" ms] ") + message;
    Serial.println(line);

    if (s_count < LOG_CAPACITY) {
        s_logs[(s_start + s_count) % LOG_CAPACITY] = line;
        s_count++;
    } else {
        s_logs[s_start] = line;
        s_start = (s_start + 1) % LOG_CAPACITY;
    }
}

int appLogCount() {
    return s_count;
}

String appLogGet(int index) {
    if (index < 0 || index >= s_count) {
        return String();
    }
    return s_logs[(s_start + index) % LOG_CAPACITY];
}
