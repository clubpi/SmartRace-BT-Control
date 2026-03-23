#include "app_config.h"
#include <Preferences.h>

static Preferences prefs;
AppConfig g_config;

void configLoad() {
    prefs.begin("smartrace", true);

    g_config.btName = prefs.getString("bt_name", "SmartRace BT Control");
    g_config.shortPrefixKey = (char)prefs.getUChar("sprefix", '7');
    g_config.shortDelayMs = prefs.getUInt("sdelay", 2000);
    g_config.longPressKey = (char)prefs.getUChar("lkey", '8');
    g_config.longPressMs = prefs.getUInt("ltime", 1200);

    g_config.buttonKeys[0] = (char)prefs.getUChar("b1", '1');
    g_config.buttonKeys[1] = (char)prefs.getUChar("b2", '2');
    g_config.buttonKeys[2] = (char)prefs.getUChar("b3", '3');
    g_config.buttonKeys[3] = (char)prefs.getUChar("b4", '4');
    g_config.buttonKeys[4] = (char)prefs.getUChar("b5", '5');
    g_config.buttonKeys[5] = (char)prefs.getUChar("b6", '6');

    prefs.end();
}

void configSave() {
    prefs.begin("smartrace", false);

    prefs.putString("bt_name", g_config.btName);
    prefs.putUChar("sprefix", (uint8_t)g_config.shortPrefixKey);
    prefs.putUInt("sdelay", g_config.shortDelayMs);
    prefs.putUChar("lkey", (uint8_t)g_config.longPressKey);
    prefs.putUInt("ltime", g_config.longPressMs);

    prefs.putUChar("b1", (uint8_t)g_config.buttonKeys[0]);
    prefs.putUChar("b2", (uint8_t)g_config.buttonKeys[1]);
    prefs.putUChar("b3", (uint8_t)g_config.buttonKeys[2]);
    prefs.putUChar("b4", (uint8_t)g_config.buttonKeys[3]);
    prefs.putUChar("b5", (uint8_t)g_config.buttonKeys[4]);
    prefs.putUChar("b6", (uint8_t)g_config.buttonKeys[5]);

    prefs.end();
}