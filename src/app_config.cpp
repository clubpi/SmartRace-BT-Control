#include "app_config.h"
#include <Preferences.h>

static Preferences prefs;
AppConfig g_config;

void configLoad() {
    prefs.begin("smartrace", false);

    g_config.btName = "SmartRace BT Control";
    g_config.batteryModeEnabled = false;
    g_config.shortPrefixKey = '7';
    g_config.shortDelayMs = 2000;
    g_config.sendPrefixOnShortPress = true;
    g_config.sendPrefixOnlyOnceUntilLongPress = false;
    g_config.longPressKey = '8';
    g_config.longPressMs = 1200;
    g_config.buttonKeys[0] = '1';
    g_config.buttonKeys[1] = '2';
    g_config.buttonKeys[2] = '3';
    g_config.buttonKeys[3] = '4';
    g_config.buttonKeys[4] = '5';
    g_config.buttonKeys[5] = '6';

    if (prefs.isKey("bt_name")) g_config.btName = prefs.getString("bt_name");
    if (prefs.isKey("bat_on")) g_config.batteryModeEnabled = prefs.getBool("bat_on");
    if (prefs.isKey("sprefix")) g_config.shortPrefixKey = (char)prefs.getUChar("sprefix");
    if (prefs.isKey("sdelay")) g_config.shortDelayMs = prefs.getUInt("sdelay");
    if (prefs.isKey("sp_on")) g_config.sendPrefixOnShortPress = prefs.getBool("sp_on");
    if (prefs.isKey("sp_once")) g_config.sendPrefixOnlyOnceUntilLongPress = prefs.getBool("sp_once");
    if (prefs.isKey("lkey")) g_config.longPressKey = (char)prefs.getUChar("lkey");
    if (prefs.isKey("ltime")) g_config.longPressMs = prefs.getUInt("ltime");
    if (prefs.isKey("b1")) g_config.buttonKeys[0] = (char)prefs.getUChar("b1");
    if (prefs.isKey("b2")) g_config.buttonKeys[1] = (char)prefs.getUChar("b2");
    if (prefs.isKey("b3")) g_config.buttonKeys[2] = (char)prefs.getUChar("b3");
    if (prefs.isKey("b4")) g_config.buttonKeys[3] = (char)prefs.getUChar("b4");
    if (prefs.isKey("b5")) g_config.buttonKeys[4] = (char)prefs.getUChar("b5");
    if (prefs.isKey("b6")) g_config.buttonKeys[5] = (char)prefs.getUChar("b6");

    prefs.end();
}

void configSave() {
    prefs.begin("smartrace", false);

    prefs.putString("bt_name", g_config.btName);
    prefs.putBool("bat_on", g_config.batteryModeEnabled);
    prefs.putUChar("sprefix", (uint8_t)g_config.shortPrefixKey);
    prefs.putUInt("sdelay", g_config.shortDelayMs);
    prefs.putBool("sp_on", g_config.sendPrefixOnShortPress);
    prefs.putBool("sp_once", g_config.sendPrefixOnlyOnceUntilLongPress);
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