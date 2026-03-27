#include "web_ui.h"
#include "app_config.h"
#include "app_log.h"
#include "ble_control.h"
#include "bond_manager.h"
#include "wifi_ap.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_system.h>

static WebServer server(80);
static String g_wifiScanHtml = "<div class='hint'>Noch kein WLAN-Scan gestartet.</div>";

enum UiPage {
    PAGE_OVERVIEW,
    PAGE_WIFI,
    PAGE_SYSTEM
};

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

static String bleText() {
    return bleIsConnected() ? "Verbunden" : "Nicht verbunden";
}

static String staText() {
    return wifiStaStatusText();
}

static String apText() {
    if (!wifiApIsActive()) return "Aus";
    if (!wifiApIsBatteryModeEnabled()) return "Dauerhaft an";
    return String("An (") + String(wifiApRemainingMs() / 1000) + String("s)");
}

static String batteryText() {
    return wifiApIsBatteryModeEnabled() ? "Aktiv" : "Aus";
}

static String statusClass(bool ok) {
    return ok ? "ok" : "warn";
}

static String commonStyle() {
    String html;
    html += "<style>";
    html += ":root{--bg:#0c1324;--panel:#17263e;--panel2:#1f3150;--line:#2f476e;--text:#ecf3ff;--muted:#9eb2d2;--accent:#0ca678;--accent2:#14b886;--ok:#22c55e;--warn:#f59e0b;--danger:#ef4444;}";
    html += "*{box-sizing:border-box}html,body{margin:0;padding:0;background:var(--bg);color:var(--text);font-family:'Segoe UI',Tahoma,Arial,sans-serif}";
    html += ".app{min-height:100vh;display:flex}";
    html += ".sidebar{width:250px;background:linear-gradient(180deg,#13233a 0%,#101d30 100%);border-right:1px solid var(--line);padding:18px;display:flex;flex-direction:column;gap:14px}";
    html += ".brand{background:linear-gradient(135deg,var(--accent) 0%,var(--accent2) 100%);border-radius:12px;padding:14px 12px;font-size:27px;font-weight:800;letter-spacing:.3px}";
    html += ".group-title{font-size:11px;letter-spacing:.09em;text-transform:uppercase;color:var(--muted);margin-top:8px}";
    html += ".nav-link{display:block;padding:11px 12px;border-radius:10px;text-decoration:none;color:var(--text);font-weight:700;border:1px solid transparent}";
    html += ".nav-link:hover{background:#1a2d49;border-color:var(--line)}";
    html += ".nav-link.active{background:linear-gradient(135deg,var(--accent) 0%,var(--accent2) 100%);color:#062419}";
    html += ".device{margin-top:auto;font-size:13px;color:var(--muted);line-height:1.5}";
    html += ".content{flex:1;padding:22px}";
    html += ".headline{font-size:40px;margin:0 0 6px;font-weight:800;letter-spacing:.2px}";
    html += ".sub{margin:0 0 14px;color:var(--muted)}";
    html += ".status-row{display:grid;grid-template-columns:repeat(5,1fr);gap:10px;margin-bottom:14px}";
    html += ".chip{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:11px}";
    html += ".chip .k{display:block;font-size:11px;color:var(--muted);text-transform:uppercase;letter-spacing:.08em;margin-bottom:4px}";
    html += ".chip .v{font-size:15px;font-weight:800}";
    html += ".badge{display:inline-flex;padding:6px 10px;border-radius:999px;font-weight:700;font-size:13px;border:1px solid transparent}";
    html += ".badge.ok{background:rgba(34,197,94,.18);border-color:rgba(34,197,94,.35);color:#9df4b8}";
    html += ".badge.warn{background:rgba(245,158,11,.15);border-color:rgba(245,158,11,.35);color:#ffd38d}";
    html += ".section{background:var(--panel);border:1px solid var(--line);border-radius:14px;padding:14px;margin-bottom:12px}";
    html += ".section h2{margin:0 0 10px;font-size:19px}";
    html += ".grid-2{display:grid;grid-template-columns:1fr 1fr;gap:10px}";
    html += ".grid-6{display:grid;grid-template-columns:repeat(6,1fr);gap:8px}";
    html += ".field{background:var(--panel2);border:1px solid var(--line);border-radius:10px;padding:10px}";
    html += ".field label{display:block;margin-bottom:6px;color:var(--muted);font-size:12px;font-weight:700}";
    html += "input{width:100%;padding:11px 12px;border-radius:10px;border:1px solid #3c5885;background:#0e1728;color:var(--text);font-size:15px;outline:none}";
    html += "input:focus{border-color:var(--accent2);box-shadow:0 0 0 3px rgba(20,184,134,.15)}";
    html += "input[type='checkbox']{width:auto;padding:0;margin:0;accent-color:var(--accent)}";
    html += ".check{display:flex;gap:10px;align-items:center;background:var(--panel2);border:1px solid var(--line);border-radius:10px;padding:10px}";
    html += ".check label{font-weight:700}";
    html += ".btn-row{display:flex;gap:8px;flex-wrap:wrap;margin-top:10px}";
    html += ".btn{border:none;border-radius:10px;padding:10px 14px;min-height:40px;font-weight:800;cursor:pointer;text-decoration:none;display:inline-flex;align-items:center;justify-content:center;font-size:13px}";
    html += ".btn-primary{background:linear-gradient(135deg,var(--accent) 0%,var(--accent2) 100%);color:#062419}";
    html += ".btn-secondary{background:#23395d;color:var(--text);border:1px solid var(--line)}";
    html += ".btn-warning{background:#4a3518;color:#ffdca3;border:1px solid rgba(245,158,11,.35)}";
    html += ".btn-danger{background:#491b22;color:#ffd7de;border:1px solid rgba(239,68,68,.35)}";
    html += ".hint{color:var(--muted);font-size:12px;margin-top:8px}";
    html += ".mono{font-family:Consolas,'Courier New',monospace;font-size:12px;line-height:1.45;background:#0f1a2d;border:1px solid var(--line);border-radius:10px;padding:10px;white-space:pre-wrap;max-height:300px;overflow:auto}";
    html += ".list{margin:0;padding-left:18px}";
    html += ".list li{margin:5px 0}";
    html += "@media(max-width:980px){.app{flex-direction:column}.sidebar{width:100%}.status-row{grid-template-columns:1fr 1fr}.grid-2{grid-template-columns:1fr}.grid-6{grid-template-columns:repeat(3,1fr)}}";
    html += "@media(max-width:620px){.status-row{grid-template-columns:1fr}.headline{font-size:30px}.grid-6{grid-template-columns:repeat(2,1fr)}}";
    html += "</style>";
    return html;
}

