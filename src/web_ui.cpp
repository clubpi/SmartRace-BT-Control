#include "web_ui.h"
#include "app_config.h"
#include "ble_control.h"
#include "bond_manager.h"
#include "wifi_ap.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

static WebServer server(80);

static String htmlEscape(const String& s) {
    String out = s;
    out.replace("&", "&amp;");
    out.replace("<", "&lt;");
    out.replace(">", "&gt;");
    out.replace("\"", "&quot;");
    out.replace("'", "&#39;");
    return out;
}

static char safeCharFromArg(const String& arg, char fallback) {
    if (arg.length() < 1) return fallback;
    return arg.charAt(0);
}

static uint32_t safeUIntFromArg(const String& arg, uint32_t fallback, uint32_t minVal, uint32_t maxVal) {
    if (arg.length() == 0) return fallback;
    long v = arg.toInt();
    if (v < (long)minVal) v = minVal;
    if (v > (long)maxVal) v = maxVal;
    return (uint32_t)v;
}

static String connectionStatusText() {
    return bleIsConnected() ? "Verbunden" : "Nicht verbunden";
}

static String connectionStatusClass() {
    return bleIsConnected() ? "ok" : "warn";
}

static String batteryModeText() {
    return wifiApIsBatteryModeEnabled() ? "Aktiv" : "Aus";
}

static String apModeText() {
    if (!wifiApIsActive()) {
        return "Aus";
    }
    if (wifiApIsBatteryModeEnabled()) {
        uint32_t remainingSeconds = wifiApRemainingMs() / 1000;
        return String("An (noch ") + String(remainingSeconds) + String("s)");
    }
    return "Dauerhaft an";
}

