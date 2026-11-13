/**
 * @file page_ga4.cpp
 * @brief Source for Screen 6 (Dedicated Google Analytics 4 Multi-Property) of the AuraDeck dashboard.
 * Layout:
 * - Top Bar: Property Title & Pagination Indicator [1/N], Switch Hint
 * - Left Zone (232x238): Big 30m Active Now counter (48px) + 3x2 7-Day KPI Grid + 7-Day Device Horizontal Bars
 * - Right Zone (138x238): 7-Day Top Cities (marquee on overflow) + 7-Day Daily Views Bar Chart with Y-Axis
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

// Devices: 3 rows with name, horizontal bar, value
static lv_obj_t* s_deviceNameLabels[3] = { nullptr };
static lv_obj_t* s_deviceBars[3]       = { nullptr };
static lv_obj_t* s_deviceValLabels[3]  = { nullptr };

// Top Cities: 5 rows
static lv_obj_t* s_cityLabels[5]      = { nullptr };

// 7D Trend Chart with Y-Axis
static lv_obj_t* s_trendMaxLabel      = nullptr;
static lv_obj_t* s_trendMidLabel      = nullptr;
static lv_obj_t* s_trendZeroLabel     = nullptr;
static lv_obj_t* s_trendBars[7]       = { nullptr };

static int s_activeGa4Idx = 0;
static DynamicJsonDocument* s_cachedGa4Doc = nullptr;

static void render_current_property();

static void format_compact_num(char* buf, size_t sz, int val) {
    if (val >= 1000000) {
        snprintf(buf, sz, "%dM", val / 1000000);
    } else if (val >= 10000) {
        snprintf(buf, sz, "%dk", val / 1000);
    } else if (val >= 1000) {
        snprintf(buf, sz, "%.1fk", val / 1000.0f);
    } else {
        snprintf(buf, sz, "%d", val);
    }
}

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
    s_trendMidLabel      = nullptr;
    s_trendZeroLabel     = nullptr;
    for (int i = 0; i < 3; i++) {
        s_deviceNameLabels[i] = nullptr;
        s_deviceBars[i]       = nullptr;
        s_deviceValLabels[i]  = nullptr;
    }
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
    static lv_point_t sep2_pts[] = { {8, 148}, {214, 148} };
    lv_line_set_points(sep2, sep2_pts, 2);
    lv_obj_set_style_line_width(sep2, 1, 0);
    lv_obj_set_style_line_color(sep2, lv_color_black(), 0);

    // 3. 7-Day Device Breakdown (Horizontal Bars)
    lv_obj_t* devHdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(devHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(devHdr, LV_ALIGN_TOP_LEFT, 6, 153);
    lv_label_set_text(devHdr, "DEVICES (7D)");

    const char* defaultDevNames[] = { "Mobile", "Desktop", "Tablet" };
    for (int i = 0; i < 3; i++) {
        int yRow = 170 + (i * 19);

        // Device Name (e.g. Mobile, Desktop, Tablet)
        s_deviceNameLabels[i] = lv_label_create(leftBox);
        lv_obj_set_style_text_font(s_deviceNameLabels[i], &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(s_deviceNameLabels[i], lv_color_black(), 0);
        lv_obj_set_width(s_deviceNameLabels[i], 52);
        lv_obj_align(s_deviceNameLabels[i], LV_ALIGN_TOP_LEFT, 6, yRow);
        lv_label_set_text(s_deviceNameLabels[i], defaultDevNames[i]);

        // Horizontal Progress Bar
        s_deviceBars[i] = lv_bar_create(leftBox);
        lv_obj_set_size(s_deviceBars[i], 78, 8);
        lv_obj_align(s_deviceBars[i], LV_ALIGN_TOP_LEFT, 60, yRow + 3);
        lv_obj_set_style_bg_color(s_deviceBars[i], lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_border_color(s_deviceBars[i], lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_border_width(s_deviceBars[i], 1, LV_PART_MAIN);
        lv_obj_set_style_radius(s_deviceBars[i], 2, LV_PART_MAIN);
        lv_obj_set_style_bg_color(s_deviceBars[i], lv_color_black(), LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(s_deviceBars[i], LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_set_style_radius(s_deviceBars[i], 1, LV_PART_INDICATOR);
        lv_bar_set_range(s_deviceBars[i], 0, 100);
        lv_bar_set_value(s_deviceBars[i], 0, LV_ANIM_OFF);

        // Value & Percentage Text (e.g. 68.2% (73))
        s_deviceValLabels[i] = lv_label_create(leftBox);
        lv_obj_set_style_text_font(s_deviceValLabels[i], &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(s_deviceValLabels[i], lv_color_black(), 0);
        lv_obj_set_width(s_deviceValLabels[i], 78);
        lv_obj_align(s_deviceValLabels[i], LV_ALIGN_TOP_LEFT, 144, yRow);
        lv_label_set_text(s_deviceValLabels[i], "--");
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

    // City rows with marquee on overflow (Prompt 12 with ThaiReshaper)
    for (int i = 0; i < 5; i++) {
        s_cityLabels[i] = lv_label_create(rightBox);
        lv_obj_set_style_text_font(s_cityLabels[i], &lv_font_prompt_12, 0);
        lv_obj_set_style_text_color(s_cityLabels[i], lv_color_black(), 0);
        lv_obj_set_width(s_cityLabels[i], 124);
        lv_label_set_long_mode(s_cityLabels[i], LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_align(s_cityLabels[i], LV_ALIGN_TOP_LEFT, 4, 23 + (i * 24));
        lv_label_set_text(s_cityLabels[i], i == 0 ? "--" : "");
    }

    lv_obj_t* trendSep = lv_line_create(rightBox);
    static lv_point_t trend_sep_pts[] = { {4, 145}, {124, 145} };
    lv_line_set_points(trendSep, trend_sep_pts, 2);
    lv_obj_set_style_line_width(trendSep, 1, 0);
    lv_obj_set_style_line_color(trendSep, lv_color_black(), 0);

    lv_obj_t* trendHdr = lv_label_create(rightBox);
    lv_obj_set_style_text_font(trendHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(trendHdr, LV_ALIGN_TOP_LEFT, 4, 150);
    lv_label_set_text(trendHdr, "7D Views");

    // Y-Axis Labels (right-aligned to x=22)
    s_trendMaxLabel = lv_label_create(rightBox);
    lv_obj_set_style_text_font(s_trendMaxLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_trendMaxLabel, lv_color_black(), 0);
    lv_obj_set_style_text_align(s_trendMaxLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_width(s_trendMaxLabel, 22);
    lv_obj_align(s_trendMaxLabel, LV_ALIGN_TOP_LEFT, 0, 166);
    lv_label_set_text(s_trendMaxLabel, "--");

    s_trendMidLabel = lv_label_create(rightBox);
    lv_obj_set_style_text_font(s_trendMidLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_trendMidLabel, lv_color_black(), 0);
    lv_obj_set_style_text_align(s_trendMidLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_width(s_trendMidLabel, 22);
    lv_obj_align(s_trendMidLabel, LV_ALIGN_TOP_LEFT, 0, 190);
    lv_label_set_text(s_trendMidLabel, "--");

    s_trendZeroLabel = lv_label_create(rightBox);
    lv_obj_set_style_text_font(s_trendZeroLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_trendZeroLabel, lv_color_black(), 0);
    lv_obj_set_style_text_align(s_trendZeroLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_width(s_trendZeroLabel, 22);
    lv_obj_align(s_trendZeroLabel, LV_ALIGN_TOP_LEFT, 0, 214);
    lv_label_set_text(s_trendZeroLabel, "0");

    // L-shaped Coordinate Axis: vertical line at x=25 (170..222) and baseline at y=222 (25..124)
    static lv_point_t s_axis_pts[] = { {25, 170}, {25, 222}, {124, 222} };
    lv_obj_t* axisLine = lv_line_create(rightBox);
    lv_line_set_points(axisLine, s_axis_pts, 3);
    lv_obj_set_style_line_width(axisLine, 1, 0);
    lv_obj_set_style_line_color(axisLine, lv_color_black(), 0);

    // Y-Axis Ticks
    static lv_point_t s_top_tick_pts[] = { {22, 172}, {25, 172} };
    lv_obj_t* topTick = lv_line_create(rightBox);
    lv_line_set_points(topTick, s_top_tick_pts, 2);
    lv_obj_set_style_line_width(topTick, 1, 0);
    lv_obj_set_style_line_color(topTick, lv_color_black(), 0);

    static lv_point_t s_mid_tick_pts[] = { {22, 196}, {25, 196} };
    lv_obj_t* midTick = lv_line_create(rightBox);
    lv_line_set_points(midTick, s_mid_tick_pts, 2);
    lv_obj_set_style_line_width(midTick, 1, 0);
    lv_obj_set_style_line_color(midTick, lv_color_black(), 0);

    // 7 Daily Bars (width 9px, step 14px, starting at x=29)
    for (int i = 0; i < 7; i++) {
        s_trendBars[i] = lv_obj_create(rightBox);
        lv_obj_set_size(s_trendBars[i], 9, 2);
        lv_obj_set_pos(s_trendBars[i], 29 + (i * 14), 220);
        lv_obj_set_style_radius(s_trendBars[i], 1, 0);
        lv_obj_set_style_border_width(s_trendBars[i], 0, 0);
        lv_obj_set_style_bg_color(s_trendBars[i], lv_color_black(), 0);
        lv_obj_clear_flag(s_trendBars[i], LV_OBJ_FLAG_SCROLLABLE);
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

    // 4. 7-Day Device Breakdown (Horizontal Bars)
    JsonArrayConst devices = prop["devices"].as<JsonArrayConst>();
    int dCount = 0;
    if (!devices.isNull()) {
        dCount = min((int)devices.size(), 3);
    }
    for (int i = 0; i < 3; i++) {
        if (!s_deviceNameLabels[i] || !s_deviceBars[i] || !s_deviceValLabels[i]) continue;
        if (i < dCount) {
            JsonObjectConst d = devices[i];
            const char* dName = d["device"] | "Other";
            int dUsers = d["active_users"] | 0;
            float dPct = d["pct"] | 0.0f;

            lv_label_set_text(s_deviceNameLabels[i], dName);

            int barVal = (int)round(dPct);
            if (dPct > 0.0f && barVal == 0) barVal = 1;
            lv_bar_set_value(s_deviceBars[i], barVal, LV_ANIM_OFF);

            char vBuf[32];
            snprintf(vBuf, sizeof(vBuf), "%.1f%% (%d)", dPct, dUsers);
            lv_label_set_text(s_deviceValLabels[i], vBuf);

            lv_obj_clear_flag(s_deviceNameLabels[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(s_deviceBars[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(s_deviceValLabels[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(s_deviceNameLabels[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(s_deviceBars[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(s_deviceValLabels[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    // 5. 7-Day Top Cities List (Circular Marquee for long city names)
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

    // 6. 7-Day Daily Views Trend Bar Chart with Y-Axis
    JsonArrayConst trend = prop["daily_trend"].as<JsonArrayConst>();
    if (!trend.isNull() && trend.size() > 0) {
        int tCount = min((int)trend.size(), 7);
        int maxVal = 0;
        for (int i = 0; i < tCount; i++) {
            int v = trend[i] | 0;
            if (v > maxVal) maxVal = v;
        }
        int scaleMax = max(1, maxVal);
        int midVal = scaleMax / 2;

        if (s_trendMaxLabel) {
            char mBuf[16];
            format_compact_num(mBuf, sizeof(mBuf), scaleMax);
            lv_label_set_text(s_trendMaxLabel, mBuf);
        }
        if (s_trendMidLabel) {
            char mBuf[16];
            format_compact_num(mBuf, sizeof(mBuf), midVal);
            lv_label_set_text(s_trendMidLabel, mBuf);
        }
        if (s_trendZeroLabel) {
            lv_label_set_text(s_trendZeroLabel, "0");
        }

        int offset = 7 - tCount; // right-align if fewer than 7 days
        for (int i = 0; i < 7; i++) {
            if (!s_trendBars[i]) continue;
            int val = (i >= offset) ? (trend[i - offset] | 0) : 0;
            int barH = (val > 0) ? max(2, (val * 48) / scaleMax) : 0;
            lv_obj_set_size(s_trendBars[i], 9, barH);
            lv_obj_set_pos(s_trendBars[i], 29 + (i * 14), 222 - barH);
        }
    } else {
        if (s_trendMaxLabel) lv_label_set_text(s_trendMaxLabel, "0");
        if (s_trendMidLabel) lv_label_set_text(s_trendMidLabel, "0");
        if (s_trendZeroLabel) lv_label_set_text(s_trendZeroLabel, "0");
        for (int i = 0; i < 7; i++) {
            if (s_trendBars[i]) lv_obj_set_size(s_trendBars[i], 9, 0);
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