static String logsHtml() {
    String html = "<div class='mono'>";
    int n = appLogCount();
    if (n == 0) {
        html += "Noch keine Logs.";
    } else {
        for (int i = 0; i < n; i++) {
            html += htmlEscape(appLogGet(i));
            html += "\n";
        }
    }
    html += "</div>";
    return html;
}

static String pageShellStart(const String& title, UiPage active) {
    String html;
    html += "<!doctype html><html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1,viewport-fit=cover'>";
    html += "<title>" + title + "</title>";
    html += commonStyle();
    html += "</head><body><div class='app'>";

    html += "<aside class='sidebar'>";
    html += "<div class='brand'>SmartRace</div>";

    html += "<div class='group-title'>Dashboard</div>";
    html += "<a class='nav-link";
    if (active == PAGE_OVERVIEW) html += " active";
    html += "' href='/'>Overview</a>";

    html += "<div class='group-title'>System Settings</div>";
    html += "<a class='nav-link";
    if (active == PAGE_WIFI) html += " active";
    html += "' href='/wifi-setup'>WiFi Setup</a>";
    html += "<a class='nav-link";
    if (active == PAGE_SYSTEM) html += " active";
    html += "' href='/system-info'>System Info</a>";

    html += "<div class='device'>";
    html += "Software: SmartRace BT Control<br>";
    html += "Hardware: ESP32-S3 DevKitC-1";
    html += "</div>";
    html += "</aside>";

    html += "<main class='content'>";
    html += "<h1 class='headline'>" + title + "</h1>";
    html += "<p class='sub'>AP + STA parallel aktiv, Web-Manager und Diagnosen</p>";

    html += "<div class='status-row'>";
    html += "<div class='chip'><span class='k'>BLE</span><span class='v'><span class='badge " + statusClass(bleIsConnected()) + "'>" + bleText() + "</span></span></div>";
    html += "<div class='chip'><span class='k'>Access Point</span><span class='v'>" + apText() + "</span></div>";
    html += "<div class='chip'><span class='k'>AP IP</span><span class='v'>" + WiFi.softAPIP().toString() + "</span></div>";
    html += "<div class='chip'><span class='k'>STA</span><span class='v'><span class='badge " + statusClass(wifiStaIsConnected()) + "'>" + staText() + "</span></span></div>";
    html += "<div class='chip'><span class='k'>Batteriemodus</span><span class='v'>" + batteryText() + "</span></div>";
    html += "</div>";

    return html;
}