static String buildPage() {
    String html;
    html += "<!doctype html><html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1,viewport-fit=cover'>";
    html += "<title>SmartRace BT Control</title>";
    html += "<style>";
    html += ":root{--bg:#0b1220;--card:#121b2d;--card2:#182338;--line:#2a3957;--text:#eef4ff;--muted:#a8b5ce;--accent:#5b7cff;--accent2:#7a95ff;--ok:#19c37d;--warn:#f5a524;--danger:#ef4444;}";
    html += "*{box-sizing:border-box}html,body{margin:0;padding:0;background:linear-gradient(180deg,#0a1020 0%,#0f1728 100%);color:var(--text);font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Arial,sans-serif}";
    html += ".wrap{max-width:980px;margin:0 auto;padding:18px}";
    html += ".hero{background:linear-gradient(135deg,#18284c 0%,#101b33 100%);border:1px solid var(--line);border-radius:18px;padding:20px;box-shadow:0 8px 30px rgba(0,0,0,.25)}";
    html += ".title{font-size:30px;font-weight:800;letter-spacing:.2px;margin:0 0 6px}";
    html += ".subtitle{margin:0;color:var(--muted);font-size:15px}";
    html += ".status-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:12px;margin-top:18px}";
    html += ".stat{background:rgba(255,255,255,.03);border:1px solid var(--line);border-radius:14px;padding:14px}";
    html += ".stat small{display:block;color:var(--muted);margin-bottom:6px;font-size:12px;text-transform:uppercase;letter-spacing:.08em}";
    html += ".stat strong{font-size:18px}";
    html += ".badge{display:inline-flex;align-items:center;gap:8px;padding:8px 12px;border-radius:999px;font-weight:700;font-size:14px}";
    html += ".badge.ok{background:rgba(25,195,125,.15);color:#8ff0c0;border:1px solid rgba(25,195,125,.35)}";
    html += ".badge.warn{background:rgba(245,165,36,.14);color:#ffd089;border:1px solid rgba(245,165,36,.35)}";
    html += ".section{margin-top:18px;background:var(--card);border:1px solid var(--line);border-radius:18px;padding:18px;box-shadow:0 8px 30px rgba(0,0,0,.18)}";
    html += ".section h2{margin:0 0 14px;font-size:20px}";
    html += ".grid-2{display:grid;grid-template-columns:1fr 1fr;gap:14px}";
    html += ".grid-6{display:grid;grid-template-columns:repeat(6,1fr);gap:10px}";
    html += ".field{background:var(--card2);border:1px solid var(--line);border-radius:14px;padding:12px}";
    html += ".field label{display:block;margin-bottom:8px;color:var(--muted);font-size:13px;font-weight:700}";
    html += "input{width:100%;padding:13px 14px;border-radius:12px;border:1px solid #33476b;background:#0d1526;color:var(--text);font-size:16px;outline:none}";
    html += "input:focus{border-color:var(--accent);box-shadow:0 0 0 3px rgba(91,124,255,.15)}";
    html += "input[type='checkbox']{width:auto;padding:0;margin:0;accent-color:var(--accent)}";
    html += ".check-row{display:flex;align-items:center;gap:10px;background:var(--card2);border:1px solid var(--line);border-radius:14px;padding:12px}";
    html += ".check-row label{margin:0;color:var(--text);font-size:15px;font-weight:700}";
    html += ".btn-row{display:flex;flex-wrap:wrap;gap:10px;margin-top:14px}";
    html += ".btn{appearance:none;border:none;display:inline-flex;align-items:center;justify-content:center;gap:8px;padding:13px 16px;border-radius:12px;font-weight:800;font-size:15px;text-decoration:none;cursor:pointer;transition:.15s ease;min-height:48px}";
    html += ".btn-primary{background:linear-gradient(135deg,var(--accent) 0%,var(--accent2) 100%);color:white}";
    html += ".btn-secondary{background:#1a2741;color:var(--text);border:1px solid var(--line)}";
    html += ".btn-danger{background:#3a1820;color:#ffd8de;border:1px solid rgba(239,68,68,.35)}";
    html += ".btn-warning{background:#3e2b14;color:#ffe1ac;border:1px solid rgba(245,165,36,.28)}";
    html += ".inline-form{display:inline} ";
    html += ".hint{font-size:13px;color:var(--muted);margin-top:8px}";
    html += ".key-card{background:var(--card2);border:1px solid var(--line);border-radius:14px;padding:12px;text-align:center}";
    html += ".key-card .k{font-size:13px;color:var(--muted);margin-bottom:8px;font-weight:700}";
    html += ".key-card input{text-align:center;font-size:22px;font-weight:800;padding:16px 8px}";
    html += ".footer-note{margin-top:16px;color:var(--muted);font-size:13px;line-height:1.5}";
    html += "@media(max-width:900px){.grid-6{grid-template-columns:repeat(3,1fr)}.status-grid{grid-template-columns:1fr}.grid-2{grid-template-columns:1fr}}";
    html += "@media(max-width:600px){.wrap{padding:12px}.title{font-size:26px}.grid-6{grid-template-columns:repeat(2,1fr)}}";
    html += "</style></head><body>";

    html += "<div class='wrap'>";
    html += "<div class='hero'>";
    html += "<h1 class='title'>SmartRace BT Control</h1>";
    html += "<p class='subtitle'>Bluetooth-Tastatur mit 6 Eingängen, Web-Setup und Bond-Reset</p>";
    html += "<div class='status-grid'>";
    html += "<div class='stat'><small>Status</small><strong><span class='badge ";
    html += connectionStatusClass();
    html += "'>";
    html += connectionStatusText();
    html += "</span></strong></div>";
    html += "<div class='stat'><small>Access Point</small><strong>";
    html += wifiApSsid();
    html += "</strong></div>";
    html += "<div class='stat'><small>Webadresse</small><strong>";
    html += WiFi.softAPIP().toString();
    html += "</strong></div>";
    html += "<div class='stat'><small>Batteriemodus</small><strong>";
    html += batteryModeText();
    html += "</strong><div class='hint' style='margin-top:6px'>AP: ";
    html += apModeText();
    html += "</div></div>";
    html += "</div></div>";

    html += "<form method='POST' action='/save'>";
    html += "<div class='section'><h2>Allgemein</h2>";
    html += "<div class='grid-2'>";
    html += "<div class='field'><label>Bluetooth Name</label><input name='bt_name' maxlength='31' value='";
    html += htmlEscape(g_config.btName);
    html += "'></div>";
    html += "<div class='field'><label>Langdruck Zahl</label><input name='long_key' maxlength='1' value='";
    html += String(g_config.longPressKey);
    html += "'></div>";
    html += "</div>";

    html += "<div class='grid-2' style='margin-top:14px'>";
    html += "<div class='field'><label>Kurzdruck Präfix</label><input name='short_prefix' maxlength='1' value='";
    html += String(g_config.shortPrefixKey);
    html += "'></div>";
    html += "<div class='field'><label>Verzögerung nach Präfix (ms)</label><input name='short_delay' type='number' min='0' max='10000' value='";
    html += String(g_config.shortDelayMs);
    html += "'></div>";
    html += "</div>";

    html += "<div class='field' style='margin-top:14px'><label>Langdruck Zeit (ms)</label><input name='long_time' type='number' min='200' max='10000' value='";
    html += String(g_config.longPressMs);
    html += "'><div class='hint'>Beispiel: 500 = sehr kurz, 1200 = angenehm, 2000 = deutlich lang</div></div>";

    html += "<div class='grid-2' style='margin-top:14px'>";
    html += "<div class='check-row'><input type='checkbox' id='send_prefix' name='send_prefix' value='1'";
    if (g_config.sendPrefixOnShortPress) html += " checked";
    html += "><label for='send_prefix'>Praefix bei Kurzdruck senden</label></div>";
    html += "<div class='check-row'><input type='checkbox' id='enable_long_press' name='enable_long_press' value='1'";
    if (g_config.enableLongPress) html += " checked";
    html += "><label for='enable_long_press'>Langdruck aktivieren</label></div>";
    html += "</div>";
    html += "</div>";

    html += "<div class='section'><h2>Tasten 1 bis 6</h2><div class='grid-6'>";
    for (int i = 0; i < 6; i++) {
        html += "<div class='key-card'><div class='k'>Taste ";
        html += String(i + 1);
        html += "</div><input name='b";
        html += String(i + 1);
        html += "' maxlength='1' value='";
        html += String(g_config.buttonKeys[i]);
        html += "'></div>";
    }
    html += "</div><div class='footer-note'>Kurzdruck sendet zuerst das Präfix und danach nach der eingestellten Wartezeit die jeweilige Taste. Langdruck sendet immer die globale Langdruck-Zahl.</div></div>";

    html += "<div class='section'><h2>Speichern</h2><div class='btn-row'>";
    html += "<button class='btn btn-primary' type='submit'>Speichern und Neustarten</button>";
    html += "<a class='btn btn-secondary' href='/'>Neu laden</a>";
    html += "</div></div></form>";

    html += "<div class='section'><h2>Service</h2><div class='btn-row'>";
    html += "<form class='inline-form' method='POST' action='/delete-bonds' onsubmit=\"return confirm('Bondings wirklich löschen und neu starten?');\">";
    html += "<button class='btn btn-warning' type='submit'>Bondings löschen</button></form>";
    html += "<form class='inline-form' method='POST' action='/reboot' onsubmit=\"return confirm('ESP32 wirklich neu starten?');\">";
    html += "<button class='btn btn-danger' type='submit'>Neustarten</button></form>";
    html += "<form class='inline-form' method='POST' action='/battery-mode-toggle'>";
    if (wifiApIsBatteryModeEnabled()) {
        html += "<button class='btn btn-secondary' type='submit'>Batteriemodus deaktivieren</button>";
    } else {
        html += "<button class='btn btn-primary' type='submit'>Batteriemodus aktivieren</button>";
    }
    html += "</form>";
    html += "</div><div class='footer-note'>Wenn das iPhone oder iPad noch komisch reagiert, dort zusätzlich unter Bluetooth das Gerät ignorieren und dann neu koppeln.</div></div>";

    html += "</div></body></html>";
    return html;
}

