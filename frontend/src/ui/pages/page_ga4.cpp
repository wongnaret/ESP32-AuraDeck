/**
 * @file page_ga4.cpp
 * @brief Source for Screen 6 (Dedicated Google Analytics 4 Multi-Property) of the AuraDeck dashboard.
 * Layout:
 * - Top Bar: Property Title & Pagination Indicator [1/N], Switch Hint
 * - Left Zone (230x216): Big 30m Active Users counter (48px) + 2x2 Grid (28D users, new users, avg time, events)
 * - Right Zone (138x216): Top Active Users by City panel (5 rows) with ThaiReshaper
 */

#include "page_ga4.h"
#include "ui/thai_reshaper.h"
#include "ui/fonts/lv_font_prompt.h"
#include <Arduino.h>

static lv_obj_t* s_propertyTitleLabel = nullptr;
static lv_obj_t* s_switchHintLabel    = nullptr;
static lv_obj_t* s_active30mLabel     = nullptr;
static lv_obj_t* s_u28Label           = nullptr;
static lv_obj_t* s_newUsersLabel      = nullptr;
static lv_obj_t* s_avgEngageLabel     = nullptr;
static lv_obj_t* s_eventCountLabel    = nullptr;
static lv_obj_t* s_cityLabels[5]      = { nullptr };

static int s_activeGa4Idx = 0;
static DynamicJsonDocument* s_cachedGa4Doc = nullptr;

static void render_current_property();