static String pageShellEnd() {
    return "</main></div></body></html>";
}

static String buildOverviewPage() {
    String html = pageShellStart("System Overview", PAGE_OVERVIEW);

    html += "<form method='POST' action='/save'>";
    html += "<section class='section'><h2>General Configuration</h2>";
    html += "<div class='grid-2'>";
    html += "<div class='field'><label>Bluetooth Name</label><input name='bt_name' maxlength='31' value='" + htmlEscape(g_config.btName) + "'></div>";
    html += "<div class='field'><label>Long Press Key</label><input name='long_key' maxlength='1' value='" + String(g_config.longPressKey) + "'></div>";
    html += "</div>";

    html += "<div class='grid-2' style='margin-top:10px'>";
    html += "<div class='field'><label>Short Prefix</label><input name='short_prefix' maxlength='1' value='" + String(g_config.shortPrefixKey) + "'></div>";
    html += "<div class='field'><label>Delay after Prefix (ms)</label><input name='short_delay' type='number' min='0' max='10000' value='" + String(g_config.shortDelayMs) + "'></div>";
    html += "</div>";

    html += "<div class='field' style='margin-top:10px'><label>Long Press Time (ms)</label><input name='long_time' type='number' min='200' max='10000' value='" + String(g_config.longPressMs) + "'></div>";

    html += "<div class='check' style='margin-top:10px'><input type='checkbox' id='send_prefix' name='send_prefix' value='1'";
    if (g_config.sendPrefixOnShortPress) html += " checked";
    html += "><label for='send_prefix'>Praefix bei Kurzdruck senden</label></div>";

    html += "<div class='check' style='margin-top:10px'><input type='checkbox' id='prefix_once' name='prefix_once' value='1'";
    if (g_config.sendPrefixOnlyOnceUntilLongPress) html += " checked";
    html += "><label for='prefix_once'>Praefix nur einmal senden bis Langdruck</label></div>";

    html += "<div class='btn-row'>";
    if (wifiApIsBatteryModeEnabled()) {
        html += "<button class='btn btn-secondary' type='submit' formmethod='POST' formaction='/battery-mode-toggle'>Batteriemodus deaktivieren</button>";
    } else {
        html += "<button class='btn btn-primary' type='submit' formmethod='POST' formaction='/battery-mode-toggle'>Batteriemodus aktivieren</button>";
    }
    html += "</div></section>";

    html += "<section class='section'><h2>Buttons 1-6</h2><div class='grid-6'>";
    for (int i = 0; i < 6; i++) {
        html += "<div class='field'><label>Taste " + String(i + 1) + "</label><input name='b" + String(i + 1) + "' maxlength='1' value='" + String(g_config.buttonKeys[i]) + "'></div>";
    }
    html += "</div>";
    html += "<div class='btn-row'><button class='btn btn-primary' type='submit'>Speichern und Neustarten</button><a class='btn btn-secondary' href='/'>Neu laden</a></div>";
    html += "</section></form>";

    html += "<section class='section'><h2>Service</h2><div class='btn-row'>";
    html += "<form method='POST' action='/delete-bonds' onsubmit=\"return confirm('Bondings wirklich loeschen und neu starten?');\"><button class='btn btn-warning' type='submit'>Bondings loeschen</button></form>";
    html += "<form method='POST' action='/reboot' onsubmit=\"return confirm('ESP32 wirklich neu starten?');\"><button class='btn btn-danger' type='submit'>Neustarten</button></form>";
    html += "</div><div class='hint'>Bei BLE-Problemen Geraet auf dem Smartphone ignorieren und neu koppeln.</div></section>";

    html += "<section class='section'><h2>Last 20 Logs</h2>" + logsHtml() + "</section>";

    html += pageShellEnd();
    return html;
}