static void handleRoot() {
    server.send(200, "text/html; charset=utf-8", buildPage());
}

static void handleSave() {
    g_config.btName = server.arg("bt_name");
    if (g_config.btName.length() < 1) g_config.btName = "SmartRace BT Control";

    g_config.shortPrefixKey = safeCharFromArg(server.arg("short_prefix"), '7');
    g_config.shortDelayMs = safeUIntFromArg(server.arg("short_delay"), 2000, 0, 10000);
    g_config.sendPrefixOnShortPress = server.hasArg("send_prefix");
    g_config.longPressKey = safeCharFromArg(server.arg("long_key"), '8');
    g_config.longPressMs = safeUIntFromArg(server.arg("long_time"), 1200, 200, 10000);
    g_config.enableLongPress = server.hasArg("enable_long_press");

    g_config.buttonKeys[0] = safeCharFromArg(server.arg("b1"), '1');
    g_config.buttonKeys[1] = safeCharFromArg(server.arg("b2"), '2');
    g_config.buttonKeys[2] = safeCharFromArg(server.arg("b3"), '3');
    g_config.buttonKeys[3] = safeCharFromArg(server.arg("b4"), '4');
    g_config.buttonKeys[4] = safeCharFromArg(server.arg("b5"), '5');
    g_config.buttonKeys[5] = safeCharFromArg(server.arg("b6"), '6');

    configSave();

    server.send(200, "text/html; charset=utf-8",
                "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<style>body{font-family:Arial;background:#0b1220;color:#eef4ff;padding:32px}</style></head><body>"
                "<h2>Gespeichert</h2><p>Der ESP32 startet jetzt neu.</p></body></html>");
    delay(500);
    ESP.restart();
}