void create_page_ga4(lv_obj_t* parent) {
    // Reset static pointers
    s_propertyTitleLabel = nullptr;
    s_switchHintLabel    = nullptr;
    s_active30mLabel     = nullptr;
    s_u28Label           = nullptr;
    s_newUsersLabel      = nullptr;
    s_avgEngageLabel     = nullptr;
    s_eventCountLabel    = nullptr;
    for (int i = 0; i < 5; i++) {
        s_cityLabels[i] = nullptr;
    }

    // ==========================================
    // Top Bar: Property Title & Switch Hint
    // ==========================================
    s_propertyTitleLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_propertyTitleLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_propertyTitleLabel, lv_color_black(), 0);
    lv_obj_align(s_propertyTitleLabel, LV_ALIGN_TOP_LEFT, 14, 6);
    lv_label_set_text(s_propertyTitleLabel, "GA4 [1/1]: Main Website");

    s_switchHintLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_switchHintLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_switchHintLabel, LV_ALIGN_TOP_RIGHT, -14, 8);
    lv_label_set_text(s_switchHintLabel, "BOOT: Switch Property");

    // ==========================================
    // Left Zone: Realtime Counter + 2x2 Stats Grid (230x216)
    // ==========================================
    lv_obj_t* leftBox = lv_obj_create(parent);
    lv_obj_set_size(leftBox, 230, 216);
    lv_obj_align(leftBox, LV_ALIGN_TOP_LEFT, 12, 28);
    lv_obj_set_style_radius(leftBox, 4, 0);
    lv_obj_set_style_border_color(leftBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(leftBox, 1, 0);
    lv_obj_set_style_bg_color(leftBox, lv_color_white(), 0);
    lv_obj_set_style_pad_all(leftBox, 4, 0);
    lv_obj_clear_flag(leftBox, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Realtime Active Users Header
    lv_obj_t* rtHdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(rtHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(rtHdr, LV_ALIGN_TOP_MID, 0, 2);
    lv_label_set_text(rtHdr, "ACTIVE NOW (LAST 30M)");

    // Big 48px Counter
    s_active30mLabel = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_active30mLabel, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(s_active30mLabel, lv_color_black(), 0);
    lv_obj_align(s_active30mLabel, LV_ALIGN_TOP_MID, 0, 16);
    lv_label_set_text(s_active30mLabel, "34");

    // Divider Line
    lv_obj_t* sep = lv_line_create(leftBox);
    static lv_point_t sep_pts[] = { {10, 68}, {210, 68} };
    lv_line_set_points(sep, sep_pts, 2);
    lv_obj_set_style_line_width(sep, 1, 0);
    lv_obj_set_style_line_color(sep, lv_color_black(), 0);

    // 2x2 Grid
    // Cell 1 (Top-Left): 28D Users
    lv_obj_t* c1Hdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(c1Hdr, &lv_font_montserrat_12, 0);
    lv_obj_align(c1Hdr, LV_ALIGN_TOP_LEFT, 6, 74);
    lv_label_set_text(c1Hdr, "28D Users");

    s_u28Label = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_u28Label, &lv_font_montserrat_16, 0);
    lv_obj_align(s_u28Label, LV_ALIGN_TOP_LEFT, 6, 92);
    lv_label_set_text(s_u28Label, "14.2K");

    // Cell 2 (Top-Right): New Users
    lv_obj_t* c2Hdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(c2Hdr, &lv_font_montserrat_12, 0);
    lv_obj_align(c2Hdr, LV_ALIGN_TOP_LEFT, 118, 74);
    lv_label_set_text(c2Hdr, "New Users");

    s_newUsersLabel = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_newUsersLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_newUsersLabel, LV_ALIGN_TOP_LEFT, 118, 92);
    lv_label_set_text(s_newUsersLabel, "1.8K");

    // Cell 3 (Bottom-Left): Avg Engagement
    lv_obj_t* c3Hdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(c3Hdr, &lv_font_montserrat_12, 0);
    lv_obj_align(c3Hdr, LV_ALIGN_TOP_LEFT, 6, 120);
    lv_label_set_text(c3Hdr, "Avg Engage");

    s_avgEngageLabel = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_avgEngageLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_avgEngageLabel, LV_ALIGN_TOP_LEFT, 6, 138);
    lv_label_set_text(s_avgEngageLabel, "2m 15s");

    // Cell 4 (Bottom-Right): Event Count
    lv_obj_t* c4Hdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(c4Hdr, &lv_font_montserrat_12, 0);
    lv_obj_align(c4Hdr, LV_ALIGN_TOP_LEFT, 118, 120);
    lv_label_set_text(c4Hdr, "Event Count");

    s_eventCountLabel = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_eventCountLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_eventCountLabel, LV_ALIGN_TOP_LEFT, 118, 138);
    lv_label_set_text(s_eventCountLabel, "92.4K");

    // ==========================================
    // Right Zone: Top Cities Panel (138x216)
    // ==========================================
    lv_obj_t* rightBox = lv_obj_create(parent);
    lv_obj_set_size(rightBox, 138, 216);
    lv_obj_align(rightBox, LV_ALIGN_TOP_RIGHT, -12, 28);
    lv_obj_set_style_radius(rightBox, 4, 0);
    lv_obj_set_style_border_color(rightBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(rightBox, 1, 0);
    lv_obj_set_style_bg_color(rightBox, lv_color_white(), 0);
    lv_obj_set_style_pad_all(rightBox, 4, 0);
    lv_obj_clear_flag(rightBox, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* cityHdr = lv_label_create(rightBox);
    lv_obj_set_style_text_font(cityHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(cityHdr, LV_ALIGN_TOP_LEFT, 4, 2);
    lv_label_set_text(cityHdr, "Active by City");

    lv_obj_t* citySep = lv_line_create(rightBox);
    static lv_point_t city_sep_pts[] = { {4, 20}, {124, 20} };
    lv_line_set_points(citySep, city_sep_pts, 2);
    lv_obj_set_style_line_width(citySep, 1, 0);
    lv_obj_set_style_line_color(citySep, lv_color_black(), 0);

    const char* defaultCities[] = {
        "1. Bangkok (18)",
        "2. Chiang Mai (6)",
        "3. Nonthaburi (4)",
        "4. Phuket (3)",
        "5. Chon Buri (2)"
    };

    for (int i = 0; i < 5; i++) {
        s_cityLabels[i] = lv_label_create(rightBox);
        lv_obj_set_style_text_font(s_cityLabels[i], &lv_font_prompt_12, 0);
        lv_obj_set_width(s_cityLabels[i], 126);
        lv_label_set_long_mode(s_cityLabels[i], LV_LABEL_LONG_DOT);
        lv_obj_align(s_cityLabels[i], LV_ALIGN_TOP_LEFT, 4, 26 + (i * 28));
        lv_label_set_text(s_cityLabels[i], ThaiReshaper::reshape(defaultCities[i]).c_str());
    }

    // Replay cached data if available
    render_current_property();
}

static void render_current_property() {
    if (!s_cachedGa4Doc) return;

    JsonObjectConst root = s_cachedGa4Doc->as<JsonObjectConst>();
    JsonArrayConst properties = root["properties"].as<JsonArrayConst>();

    JsonObjectConst prop = root; // default to root object for flat payloads
    int totalProps = 1;

    if (!properties.isNull() && properties.size() > 0) {
        totalProps = properties.size();
        if (s_activeGa4Idx >= totalProps) s_activeGa4Idx = 0;
        prop = properties[s_activeGa4Idx];
    }

    // 1. Update Header Title [Idx/Total]
    if (s_propertyTitleLabel) {
        const char* pName = prop["property_name"] | "GA4 Analytics";
        char tBuf[64];
        snprintf(tBuf, sizeof(tBuf), "GA4 [%d/%d]: %s", s_activeGa4Idx + 1, totalProps, pName);
        lv_label_set_text(s_propertyTitleLabel, tBuf);
    }

    // 2. Realtime Active Users (30m)
    int act30 = prop["active_users_30m"] | prop["ga4_active_users"] | 0;
    if (s_active30mLabel) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", act30 > 0 ? act30 : 34);
        lv_label_set_text(s_active30mLabel, buf);
    }

    // 3. 28D Users
    const char* u28 = prop["active_28d_users"] | "14.2K";
    if (s_u28Label) {
        lv_label_set_text(s_u28Label, u28);
    }

    // 4. New Users
    const char* nu = prop["new_users"] | "1.8K";
    if (s_newUsersLabel) {
        lv_label_set_text(s_newUsersLabel, nu);
    }

    // 5. Avg Engagement Time
    const char* avgTime = prop["avg_engagement_time"] | "2m 15s";
    if (s_avgEngageLabel) {
        lv_label_set_text(s_avgEngageLabel, avgTime);
    }

    // 6. Event Count
    const char* ev = prop["event_count"] | "92.4K";
    if (s_eventCountLabel) {
        lv_label_set_text(s_eventCountLabel, ev);
    }

    // 7. Top Cities List
    JsonArrayConst cities = prop["top_cities"].as<JsonArrayConst>();
    if (!cities.isNull()) {
        int count = min((int)cities.size(), 5);
        for (int i = 0; i < 5; i++) {
            if (s_cityLabels[i]) {
                if (i < count) {
                    JsonObjectConst item = cities[i];
                    const char* name = item["city"] | "Other";
                    int cUsers = item["active_users"] | 0;

                    char cBuf[64];
                    snprintf(cBuf, sizeof(cBuf), "%d. %s (%d)", i + 1, name, cUsers);
                    String reshaped = ThaiReshaper::reshape(cBuf);
                    lv_label_set_text(s_cityLabels[i], reshaped.c_str());
                } else {
                    lv_label_set_text(s_cityLabels[i], "");
                }
            }
        }
    }
}

void update_page_ga4(JsonVariantConst data) {
    if (!s_cachedGa4Doc) {
        s_cachedGa4Doc = new DynamicJsonDocument(8192);
    }
    s_cachedGa4Doc->set(data);
    render_current_property();
}

void cycle_ga4_property() {
    if (!s_cachedGa4Doc) return;
    JsonArrayConst properties = (*s_cachedGa4Doc)["properties"].as<JsonArrayConst>();
    if (properties.isNull() || properties.size() <= 1) return;

    s_activeGa4Idx = (s_activeGa4Idx + 1) % properties.size();
    Serial.printf("🔄 Switched to GA4 Property index %d\n", s_activeGa4Idx);
    render_current_property();
}

void destroy_page_ga4() {
    s_propertyTitleLabel = nullptr;
    s_switchHintLabel    = nullptr;
    s_active30mLabel     = nullptr;
    s_u28Label           = nullptr;
    s_newUsersLabel      = nullptr;
    s_avgEngageLabel     = nullptr;
    s_eventCountLabel    = nullptr;
    for (int i = 0; i < 5; i++) {
        s_cityLabels[i] = nullptr;
    }
}
