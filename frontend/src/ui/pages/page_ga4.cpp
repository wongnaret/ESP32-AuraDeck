/**
 * @file page_ga4.cpp
 * @brief Source for Screen 6 (Dedicated Google Analytics 4 Multi-Property) of the AuraDeck dashboard.
 * Layout:
 * - Top Bar: Property Title & Pagination Indicator [1/N], Switch Hint
 * - Left Zone (232x238): Big 30m Active Now counter (48px) + 3x2 7-Day KPI Grid + 7-Day Device Breakdown
 * - Right Zone (138x238): 7-Day Top Cities (5 rows) + 7-Day Daily Views Trend mini bar chart
 */

#include "page_ga4.h"
#include "ui/thai_reshaper.h"
#include "ui/fonts/lv_font_prompt.h"
#include <Arduino.h>

static lv_obj_t* s_propertyTitleLabel = nullptr;
static lv_obj_t* s_switchHintLabel    = nullptr;
static lv_obj_t* s_active30mLabel     = nullptr;
static lv_obj_t* s_u7Label            = nullptr;
static lv_obj_t* s_newUsersLabel      = nullptr;
static lv_obj_t* s_sessionsLabel      = nullptr;
static lv_obj_t* s_viewsLabel         = nullptr;
static lv_obj_t* s_avgEngageLabel     = nullptr;
static lv_obj_t* s_bounceLabel        = nullptr;
static lv_obj_t* s_deviceLabels[3]    = { nullptr };
static lv_obj_t* s_cityLabels[5]      = { nullptr };
static lv_obj_t* s_trendMaxLabel      = nullptr;
static lv_obj_t* s_trendBars[7]       = { nullptr };

static int s_activeGa4Idx = 0;
static DynamicJsonDocument* s_cachedGa4Doc = nullptr;

static void render_current_property();

static void reset_static_pointers() {
    s_propertyTitleLabel = nullptr;
    s_switchHintLabel    = nullptr;
    s_active30mLabel     = nullptr;
    s_u7Label            = nullptr;
    s_newUsersLabel      = nullptr;
    s_sessionsLabel      = nullptr;
    s_viewsLabel         = nullptr;
    s_avgEngageLabel     = nullptr;
    s_bounceLabel        = nullptr;
    s_trendMaxLabel      = nullptr;
    for (int i = 0; i < 3; i++) s_deviceLabels[i] = nullptr;
    for (int i = 0; i < 5; i++) s_cityLabels[i] = nullptr;
    for (int i = 0; i < 7; i++) s_trendBars[i] = nullptr;
}

