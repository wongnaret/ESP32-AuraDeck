/**
 * @file network_manager.cpp
 * @brief Manages asynchronous Wi-Fi, NTP sync with RTC, and MQTT subscriptions.
 */

#include "network_manager.h"
#include "config.h"
#include "ui/ui_manager.h"

// External global UI Manager reference
extern UIManager g_ui;

// Singleton pointer for the static callback routing
static AuraNetworkManager* s_instance = nullptr;

AuraNetworkManager::AuraNetworkManager() {
    s_instance = this;
}

AuraNetworkManager::~AuraNetworkManager() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

bool AuraNetworkManager::begin(PCF85063RTC* rtc) {
    m_rtc = rtc;

    Serial.printf("📡 Starting Wi-Fi Station... Target SSID: %s\n", WIFI_SSID);

    // Reset Wi-Fi configuration and clean state before connecting
    WiFi.persistent(false);
    WiFi.disconnect(true);
    delay(100);

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    // Allow connection to legacy WPA-PSK (Sec: 2) access points
    WiFi.setMinSecurity(WIFI_AUTH_WPA_PSK);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Setup MQTT client
    m_mqttClient.setClient(m_espClient);
    m_mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    m_mqttClient.setCallback(staticMqttCallback);
    m_mqttClient.setBufferSize(1024 * 4); // Limit to 4KB buffer for large stock/calendar payloads

    return true;
}

void AuraNetworkManager::connectWifi() {
    if (WiFi.status() == WL_CONNECTED) return;

    uint32_t now = millis();
    // Non-blocking reconnect attempt every 15 seconds
    if (now - m_lastWifiCheckTime > 15000) {
        m_lastWifiCheckTime = now;
        uint8_t status = WiFi.status();
        
        Serial.printf("📶 Wi-Fi Status: %d (%s). Retrying connection to SSID [%s]...\n", 
                      status, 
                      status == 1 ? "WL_NO_SSID_AVAIL (SSID Not Found/Security Mismatch)" : 
                      status == 4 ? "WL_CONNECT_FAILED (Auth Error)" : 
                      status == 6 ? "WL_DISCONNECTED" : "CONNECTING",
                      WIFI_SSID);

        int targetChannel = 0;

        // If SSID is not found or disconnected, disconnect first and run a clean Wi-Fi scan
        if (status == WL_NO_SSID_AVAIL || status == WL_DISCONNECTED) {
            Serial.println("🔍 Running 2.4GHz Wi-Fi scan to check visible SSIDs...");
            WiFi.disconnect();
            delay(100);
            int n = WiFi.scanNetworks(false, true); // false = synchronous scan, true = show hidden SSIDs
            if (n < 0) {
                Serial.printf("  ⚠️ Wi-Fi Scan failed with error code: %d\n", n);
            } else if (n == 0) {
                Serial.println("  ⚠️ No 2.4GHz Wi-Fi networks found! Check ESP32 Antenna hardware.");
            } else {
                Serial.printf("  Found %d 2.4GHz Wi-Fi network(s) nearby:\n", n);
                for (int i = 0; i < n; ++i) {
                    String scannedSsid = WiFi.SSID(i);
                    int rssi = WiFi.RSSI(i);
                    int channel = WiFi.channel(i);
                    int sec = WiFi.encryptionType(i);

                    Serial.printf("    [%d] SSID: '%s' | RSSI: %d dBm | Ch: %d | Sec: %d\n",
                                  i + 1, scannedSsid.c_str(), rssi, channel, sec);

                    if (scannedSsid == WIFI_SSID) {
                        targetChannel = channel;
                        Serial.printf("    🎯 Target AP '%s' detected on Channel %d (Sec: %d)!\n", 
                                      WIFI_SSID, targetChannel, sec);
                    }
                }
            }
            WiFi.scanDelete();
        }

        WiFi.setMinSecurity(WIFI_AUTH_WPA_PSK);
        if (targetChannel > 0) {
            Serial.printf("⚡ Direct connecting to [%s] on Channel %d...\n", WIFI_SSID, targetChannel);
            WiFi.begin(WIFI_SSID, WIFI_PASS, targetChannel);
        } else {
            WiFi.begin(WIFI_SSID, WIFI_PASS);
        }
    }
}