static void handleDeleteBonds() {
    server.send(200, "text/html; charset=utf-8",
                "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<style>body{font-family:Arial;background:#0b1220;color:#eef4ff;padding:32px}</style></head><body>"
                "<h2>Bondings löschen</h2><p>Der ESP32 startet jetzt neu.</p></body></html>");
    delay(400);
    deleteBondsNow(true);
}

static void handleReboot() {
    server.send(200, "text/html; charset=utf-8",
                "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<style>body{font-family:Arial;background:#0b1220;color:#eef4ff;padding:32px}</style></head><body>"
                "<h2>Neustart</h2><p>Der ESP32 startet jetzt neu.</p></body></html>");
    delay(400);
    ESP.restart();
}

static void handleBatteryModeToggle() {
    bool enableBatteryMode = !wifiApIsBatteryModeEnabled();
    wifiApSetBatteryMode(enableBatteryMode);
    g_config.batteryModeEnabled = enableBatteryMode;
    configSave();

    server.send(200, "text/html; charset=utf-8",
                "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<style>body{font-family:Arial;background:#0b1220;color:#eef4ff;padding:32px}</style></head><body>"
                "<h2>Batteriemodus aktualisiert</h2><p>Einstellung wurde gespeichert.</p><p><a href='/' style='color:#8fb4ff'>Zurueck</a></p></body></html>");
}

void webUiInit() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/delete-bonds", HTTP_POST, handleDeleteBonds);
    server.on("/reboot", HTTP_POST, handleReboot);
    server.on("/battery-mode-toggle", HTTP_POST, handleBatteryModeToggle);
    server.begin();

    Serial.println("Web UI started");
    Serial.println("Open browser: http://192.168.4.1");
}

void webUiLoop() {
    server.handleClient();
}