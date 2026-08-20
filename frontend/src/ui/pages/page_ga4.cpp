/**
 * @file page_ga4.cpp
 * @brief Source for Screen 6 (Dedicated Google Analytics 4) of the AuraDeck dashboard.
 * Layout:
 * - Left Zone: Big 30m Active Users counter (48px) + 2x2 Grid (28D users, new users, avg time, events)
 * - Right Zone: Top Active Users by City panel (5 rows)
 */

#include "page_ga4.h"
#include "ui/thai_reshaper.h"
#include "ui/fonts/lv_font_prompt.h"
#include <Arduino.h>

static lv_obj_t* s_active30mLabel = nullptr;
static lv_obj_t* s_u28Label = nullptr;
static lv_obj_t* s_newUsersLabel = nullptr;
static lv_obj_t* s_avgEngageLabel = nullptr;
static lv_obj_t* s_eventCountLabel = nullptr;
static lv_obj_t* s_cityLabels[5] = { nullptr };

void create_page_ga4(lv_obj_t* parent) {
    // Reset static pointers
    s_active30mLabel   = nullptr;
    s_u28Label         = nullptr;
    s_newUsersLabel    = nullptr;
    s_avgEngageLabel   = nullptr;
    s_eventCountLabel  = nullptr;
    for (int i = 0; i < 5; i++) {
        s_cityLabels[i] = nullptr;
    }

    // ==========================================
    // Left Zone: Realtime Counter + 2x2 Stats Grid
    // ==========================================
    lv_obj_t* leftBox = lv_obj_create(parent);
    lv_obj_set_size(leftBox, 230, 240);
    lv_obj_align(leftBox, LV_ALIGN_TOP_LEFT, 12, 6);
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
    static lv_point_t sep_pts[] = { {10, 72}, {210, 72} };
    lv_line_set_points(sep, sep_pts, 2);
    lv_obj_set_style_line_width(sep, 1, 0);
    lv_obj_set_style_line_color(sep, lv_color_black(), 0);

    // 2x2 Grid (y = 78 to 226)
    // Cell 1 (Top-Left): 28D Users
    lv_obj_t* c1Hdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(c1Hdr, &lv_font_montserrat_12, 0);
    lv_obj_align(c1Hdr, LV_ALIGN_TOP_LEFT, 6, 78);
    lv_label_set_text(c1Hdr, "28D Users");

    s_u28Label = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_u28Label, &lv_font_montserrat_16, 0);
    lv_obj_align(s_u28Label, LV_ALIGN_TOP_LEFT, 6, 96);
    lv_label_set_text(s_u28Label, "14.2K");

    // Cell 2 (Top-Right): New Users
    lv_obj_t* c2Hdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(c2Hdr, &lv_font_montserrat_12, 0);
    lv_obj_align(c2Hdr, LV_ALIGN_TOP_LEFT, 118, 78);
    lv_label_set_text(c2Hdr, "New Users");

    s_newUsersLabel = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_newUsersLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_newUsersLabel, LV_ALIGN_TOP_LEFT, 118, 96);
    lv_label_set_text(s_newUsersLabel, "1.8K");

    // Cell 3 (Bottom-Left): Avg Engagement
    lv_obj_t* c3Hdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(c3Hdr, &lv_font_montserrat_12, 0);
    lv_obj_align(c3Hdr, LV_ALIGN_TOP_LEFT, 6, 126);
    lv_label_set_text(c3Hdr, "Avg Engage");

    s_avgEngageLabel = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_avgEngageLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_avgEngageLabel, LV_ALIGN_TOP_LEFT, 6, 144);
    lv_label_set_text(s_avgEngageLabel, "2m 15s");

    // Cell 4 (Bottom-Right): Event Count
    lv_obj_t* c4Hdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(c4Hdr, &lv_font_montserrat_12, 0);
    lv_obj_align(c4Hdr, LV_ALIGN_TOP_LEFT, 118, 126);
    lv_label_set_text(c4Hdr, "Event Count");

    s_eventCountLabel = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_eventCountLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_eventCountLabel, LV_ALIGN_TOP_LEFT, 118, 144);
    lv_label_set_text(s_eventCountLabel, "92.4K");

    // ==========================================
    // Right Zone: Top Cities Panel
    // ==========================================
    lv_obj_t* rightBox = lv_obj_create(parent);
    lv_obj_set_size(rightBox, 138, 240);
    lv_obj_align(rightBox, LV_ALIGN_TOP_RIGHT, -12, 6);
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
    static lv_point_t city_sep_pts[] = { {4, 22}, {124, 22} };
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
        lv_obj_align(s_cityLabels[i], LV_ALIGN_TOP_LEFT, 4, 28 + (i * 30));
        lv_label_set_text(s_cityLabels[i], ThaiReshaper::reshape(defaultCities[i]).c_str());
    }
}

void update_page_ga4(JsonVariantConst data) {
    // 1. Realtime Active Users (30m)
    int act30 = data["active_users_30m"] | data["ga4_active_users"] | 0;
    if (s_active30mLabel && act30 > 0) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", act30);
        lv_label_set_text(s_active30mLabel, buf);
    }

    // 2. 28D Users
    const char* u28 = data["active_28d_users"] | nullptr;
    if (s_u28Label && u28) {
        lv_label_set_text(s_u28Label, u28);
    }

    // 3. New Users
    const char* nu = data["new_users"] | nullptr;
    if (s_newUsersLabel && nu) {
        lv_label_set_text(s_newUsersLabel, nu);
    }

    // 4. Avg Engagement Time
    const char* avgTime = data["avg_engagement_time"] | nullptr;
    if (s_avgEngageLabel && avgTime) {
        lv_label_set_text(s_avgEngageLabel, avgTime);
    }

    // 5. Event Count
    const char* ev = data["event_count"] | nullptr;
    if (s_eventCountLabel && ev) {
        lv_label_set_text(s_eventCountLabel, ev);
    }

    // 6. Top Cities List
    JsonArrayConst cities = data["top_cities"].as<JsonArrayConst>();
    if (!cities.isNull()) {
        int count = min((int)cities.size(), 5);
        for (int i = 0; i < count; i++) {
            JsonObjectConst item = cities[i];
            const char* name = item["city"] | "Other";
            int cUsers = item["active_users"] | 0;

            if (s_cityLabels[i]) {
                char cBuf[64];
                snprintf(cBuf, sizeof(cBuf), "%d. %s (%d)", i + 1, name, cUsers);
                String reshaped = ThaiReshaper::reshape(cBuf);
                lv_label_set_text(s_cityLabels[i], reshaped.c_str());
            }
        }
    }
}

void destroy_page_ga4() {
    s_active30mLabel   = nullptr;
    s_u28Label         = nullptr;
    s_newUsersLabel    = nullptr;
    s_avgEngageLabel   = nullptr;
    s_eventCountLabel  = nullptr;
    for (int i = 0; i < 5; i++) {
        s_cityLabels[i] = nullptr;
    }
}