void AuraNetworkManager::connectMqtt() {
    if (m_mqttClient.connected()) return;

    uint32_t now = millis();
    // Non-blocking MQTT reconnect attempt every 8 seconds
    if (now - m_lastMqttReconnectTime > 8000) {
        m_lastMqttReconnectTime = now;
        
        IPAddress targetServerIp;
        IPAddress gatewayIp = WiFi.gatewayIP();

        // Check if gateway matches Raspberry Pi Hotspot IP or if MQTT_SERVER is explicit
        if (targetServerIp.fromString(MQTT_SERVER) && targetServerIp != IPAddress(0, 0, 0, 0)) {
            // Use explicit MQTT_SERVER config
        } else if (gatewayIp != IPAddress(0, 0, 0, 0)) {
            targetServerIp = gatewayIp;
        } else {
            targetServerIp.fromString("10.42.0.1");
        }

        m_mqttClient.setServer(targetServerIp, MQTT_PORT);

        Serial.printf("🔌 Connecting to Mosquitto Broker at %s:%d...\n", 
                      targetServerIp.toString().c_str(), MQTT_PORT);
        String clientId = "AuraDeckScreen-" + String((uint32_t)ESP.getEfuseMac());
        
        if (m_mqttClient.connect(clientId.c_str())) {
            Serial.println("✅ Connected to MQTT Broker successfully.");

            // Subscribe to generic dev/test topics (backward compat with Developer Sandbox)
            m_mqttClient.subscribe("auradeck/spotify");
            m_mqttClient.subscribe("auradeck/calendar");
            m_mqttClient.subscribe("auradeck/todos");
            m_mqttClient.subscribe("auradeck/stocks");
            m_mqttClient.subscribe("auradeck/ga4");
            m_mqttClient.subscribe("auradeck/gcp");
            m_mqttClient.subscribe("auradeck/analytics");
            m_mqttClient.subscribe("auradeck/antigravity");
            m_mqttClient.subscribe("auradeck/weather");
            m_mqttClient.subscribe("auradeck/time_sync");
            Serial.println("📬 Subscribed to generic AuraDeck topics (dev mode).");

            // Restore device-specific topics from NVS if previously paired
            if (m_deviceMac[0] == '\0') {
                m_prefs.begin("auradeck", true); // read-only
                String savedMac = m_prefs.getString("device_mac", "");
                m_prefs.end();
                if (savedMac.length() > 0) {
                    strncpy(m_deviceMac, savedMac.c_str(), sizeof(m_deviceMac) - 1);
                    Serial.printf("🔑 Restored pairing MAC from NVS: %s\n", m_deviceMac);
                }
            }

            // If device is paired (fresh or restored from NVS), subscribe to device topics
            if (m_deviceMac[0] != '\0') {
                subscribeDeviceTopics(m_deviceMac);
            }
        } else {
            Serial.printf("❌ MQTT connection failed! State error code: %d\n", m_mqttClient.state());
        }
    }
}

void AuraNetworkManager::subscribeDeviceTopics(const char* mac) {
    strncpy(m_deviceMac, mac, sizeof(m_deviceMac) - 1);

    // Persist MAC to NVS so device-specific topics survive firmware reboots
    m_prefs.begin("auradeck", false); // read-write
    m_prefs.putString("device_mac", mac);
    m_prefs.end();
    Serial.printf("💾 Pairing MAC saved to NVS: %s\n", mac);

    if (!m_mqttClient.connected()) {
        Serial.println("[Network] subscribeDeviceTopics: MQTT not connected. Will subscribe on reconnect.");
        return;
    }

    // Subscribe to per-device MAC-addressed production topics as per API.md
    const char* services[] = { "spotify", "calendar", "todos", "stocks", "ga4", "gcp", "analytics", "antigravity", "weather", "time_sync" };
    for (const char* svc : services) {
        char topic[64];
        snprintf(topic, sizeof(topic), "auradeck/device/%s/%s", mac, svc);
        m_mqttClient.subscribe(topic);
        Serial.printf("📬 Subscribed to device topic: %s\n", topic);
    }
}