static String buildWifiPage() {
    String html = pageShellStart("WiFi Setup", PAGE_WIFI);

    html += "<section class='section'><h2>WiFi Manager (AP + STA)</h2>";
    html += "<form method='POST' action='/wifi-save'>";
    html += "<div class='grid-2'>";
    html += "<div class='field'><label>STA SSID</label><input name='sta_ssid' maxlength='32' value='" + htmlEscape(g_config.staSsid) + "'></div>";
    html += "<div class='field'><label>STA Password</label><input name='sta_password' type='password' maxlength='64' value='" + htmlEscape(g_config.staPassword) + "'></div>";
    html += "</div>";
    html += "<div class='check' style='margin-top:10px'><input type='checkbox' id='sta_auto' name='sta_auto' value='1'";
    if (g_config.staAutoConnect) html += " checked";
    html += "><label for='sta_auto'>Auto-Connect beim Start</label></div>";

    html += "<div class='btn-row'>";
    html += "<button class='btn btn-primary' type='submit'>WLAN Daten speichern</button>";
    html += "<button class='btn btn-secondary' type='submit' formaction='/wifi-connect'>Jetzt verbinden</button>";
    html += "<button class='btn btn-warning' type='submit' formaction='/wifi-disconnect'>Trennen</button>";
    html += "<button class='btn btn-secondary' type='submit' formaction='/wifi-scan'>Scan</button>";
    html += "</div></form>";

    html += "<div class='hint'>STA Status: " + wifiStaStatusText() + " | SSID: " + htmlEscape(wifiStaSsid()) + " | IP: " + wifiStaIp();
    if (wifiStaIsConnected()) {
        html += " | RSSI: " + String(wifiStaRssi()) + " dBm";
    }
    html += "</div>";

    html += "<div style='margin-top:12px'>" + g_wifiScanHtml + "</div>";
    html += "</section>";

    html += "<section class='section'><h2>Network Notes</h2><ul class='list'>";
    html += "<li>Der ESP bleibt im AP-Modus aktiv (SmartRace-Setup).</li>";
    html += "<li>Zusatzlich kann er gleichzeitig als STA mit deinem WLAN verbunden sein.</li>";
    html += "<li>So bleibt die lokale Setup-Seite immer erreichbar.</li>";
    html += "</ul></section>";

    html += "<section class='section'><h2>Last 20 Logs</h2>" + logsHtml() + "</section>";

    html += pageShellEnd();
    return html;
}

static String buildSystemInfoPage() {
    String html = pageShellStart("System Info", PAGE_SYSTEM);

    html += "<section class='section'><h2>Device</h2><ul class='list'>";
    html += "<li>Chip: " + htmlEscape(ESP.getChipModel()) + " Rev " + String(ESP.getChipRevision()) + "</li>";
    html += "<li>CPU: " + String(ESP.getCpuFreqMHz()) + " MHz</li>";
    html += "<li>Flash: " + String((uint32_t)(ESP.getFlashChipSize() / (1024 * 1024))) + " MB</li>";
    html += "<li>Sketch size: " + String(ESP.getSketchSize()) + " bytes</li>";
    html += "<li>Free heap: " + String(ESP.getFreeHeap()) + " bytes</li>";
    html += "<li>Min free heap: " + String(ESP.getMinFreeHeap()) + " bytes</li>";
    html += "<li>Uptime: " + String(millis() / 1000) + " s</li>";
    html += "<li>Reset reason: " + String((int)esp_reset_reason()) + "</li>";
    html += "</ul></section>";

    html += "<section class='section'><h2>Network</h2><ul class='list'>";
    html += "<li>AP: " + String(wifiApIsActive() ? "An" : "Aus") + " (" + String(wifiApSsid()) + ")</li>";
    html += "<li>AP IP: " + WiFi.softAPIP().toString() + "</li>";
    html += "<li>STA Status: " + wifiStaStatusText() + "</li>";
    html += "<li>STA SSID: " + htmlEscape(wifiStaSsid()) + "</li>";
    html += "<li>STA IP: " + wifiStaIp() + "</li>";
    if (wifiStaIsConnected()) {
        html += "<li>STA RSSI: " + String(wifiStaRssi()) + " dBm</li>";
    }
    html += "</ul></section>";

    html += "<section class='section'><h2>Last 20 Logs</h2>" + logsHtml() + "</section>";

    html += pageShellEnd();
    return html;
}

static void handleRoot() {
    server.send(200, "text/html; charset=utf-8", buildOverviewPage());
}

static void handleWifiSetup() {
    server.send(200, "text/html; charset=utf-8", buildWifiPage());
}

static void handleSystemInfo() {
    server.send(200, "text/html; charset=utf-8", buildSystemInfoPage());
}

static void applyWifiConfigFromRequest() {
    g_config.staSsid = server.arg("sta_ssid");
    g_config.staPassword = server.arg("sta_password");
    g_config.staAutoConnect = server.hasArg("sta_auto");
}