static lv_obj_t* create_kpi_cell(lv_obj_t* parent, const char* headerText, int x, int y, const char* initVal) {
    lv_obj_t* hdr = lv_label_create(parent);
    lv_obj_set_style_text_font(hdr, &lv_font_montserrat_12, 0);
    lv_obj_align(hdr, LV_ALIGN_TOP_LEFT, x, y);
    lv_label_set_text(hdr, headerText);

    lv_obj_t* val = lv_label_create(parent);
    lv_obj_set_style_text_font(val, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(val, lv_color_black(), 0);
    lv_obj_align(val, LV_ALIGN_TOP_LEFT, x, y + 14);
    lv_label_set_text(val, initVal);
    return val;
}

void create_page_ga4(lv_obj_t* parent) {
    reset_static_pointers();

    // ==========================================
    // Top Bar: Property Title & Switch Hint
    // ==========================================
    s_propertyTitleLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_propertyTitleLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_propertyTitleLabel, lv_color_black(), 0);
    lv_obj_set_width(s_propertyTitleLabel, 232);
    lv_label_set_long_mode(s_propertyTitleLabel, LV_LABEL_LONG_DOT);
    lv_obj_align(s_propertyTitleLabel, LV_ALIGN_TOP_LEFT, 12, 4);
    lv_label_set_text(s_propertyTitleLabel, "GA4: Waiting...");

    s_switchHintLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_switchHintLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_switchHintLabel, LV_ALIGN_TOP_RIGHT, -12, 6);
    lv_label_set_text(s_switchHintLabel, "BOOT: Switch Property");

    // ==========================================
    // Left Zone: Realtime + 7D KPIs + Devices (232x238)
    // ==========================================
    lv_obj_t* leftBox = lv_obj_create(parent);
    lv_obj_set_size(leftBox, 232, 238);
    lv_obj_align(leftBox, LV_ALIGN_TOP_LEFT, 12, 26);
    lv_obj_set_style_radius(leftBox, 4, 0);
    lv_obj_set_style_border_color(leftBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(leftBox, 1, 0);
    lv_obj_set_style_bg_color(leftBox, lv_color_white(), 0);
    lv_obj_set_style_pad_all(leftBox, 4, 0);
    lv_obj_clear_flag(leftBox, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Realtime Active Users (30m)
    lv_obj_t* rtHdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(rtHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(rtHdr, LV_ALIGN_TOP_MID, 0, 2);
    lv_label_set_text(rtHdr, "ACTIVE NOW (30M)");

    s_active30mLabel = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_active30mLabel, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(s_active30mLabel, lv_color_black(), 0);
    lv_obj_align(s_active30mLabel, LV_ALIGN_TOP_MID, 0, 15);
    lv_label_set_text(s_active30mLabel, "0");

    lv_obj_t* sep1 = lv_line_create(leftBox);
    static lv_point_t sep1_pts[] = { {8, 66}, {214, 66} };
    lv_line_set_points(sep1, sep1_pts, 2);
    lv_obj_set_style_line_width(sep1, 1, 0);
    lv_obj_set_style_line_color(sep1, lv_color_black(), 0);

    // 2. 7-Day Summary 3x2 KPI Grid
    s_u7Label        = create_kpi_cell(leftBox, "7D Users",  6,   70, "--");
    s_newUsersLabel  = create_kpi_cell(leftBox, "New Users", 80,  70, "--");
    s_sessionsLabel  = create_kpi_cell(leftBox, "Sessions",  154, 70, "--");
    s_viewsLabel     = create_kpi_cell(leftBox, "Views",     6,   110, "--");
    s_avgEngageLabel = create_kpi_cell(leftBox, "Avg Time",  80,  110, "--");
    s_bounceLabel    = create_kpi_cell(leftBox, "Bounce",    154, 110, "--");

    lv_obj_t* sep2 = lv_line_create(leftBox);
    static lv_point_t sep2_pts[] = { {8, 150}, {214, 150} };
    lv_line_set_points(sep2, sep2_pts, 2);
    lv_obj_set_style_line_width(sep2, 1, 0);
    lv_obj_set_style_line_color(sep2, lv_color_black(), 0);

    // 3. 7-Day Device Breakdown
    lv_obj_t* devHdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(devHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(devHdr, LV_ALIGN_TOP_LEFT, 6, 155);
    lv_label_set_text(devHdr, "DEVICES (7D)");

    for (int i = 0; i < 3; i++) {
        s_deviceLabels[i] = lv_label_create(leftBox);
        lv_obj_set_style_text_font(s_deviceLabels[i], &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(s_deviceLabels[i], lv_color_black(), 0);
        lv_obj_set_width(s_deviceLabels[i], 210);
        lv_label_set_long_mode(s_deviceLabels[i], LV_LABEL_LONG_DOT);
        lv_obj_align(s_deviceLabels[i], LV_ALIGN_TOP_LEFT, 6, 172 + (i * 18));
        lv_label_set_text(s_deviceLabels[i], i == 0 ? "Waiting for data..." : "");
    }

    // ==========================================
    // Right Zone: 7D Top Cities + 7D Views Trend (138x238)
    // ==========================================
    lv_obj_t* rightBox = lv_obj_create(parent);
    lv_obj_set_size(rightBox, 138, 238);
    lv_obj_align(rightBox, LV_ALIGN_TOP_RIGHT, -12, 26);
    lv_obj_set_style_radius(rightBox, 4, 0);
    lv_obj_set_style_border_color(rightBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(rightBox, 1, 0);
    lv_obj_set_style_bg_color(rightBox, lv_color_white(), 0);
    lv_obj_set_style_pad_all(rightBox, 4, 0);
    lv_obj_clear_flag(rightBox, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* cityHdr = lv_label_create(rightBox);
    lv_obj_set_style_text_font(cityHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(cityHdr, LV_ALIGN_TOP_LEFT, 4, 2);
    lv_label_set_text(cityHdr, "Top Cities (7D)");

    lv_obj_t* citySep = lv_line_create(rightBox);
    static lv_point_t city_sep_pts[] = { {4, 19}, {124, 19} };
    lv_line_set_points(citySep, city_sep_pts, 2);
    lv_obj_set_style_line_width(citySep, 1, 0);
    lv_obj_set_style_line_color(citySep, lv_color_black(), 0);

    for (int i = 0; i < 5; i++) {
        s_cityLabels[i] = lv_label_create(rightBox);
        lv_obj_set_style_text_font(s_cityLabels[i], &lv_font_prompt_12, 0);
        lv_obj_set_style_text_color(s_cityLabels[i], lv_color_black(), 0);
        lv_obj_set_width(s_cityLabels[i], 122);
        lv_label_set_long_mode(s_cityLabels[i], LV_LABEL_LONG_DOT);
        lv_obj_align(s_cityLabels[i], LV_ALIGN_TOP_LEFT, 4, 24 + (i * 24));
        lv_label_set_text(s_cityLabels[i], i == 0 ? "--" : "");
    }

    lv_obj_t* trendSep = lv_line_create(rightBox);
    static lv_point_t trend_sep_pts[] = { {4, 148}, {124, 148} };
    lv_line_set_points(trendSep, trend_sep_pts, 2);
    lv_obj_set_style_line_width(trendSep, 1, 0);
    lv_obj_set_style_line_color(trendSep, lv_color_black(), 0);

    lv_obj_t* trendHdr = lv_label_create(rightBox);
    lv_obj_set_style_text_font(trendHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(trendHdr, LV_ALIGN_TOP_LEFT, 4, 153);
    lv_label_set_text(trendHdr, "7D Views");

    s_trendMaxLabel = lv_label_create(rightBox);
    lv_obj_set_style_text_font(s_trendMaxLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_trendMaxLabel, LV_ALIGN_TOP_RIGHT, -4, 153);
    lv_label_set_text(s_trendMaxLabel, "--");

    for (int i = 0; i < 7; i++) {
        s_trendBars[i] = lv_obj_create(rightBox);
        lv_obj_set_size(s_trendBars[i], 10, 2);
        lv_obj_set_pos(s_trendBars[i], 8 + (i * 16), 220);
        lv_obj_set_style_radius(s_trendBars[i], 1, 0);
        lv_obj_set_style_border_width(s_trendBars[i], 0, 0);
        lv_obj_set_style_bg_color(s_trendBars[i], lv_color_black(), 0);
        lv_obj_clear_flag(s_trendBars[i], LV_OBJ_FLAG_SCROLLABLE);
    }

    lv_obj_t* trendBase = lv_line_create(rightBox);
    static lv_point_t trend_base_pts[] = { {6, 223}, {122, 223} };
    lv_line_set_points(trendBase, trend_base_pts, 2);
    lv_obj_set_style_line_width(trendBase, 1, 0);
    lv_obj_set_style_line_color(trendBase, lv_color_black(), 0);

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

    // 2. Realtime Active Users (30m) — preserve 0 accurately
    int act30 = prop["active_users_30m"] | prop["ga4_active_users"] | 0;
    if (s_active30mLabel) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", act30);
        lv_label_set_text(s_active30mLabel, buf);
    }

    // 3. 7-Day KPI Grid
    const char* u7 = prop["active_7d_users"] | prop["active_28d_users"] | "0";
    if (s_u7Label) lv_label_set_text(s_u7Label, u7);

    const char* nu = prop["new_users"] | "0";
    if (s_newUsersLabel) lv_label_set_text(s_newUsersLabel, nu);

    const char* sess = prop["sessions"] | "0";
    if (s_sessionsLabel) lv_label_set_text(s_sessionsLabel, sess);

    const char* views = prop["page_views"] | prop["event_count"] | "0";
    if (s_viewsLabel) lv_label_set_text(s_viewsLabel, views);

    const char* avgTime = prop["avg_engagement_time"] | "0m 00s";
    if (s_avgEngageLabel) lv_label_set_text(s_avgEngageLabel, avgTime);

    const char* bounce = prop["bounce_rate"] | "0.0%";
    if (s_bounceLabel) lv_label_set_text(s_bounceLabel, bounce);

    // 4. 7-Day Device Breakdown
    JsonArrayConst devices = prop["devices"].as<JsonArrayConst>();
    if (!devices.isNull()) {
        int dCount = min((int)devices.size(), 3);
        for (int i = 0; i < 3; i++) {
            if (!s_deviceLabels[i]) continue;
            if (i < dCount) {
                JsonObjectConst d = devices[i];
                const char* dName = d["device"] | "Other";
                int dUsers = d["active_users"] | 0;
                float dPct = d["pct"] | 0.0f;
                char dBuf[48];
                snprintf(dBuf, sizeof(dBuf), "%-7s %5.1f%% (%d)", dName, dPct, dUsers);
                lv_label_set_text(s_deviceLabels[i], dBuf);
            } else {
                lv_label_set_text(s_deviceLabels[i], "");
            }
        }
    }

    // 5. 7-Day Top Cities List
    JsonArrayConst cities = prop["top_cities"].as<JsonArrayConst>();
    if (!cities.isNull()) {
        int count = min((int)cities.size(), 5);
        for (int i = 0; i < 5; i++) {
            if (!s_cityLabels[i]) continue;
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

    // 6. 7-Day Daily Views Trend Mini Bar Chart
    JsonArrayConst trend = prop["daily_trend"].as<JsonArrayConst>();
    if (!trend.isNull() && trend.size() > 0) {
        int tCount = min((int)trend.size(), 7);
        int maxVal = 0;
        for (int i = 0; i < tCount; i++) {
            int v = trend[i] | 0;
            if (v > maxVal) maxVal = v;
        }
        int scaleMax = max(1, maxVal);
        if (s_trendMaxLabel) {
            char mBuf[16];
            snprintf(mBuf, sizeof(mBuf), "max %d", maxVal);
            lv_label_set_text(s_trendMaxLabel, mBuf);
        }
        int offset = 7 - tCount; // right-align if fewer than 7 days
        for (int i = 0; i < 7; i++) {
            if (!s_trendBars[i]) continue;
            int val = (i >= offset) ? (trend[i - offset] | 0) : 0;
            int barH = (val > 0) ? max(3, (val * 46) / scaleMax) : 1;
            lv_obj_set_size(s_trendBars[i], 10, barH);
            lv_obj_set_pos(s_trendBars[i], 8 + (i * 16), 222 - barH);
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
    reset_static_pointers();
}