bool AuraNetworkManager::syncNTPTime() {
    if (m_timeSynced || !isConnected()) return false;

    uint32_t now = millis();

    // Start background SNTP daemon ONCE on connect, or retry every 30 seconds if unsynced
    if (!m_sntpStarted || (now - m_lastNtpRetryTime > 30000)) {
        m_sntpStarted = true;
        m_lastNtpRetryTime = now;
        String gwIp = WiFi.gatewayIP().toString();
        Serial.printf("🌐 Starting background SNTP daemon (NTP1: %s, NTP2: %s, GW: %s)...\n",
                      NTP_SERVER_1, NTP_SERVER_2, gwIp.c_str());
        configTime(UTC_OFFSET_SECS, 0, NTP_SERVER_1, NTP_SERVER_2, gwIp.c_str());
    }

    // Non-blocking check (10ms timeout) for time synchronization
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10)) {
        if (timeinfo.tm_year > 120) { // Valid year after 2020
            DateTime ntp_dt(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            
            if (m_rtc) {
                m_rtc->adjust(ntp_dt);
            }
            m_timeSynced = true;
            Serial.printf("🌐 Success: NTP clock is synced [%04d-%02d-%02d %02d:%02d:%02d] and hardware RTC updated.\n",
                          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            return true;
        }
    }

    return false;
}

void AuraNetworkManager::tick() {
    connectWifi();

    if (isConnected()) {
        // Non-blocking NTP time sync check
        if (!m_timeSynced) {
            syncNTPTime();
        }

        connectMqtt();
        
        if (isMqttConnected()) {
            m_mqttClient.loop();
        }
    }
}

// Static bridge routing callback
void AuraNetworkManager::staticMqttCallback(char* topic, byte* payload, unsigned int length) {
    if (s_instance == nullptr) return;

    // NOTE: ArduinoJson v6 requires char* (not byte*/uint8_t*) for reliable
    // zero-copy mutable mode. Casting byte* → char* ensures the correct reader path.
    // Also: payload is PubSubClient's internal buffer — NOT null-terminated.
    // We temporarily null-terminate it for robust parsing (safe: PubSubClient
    // guarantees payload[length] is within the allocated setBufferSize() region).
    char* buf = reinterpret_cast<char*>(payload);
    char savedByte = buf[length];
    buf[length] = '\0'; // Temporarily null-terminate

    DynamicJsonDocument doc(8192); // 8KB: safe margin for large watchlists (7+ items)
    // Use (const char*) to force COPY mode — strings are owned by doc's pool.
    // This is critical: if we use (char*), ArduinoJson uses zero-copy mode and stores
    // key/value strings as raw pointers into PubSubClient's buffer.  Once the callback
    // returns the buffer is reused, making all cached string accesses (key lookups like
    // stock["price"]) return stale garbage → zeros on screen after page navigation.
    DeserializationError error = deserializeJson(doc, (const char*)buf);

    buf[length] = savedByte; // Restore original byte immediately after parsing

    if (error) {
        Serial.printf("❌ Error: Failed to deserialize JSON on topic [%s]: %s (len=%u)\n",
                      topic, error.c_str(), length);
        return;
    }

    Serial.printf("  DBG: len=%u, is_array=%d, size=%u\n",
                  length, (int)doc.is<JsonArrayConst>(), (unsigned)doc.size());
    s_instance->handleMqttMessage(topic, doc.as<JsonVariantConst>());
}

void AuraNetworkManager::handleMqttMessage(const char* topic, JsonVariantConst data) {
    Serial.printf("\n📥 [MQTT Received] Topic: %s\n", topic);

    // Helper: extract service name from either generic or device-specific topics
    // Generic:  "auradeck/{service}"
    // Device:   "auradeck/device/{mac}/{service}"
    bool isDeviceTopic = false;
    const char* service = nullptr;
    const char* prefix = "auradeck/device/";
    if (strncmp(topic, prefix, strlen(prefix)) == 0) {
        isDeviceTopic = true;
        // Device-specific topic: skip past "auradeck/device/{mac}/"
        const char* macEnd = strchr(topic + strlen(prefix), '/');
        service = (macEnd != nullptr) ? macEnd + 1 : nullptr;
    } else if (strncmp(topic, "auradeck/", 9) == 0) {
        // Generic dev topic: skip past "auradeck/"
        service = topic + 9;
    }

    if (service == nullptr) {
        Serial.printf("⚠️ Unrecognized MQTT topic format: %s\n", topic);
        return;
    }

    if (strcmp(service, "spotify") == 0) {
        bool isPlaying = data["is_playing"] | false;
        if (isPlaying) {
            const char* track = data["title"] | data["track"] | "Unknown Track";
            const char* artist = data["artist"] | "Unknown Artist";
            int progressMs = data["progress_ms"] | 0;
            int durationMs = data["duration_ms"] | 0;
            int progress = (progressMs > 0) ? progressMs / 1000 : (data["progress"] | 0);
            int duration = (durationMs > 0) ? durationMs / 1000 : (data["duration"] | 0);
            Serial.printf("  🎵 Spotify Playing: %s - %s [%d/%ds]\n", track, artist, progress, duration);
        } else {
            Serial.println("  🎵 Spotify Offline (No track active)");
        }
    } 
    else if (strcmp(service, "calendar") == 0) {
        JsonArrayConst monthEvents = data["month_days_with_events"].as<JsonArrayConst>();
        Serial.print("  📅 Calendar Month Event Days: ");
        for (int val : monthEvents) { Serial.printf("%d ", val); }
        Serial.println();

        JsonArrayConst events = data["events"].as<JsonArrayConst>();
        Serial.println("  📅 Daily Agendas:");
        for (JsonObjectConst event : events) {
            const char* time  = event["time"]  | "";
            const char* title = event["title"] | "No Title";
            bool is_today     = event["is_today"] | false;
            Serial.printf("    - [%s] %s (%s)\n", time, title, is_today ? "Today" : "Tomorrow");
        }
    } 
    else if (strcmp(service, "todos") == 0) {
        // Todos payload is a root array of objects [{title:...}, ...] or wrapped {todos:[...]}
        JsonArrayConst todos;
        if (data.is<JsonArrayConst>()) {
            todos = data.as<JsonArrayConst>();
        } else if (data["todos"].is<JsonArrayConst>()) {
            todos = data["todos"].as<JsonArrayConst>();
        }
        Serial.println("  📋 Google Checklist Items:");
        for (JsonVariantConst item : todos) {
            if (item.is<JsonObjectConst>()) {
                const char* title = item["title"] | "Untitled";
                Serial.printf("    - [ ] %s\n", title);
            } else {
                const char* str = item.as<const char*>();
                Serial.printf("    - [ ] %s\n", str ? str : "(null)");
            }
        }
    } 
    else if (strcmp(service, "stocks") == 0) {
        // Stocks payload is a root array [{symbol:..., price:..., change_pct:...}, ...]
        JsonArrayConst stocks;
        if (data.is<JsonArrayConst>()) {
            stocks = data.as<JsonArrayConst>();
        } else if (data["stocks"].is<JsonArrayConst>()) {
            stocks = data["stocks"].as<JsonArrayConst>();
        }
        Serial.printf("  DBG2: is_arr=%d stocks_sz=%d\n",
                      (int)data.is<JsonArrayConst>(), (int)stocks.size());
        Serial.println("  📈 Watchlist Quotes:");
        int itemIdx = 0;
        for (JsonVariantConst item : stocks) {
            bool isObj = item.is<JsonObjectConst>();
            Serial.printf("  DBG2 item[%d]: isObj=%d\n", itemIdx, (int)isObj);
            JsonObjectConst s = item.as<JsonObjectConst>();
            const char* sym = s["symbol"] | "??";
            float price = s["price"] | 0.0f;
            float pct   = s["change_pct"] | s["change_percent"] | 0.0f;
            Serial.printf("    - %s: %.2f (%+.2f%%)\n", sym, price, pct);
            itemIdx++;
        }
        Serial.printf("  DBG2: total=%d items logged\n", itemIdx);
    } 
    else if (strcmp(service, "analytics") == 0) {
        int   activeUsers = data["ga4_active_users"] | data["active_users"] | 0;
        Serial.printf("  📊 GA4 Active Visitors : %d\n", activeUsers);
        if (data["gcp_billing"].is<JsonArrayConst>()) {
            float totalMtd = 0.0;
            JsonArrayConst billing = data["gcp_billing"].as<JsonArrayConst>();
            for (JsonObjectConst p : billing) { totalMtd += p["cost_mtd"] | 0.0f; }
            Serial.printf("  💵 GCP Cloud Billing   : $%.2f MTD\n", totalMtd);
        }
    } 
    else if (strcmp(service, "antigravity") == 0) {
        float remaining = data["credit_hours_remaining"] | 0.0;
        float percent   = data["percent_quota_used"]     | 0.0;
        Serial.printf("  🛸 Antigravity Credits: %.1f hours remaining (%.1f%% used)\n", remaining, percent);
    }
    else if (strcmp(service, "time_sync") == 0) {
        int yr  = data["year"]   | 0;
        int mon = data["month"]  | 0;
        int day = data["day"]    | 0;
        int hr  = data["hour"]   | 0;
        int min = data["minute"] | 0;
        int sec = data["second"] | 0;

        if (yr >= 2024 && mon >= 1 && mon <= 12 && day >= 1 && day <= 31) {
            DateTime server_dt(yr, mon, day, hr, min, sec);
            if (m_rtc) {
                m_rtc->adjust(server_dt);
            }
            m_timeSynced = true;
            Serial.printf("  ⏰ Time Synced via Backend MQTT: %04d-%02d-%02d %02d:%02d:%02d\n",
                          yr, mon, day, hr, min, sec);
        }
    }

    // Normalize topic to generic form before forwarding to UI pages.
    // This decouples UI dispatch from both generic (auradeck/{service}) and
    // device-specific (auradeck/device/{mac}/{service}) topic formats,
    // so dispatchData() always receives a predictable auradeck/{service} topic.
    char normalizedTopic[48];
    snprintf(normalizedTopic, sizeof(normalizedTopic), "auradeck/%s", service);
    g_ui.dispatchData(normalizedTopic, data);
}

bool AuraNetworkManager::publishCommand(const char* topic, const char* payload) {
    if (!isMqttConnected()) {
        Serial.printf("⚠️ Cannot publish command to %s — MQTT not connected.\n", topic);
        return false;
    }
    bool res = m_mqttClient.publish(topic, payload);
    Serial.printf("🚀 Published command to %s (success=%d): %s\n", topic, res, payload);
    return res;
}