static void handleSave() {
    g_config.btName = server.arg("bt_name");
    if (g_config.btName.length() < 1) g_config.btName = "SmartRace BT Control";

    g_config.shortPrefixKey = safeCharFromArg(server.arg("short_prefix"), '7');
    g_config.shortDelayMs = safeUIntFromArg(server.arg("short_delay"), 2000, 0, 10000);
    g_config.sendPrefixOnShortPress = server.hasArg("send_prefix");
    g_config.sendPrefixOnlyOnceUntilLongPress = server.hasArg("prefix_once");
    g_config.longPressKey = safeCharFromArg(server.arg("long_key"), '8');
    g_config.longPressMs = safeUIntFromArg(server.arg("long_time"), 1200, 200, 10000);

    g_config.buttonKeys[0] = safeCharFromArg(server.arg("b1"), '1');
    g_config.buttonKeys[1] = safeCharFromArg(server.arg("b2"), '2');
    g_config.buttonKeys[2] = safeCharFromArg(server.arg("b3"), '3');
    g_config.buttonKeys[3] = safeCharFromArg(server.arg("b4"), '4');
    g_config.buttonKeys[4] = safeCharFromArg(server.arg("b5"), '5');
    g_config.buttonKeys[5] = safeCharFromArg(server.arg("b6"), '6');

    configSave();
    appLog("Config saved, restart requested");

    server.send(200, "text/html; charset=utf-8",
                "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<style>body{font-family:Arial;background:#0b1220;color:#eef4ff;padding:32px}</style></head><body>"
                "<h2>Gespeichert</h2><p>Der ESP32 startet jetzt neu.</p></body></html>");
    delay(500);
    ESP.restart();
}

static void handleWifiSave() {
    applyWifiConfigFromRequest();
    configSave();
    appLog(String("WiFi config saved (SSID=") + g_config.staSsid + ")");
    server.sendHeader("Location", "/wifi-setup");
    server.send(303, "text/plain", "");
}

static void handleWifiConnect() {
    applyWifiConfigFromRequest();
    configSave();
    bool ok = wifiStaConnect();
    appLog(String("WiFi connect action: ") + (ok ? "success" : "failed"));
    server.sendHeader("Location", "/wifi-setup");
    server.send(303, "text/plain", "");
}

static void handleWifiDisconnect() {
    wifiStaDisconnect();
    appLog("WiFi disconnect action");
    server.sendHeader("Location", "/wifi-setup");
    server.send(303, "text/plain", "");
}

static void handleWifiScan() {
    int n = WiFi.scanNetworks();
    if (n <= 0) {
        g_wifiScanHtml = "<div class='hint'>Keine Netzwerke gefunden.</div>";
    } else {
        String html = "<div class='field'><label>Scan Ergebnis</label><ul class='list'>";
        int count = n > 20 ? 20 : n;
        for (int i = 0; i < count; i++) {
            html += "<li>" + htmlEscape(WiFi.SSID(i)) + " (" + String(WiFi.RSSI(i)) + " dBm";
            if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
                html += ", offen";
            }
            html += ")</li>";
        }
        html += "</ul></div>";
        g_wifiScanHtml = html;
    }
    appLog(String("WiFi scan done: ") + String(n) + " networks");
    server.sendHeader("Location", "/wifi-setup");
    server.send(303, "text/plain", "");
}

static void handleDeleteBonds() {
    appLog("Delete bonds requested");
    server.send(200, "text/html; charset=utf-8",
                "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<style>body{font-family:Arial;background:#0b1220;color:#eef4ff;padding:32px}</style></head><body>"
                "<h2>Bondings loeschen</h2><p>Der ESP32 startet jetzt neu.</p></body></html>");
    delay(400);
    deleteBondsNow(true);
}

static void handleReboot() {
    appLog("Reboot requested");
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
    appLog(String("Battery mode ") + (enableBatteryMode ? "enabled" : "disabled"));

    server.sendHeader("Location", "/");
    server.send(303, "text/plain", "");
}

void webUiInit() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/wifi-setup", HTTP_GET, handleWifiSetup);
    server.on("/system-info", HTTP_GET, handleSystemInfo);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/wifi-save", HTTP_POST, handleWifiSave);
    server.on("/wifi-connect", HTTP_POST, handleWifiConnect);
    server.on("/wifi-disconnect", HTTP_POST, handleWifiDisconnect);
    server.on("/wifi-scan", HTTP_POST, handleWifiScan);
    server.on("/delete-bonds", HTTP_POST, handleDeleteBonds);
    server.on("/reboot", HTTP_POST, handleReboot);
    server.on("/battery-mode-toggle", HTTP_POST, handleBatteryModeToggle);
    server.begin();

    appLog("Web UI started");
    appLog("Open browser: http://192.168.4.1");
}

void webUiLoop() {
    server.handleClient();
}
