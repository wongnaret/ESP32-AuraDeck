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
    WiFi.mode(WIFI_STA);
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
        Serial.println("📶 Wi-Fi disconnected or connecting... Retrying...");
        WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
}

void AuraNetworkManager::connectMqtt() {
    if (m_mqttClient.connected()) return;

    uint32_t now = millis();
    // Non-blocking MQTT reconnect attempt every 8 seconds
    if (now - m_lastMqttReconnectTime > 8000) {
        m_lastMqttReconnectTime = now;
        
        Serial.printf("🔌 Connecting to Mosquitto Broker at %s:%d...\n", MQTT_SERVER, MQTT_PORT);
        String clientId = "AuraDeckScreen-" + String((uint32_t)ESP.getEfuseMac());
        
        if (m_mqttClient.connect(clientId.c_str())) {
            Serial.println("✅ Connected to MQTT Broker successfully.");
            
            // Subscribe to all target board telemetry topics
            m_mqttClient.subscribe("auradeck/spotify");
            m_mqttClient.subscribe("auradeck/calendar");
            m_mqttClient.subscribe("auradeck/todos");
            m_mqttClient.subscribe("auradeck/stocks");
            m_mqttClient.subscribe("auradeck/analytics");
            m_mqttClient.subscribe("auradeck/antigravity");
            
            Serial.println("📬 Subscribed to all AuraDeck payload topics.");
        } else {
            Serial.printf("❌ MQTT connection failed! State error code: %d\n", m_mqttClient.state());
        }
    }
}

bool AuraNetworkManager::syncNTPTime() {
    if (m_timeSynced || !isConnected()) return false;

    Serial.println("🌐 Connecting to NTP server to synchronize system clock...");
    configTime(UTC_OFFSET_SECS, 0, NTP_SERVER_1, NTP_SERVER_2);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 3000)) { // wait up to 3 seconds for NTP packet response
        // Convert to DateTime format
        DateTime ntp_dt(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        
        // Push current NTP time to physical RTC registers
        m_rtc->adjust(ntp_dt);
        m_timeSynced = true;
        Serial.println("🌐 Success: NTP clock is synced and hardware RTC registers are updated.");
        return true;
    } else {
        Serial.println("⚠️ Warning: NTP sync timed out. Will retry next loop.");
        return false;
    }
}

void AuraNetworkManager::tick() {
    connectWifi();

    if (isConnected()) {
        // Handle NTP time sync once on initial Wi-Fi connection
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

    // Allocate JSON document buffer (4KB size limit matches config)
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
        Serial.printf("❌ Error: Failed to deserialize JSON on topic [%s]: %s\n", topic, error.c_str());
        return;
    }

    s_instance->handleMqttMessage(topic, doc);
}

void AuraNetworkManager::handleMqttMessage(const char* topic, const JsonDocument& doc) {
    Serial.printf("\n📥 [MQTT Received] Topic: %s\n", topic);

    if (strcmp(topic, "auradeck/spotify") == 0) {
        bool isPlaying = doc["is_playing"] | false;
        if (isPlaying) {
            const char* track = doc["track"] | "Unknown Track";
            const char* artist = doc["artist"] | "Unknown Artist";
            int progress = doc["progress"] | 0;
            int duration = doc["duration"] | 0;
            Serial.printf("  🎵 Spotify Playing: %s - %s [%d/%ds]\n", track, artist, progress, duration);
        } else {
            Serial.println("  🎵 Spotify Offline (No track active)");
        }
    } 
    else if (strcmp(topic, "auradeck/calendar") == 0) {
        JsonArrayConst monthEvents = doc["month_days_with_events"].as<JsonArrayConst>();
        Serial.print("  📅 Calendar Month Event Days: ");
        for (int val : monthEvents) {
            Serial.printf("%d ", val);
        }
        Serial.println();

        JsonArrayConst events = doc["events"].as<JsonArrayConst>();
        Serial.println("  📅 Daily Agendas:");
        for (JsonObjectConst event : events) {
            const char* time = event["time"] | "";
            const char* title = event["title"] | "No Title";
            bool is_today = event["is_today"] | false;
            Serial.printf("    - [%s] %s (%s)\n", time, title, is_today ? "Today" : "Tomorrow");
        }
    } 
    else if (strcmp(topic, "auradeck/todos") == 0) {
        JsonArrayConst todos = doc["todos"].as<JsonArrayConst>();
        Serial.println("  📋 Google Checklist Items:");
        for (const char* todo : todos) {
            Serial.printf("    - [ ] %s\n", todo);
        }
    } 
    else if (strcmp(topic, "auradeck/stocks") == 0) {
        JsonArrayConst stocks = doc["stocks"].as<JsonArrayConst>();
        Serial.println("  📈 Watchlist Quotes:");
        for (JsonObjectConst s : stocks) {
            const char* sym = s["symbol"] | "";
            float price = s["price"] | 0.0;
            float pct = s["change_percent"] | 0.0;
            Serial.printf("    - %s: %s%.2f (%+.2f%%)\n", sym, (strcmp(sym, "GOLD") == 0) ? "฿" : "$", price, pct);
        }
    } 
    else if (strcmp(topic, "auradeck/analytics") == 0) {
        int activeUsers = doc["active_users"] | 0;
        float mtdBilling = doc["mtd_billing"] | 0.0;
        Serial.printf("  📊 GA4 Active Visitors : %d\n", activeUsers);
        Serial.printf("  💵 GCP Cloud Billing   : $%.2f MTD\n", mtdBilling);
    } 
    else if (strcmp(topic, "auradeck/antigravity") == 0) {
        float remaining = doc["credit_hours_remaining"] | 0.0;
        float percent = doc["percent_quota_used"] | 0.0;
        Serial.printf("  🛸 Antigravity Credits: %.1f hours remaining (%.1f%% used)\n", remaining, percent);
    }

    // Forward telemetry payload dynamically to active pages in UI
    g_ui.dispatchData(topic, doc);
}
