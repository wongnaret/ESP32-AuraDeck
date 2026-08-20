/**
 * @file page_gcp.cpp
 * @brief Source for Screen 7 (GCP Multi-Project Cloud Billing & Daily Forecast) of the AuraDeck dashboard.
 * Layout:
 * - Top: Project Name, Index Indicator [1/N], and Click-to-switch hint
 * - Left Zone: MTD Spend, Month-End Forecast, Top 4 Services breakdown
 * - Right Zone: 10-Day Daily Spend Bar Chart with Min/Max scaling
 */

#include "page_gcp.h"
#include "ui/thai_reshaper.h"
#include "ui/fonts/lv_font_prompt.h"
#include <Arduino.h>

static lv_obj_t* s_projectTitleLabel = nullptr;
static lv_obj_t* s_switchHintLabel   = nullptr;
static lv_obj_t* s_mtdCostLabel      = nullptr;
static lv_obj_t* s_forecastLabel     = nullptr;
static lv_obj_t* s_serviceLabels[4]  = { nullptr };

static lv_obj_t* s_chartContainer    = nullptr;
static lv_obj_t* s_chartMaxLabel     = nullptr;
static lv_obj_t* s_barObjects[10]    = { nullptr };
static lv_obj_t* s_barDateLabels[10] = { nullptr };

static int s_activeProjectIdx = 0;
static DynamicJsonDocument* s_cachedGcpDoc = nullptr;

static void render_current_project();

void create_page_gcp(lv_obj_t* parent) {
    // Reset static pointers
    s_projectTitleLabel = nullptr;
    s_switchHintLabel   = nullptr;
    s_mtdCostLabel      = nullptr;
    s_forecastLabel     = nullptr;
    s_chartContainer    = nullptr;
    s_chartMaxLabel     = nullptr;
    for (int i = 0; i < 4; i++) {
        s_serviceLabels[i] = nullptr;
    }
    for (int i = 0; i < 10; i++) {
        s_barObjects[i] = nullptr;
        s_barDateLabels[i] = nullptr;
    }

    // ==========================================
    // Top Bar: Project Title & Switch Hint
    // ==========================================
    s_projectTitleLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_projectTitleLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_projectTitleLabel, lv_color_black(), 0);
    lv_obj_align(s_projectTitleLabel, LV_ALIGN_TOP_LEFT, 14, 6);
    lv_label_set_text(s_projectTitleLabel, "GCP [1/1]: AuraDeck Prod");

    s_switchHintLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_switchHintLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_switchHintLabel, LV_ALIGN_TOP_RIGHT, -14, 8);
    lv_label_set_text(s_switchHintLabel, "Click to Switch Project");

    // ==========================================
    // Left Zone: Spend Summary & Top Services (180x216)
    // ==========================================
    lv_obj_t* leftBox = lv_obj_create(parent);
    lv_obj_set_size(leftBox, 180, 216);
    lv_obj_align(leftBox, LV_ALIGN_TOP_LEFT, 12, 28);
    lv_obj_set_style_radius(leftBox, 4, 0);
    lv_obj_set_style_border_color(leftBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(leftBox, 1, 0);
    lv_obj_set_style_bg_color(leftBox, lv_color_white(), 0);
    lv_obj_set_style_pad_all(leftBox, 4, 0);
    lv_obj_clear_flag(leftBox, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* mtdHdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(mtdHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(mtdHdr, LV_ALIGN_TOP_LEFT, 4, 2);
    lv_label_set_text(mtdHdr, "Month-to-Date Spend");

    s_mtdCostLabel = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_mtdCostLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(s_mtdCostLabel, lv_color_black(), 0);
    lv_obj_align(s_mtdCostLabel, LV_ALIGN_TOP_LEFT, 4, 18);
    lv_label_set_text(s_mtdCostLabel, "14,250 MTD");

    s_forecastLabel = lv_label_create(leftBox);
    lv_obj_set_style_text_font(s_forecastLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_forecastLabel, LV_ALIGN_TOP_LEFT, 4, 48);
    lv_label_set_text(s_forecastLabel, "Est. Month-End: 18,500");

    lv_obj_t* lSep = lv_line_create(leftBox);
    static lv_point_t l_sep_pts[] = { {4, 68}, {168, 68} };
    lv_line_set_points(lSep, l_sep_pts, 2);
    lv_obj_set_style_line_width(lSep, 1, 0);
    lv_obj_set_style_line_color(lSep, lv_color_black(), 0);

    lv_obj_t* svcHdr = lv_label_create(leftBox);
    lv_obj_set_style_text_font(svcHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(svcHdr, LV_ALIGN_TOP_LEFT, 4, 74);
    lv_label_set_text(svcHdr, "Top Services Breakdown");

    const char* defaultSvcs[] = {
        "1. Compute Engine: 45%",
        "2. BigQuery & AI: 25%",
        "3. Cloud Run/GKE: 20%",
        "4. Cloud Storage: 10%"
    };

    for (int i = 0; i < 4; i++) {
        s_serviceLabels[i] = lv_label_create(leftBox);
        lv_obj_set_style_text_font(s_serviceLabels[i], &lv_font_montserrat_12, 0);
        lv_obj_set_width(s_serviceLabels[i], 168);
        lv_label_set_long_mode(s_serviceLabels[i], LV_LABEL_LONG_DOT);
        lv_obj_align(s_serviceLabels[i], LV_ALIGN_TOP_LEFT, 4, 94 + (i * 26));
        lv_label_set_text(s_serviceLabels[i], defaultSvcs[i]);
    }

    // ==========================================
    // Right Zone: 10-Day Daily Spend Bar Chart (188x216)
    // ==========================================
    s_chartContainer = lv_obj_create(parent);
    lv_obj_set_size(s_chartContainer, 188, 216);
    lv_obj_align(s_chartContainer, LV_ALIGN_TOP_RIGHT, -12, 28);
    lv_obj_set_style_radius(s_chartContainer, 4, 0);
    lv_obj_set_style_border_color(s_chartContainer, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_chartContainer, 1, 0);
    lv_obj_set_style_bg_color(s_chartContainer, lv_color_white(), 0);
    lv_obj_set_style_pad_all(s_chartContainer, 4, 0);
    lv_obj_clear_flag(s_chartContainer, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* chartHdr = lv_label_create(s_chartContainer);
    lv_obj_set_style_text_font(chartHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(chartHdr, LV_ALIGN_TOP_LEFT, 4, 2);
    lv_label_set_text(chartHdr, "Daily Cost (10 Days)");

    s_chartMaxLabel = lv_label_create(s_chartContainer);
    lv_obj_set_style_text_font(s_chartMaxLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_chartMaxLabel, LV_ALIGN_TOP_RIGHT, -4, 2);
    lv_label_set_text(s_chartMaxLabel, "Max: 600");

    // Baseline Line for Bar Chart
    lv_obj_t* baseLine = lv_line_create(s_chartContainer);
    static lv_point_t base_pts[] = { {4, 175}, {174, 175} };
    lv_line_set_points(baseLine, base_pts, 2);
    lv_obj_set_style_line_width(baseLine, 1, 0);
    lv_obj_set_style_line_color(baseLine, lv_color_black(), 0);

    // 10 Bars and Date Labels
    // Available chart height = 140px (from y=25 to y=175)
    // Bar width = 11px, gap = 6px -> total width = 10 * 17 = 170px
    for (int i = 0; i < 10; i++) {
        int xPos = 4 + (i * 17);

        // Bar Object (black filled rectangle)
        s_barObjects[i] = lv_obj_create(s_chartContainer);
        lv_obj_set_size(s_barObjects[i], 11, 40 + (i * 5)); // Initial placeholder height
        lv_obj_set_style_radius(s_barObjects[i], 2, 0);
        lv_obj_set_style_bg_color(s_barObjects[i], lv_color_black(), 0);
        lv_obj_set_style_border_width(s_barObjects[i], 0, 0);
        lv_obj_set_style_pad_all(s_barObjects[i], 0, 0);
        lv_obj_clear_flag(s_barObjects[i], LV_OBJ_FLAG_SCROLLABLE);
        // Align to bottom baseline
        lv_obj_align(s_barObjects[i], LV_ALIGN_TOP_LEFT, xPos, 175 - (40 + i * 5));

        // Date label below bar
        s_barDateLabels[i] = lv_label_create(s_chartContainer);
        lv_obj_set_style_text_font(s_barDateLabels[i], &lv_font_montserrat_12, 0);
        lv_obj_align(s_barDateLabels[i], LV_ALIGN_TOP_LEFT, xPos - 2, 180);
        lv_label_set_text_fmt(s_barDateLabels[i], "%d", (11 + i) % 31);
    }

    // Render initial cached project if exists
    if (s_cachedGcpDoc != nullptr) {
        render_current_project();
    }
}

static void render_current_project() {
    if (!s_cachedGcpDoc) return;
    JsonVariantConst data = s_cachedGcpDoc->as<JsonVariantConst>();

    JsonArrayConst projects = data["projects"].as<JsonArrayConst>();
    int total = data["total_projects"] | 0;
    if (projects.isNull() || projects.size() == 0) {
        return;
    }

    if (s_activeProjectIdx >= (int)projects.size()) {
        s_activeProjectIdx = 0;
    }

    JsonObjectConst proj = projects[s_activeProjectIdx];
    const char* pName = proj["project_name"] | proj["project_id"] | "GCP Project";
    const char* curr = proj["currency"] | "THB";
    float mtdCost = proj["cost_mtd"] | 0.0f;
    float forecast = proj["forecast_end_of_month"] | 0.0f;

    // 1. Update Title Header
    if (s_projectTitleLabel) {
        char tBuf[64];
        snprintf(tBuf, sizeof(tBuf), "GCP [%d/%d]: %s", s_activeProjectIdx + 1, (int)projects.size(), pName);
        lv_label_set_text(s_projectTitleLabel, tBuf);
    }

    // 2. MTD Cost & Forecast
    if (s_mtdCostLabel) {
        char cBuf[32];
        if (strcmp(curr, "USD") == 0) {
            snprintf(cBuf, sizeof(cBuf), "$%.2f MTD", mtdCost);
        } else {
            snprintf(cBuf, sizeof(cBuf), "%.0f %s MTD", mtdCost, curr);
        }
        lv_label_set_text(s_mtdCostLabel, cBuf);
    }

    if (s_forecastLabel) {
        char fBuf[32];
        if (strcmp(curr, "USD") == 0) {
            snprintf(fBuf, sizeof(fBuf), "Est. End: $%.2f", forecast);
        } else {
            snprintf(fBuf, sizeof(fBuf), "Est. End: %.0f %s", forecast, curr);
        }
        lv_label_set_text(s_forecastLabel, fBuf);
    }

    // 3. Top Services Breakdown
    JsonArrayConst svcs = proj["service_breakdown"].as<JsonArrayConst>();
    if (!svcs.isNull()) {
        for (int i = 0; i < 4; i++) {
            if (i < (int)svcs.size() && s_serviceLabels[i]) {
                JsonObjectConst sItem = svcs[i];
                const char* sName = sItem["service"] | "Service";
                int pct = sItem["pct"] | 0;
                float sCost = sItem["cost"] | 0.0f;

                char sBuf[64];
                if (strcmp(curr, "USD") == 0) {
                    snprintf(sBuf, sizeof(sBuf), "%d. %s: %d%% ($%.0f)", i + 1, sName, pct, sCost);
                } else {
                    snprintf(sBuf, sizeof(sBuf), "%d. %s: %d%%", i + 1, sName, pct);
                }
                lv_label_set_text(s_serviceLabels[i], sBuf);
            }
        }
    }

    // 4. Daily Cost Bar Chart (10 Bars)
    JsonArrayConst daily = proj["daily_costs"].as<JsonArrayConst>();
    if (!daily.isNull() && daily.size() > 0) {
        // Find max cost for scaling
        float maxCost = 1.0f;
        for (JsonObjectConst d : daily) {
            float c = d["cost"] | 0.0f;
            if (c > maxCost) maxCost = c;
        }

        if (s_chartMaxLabel) {
            char mBuf[16];
            snprintf(mBuf, sizeof(mBuf), "Max: %.0f", maxCost);
            lv_label_set_text(s_chartMaxLabel, mBuf);
        }

        int count = min((int)daily.size(), 10);
        for (int i = 0; i < count; i++) {
            JsonObjectConst d = daily[i];
            float c = d["cost"] | 0.0f;
            const char* dateStr = d["date"] | "";

            // Calculate bar height: max 125px
            int barHeight = max(4, (int)((c / maxCost) * 125.0f));
            int xPos = 4 + (i * 17);

            if (s_barObjects[i]) {
                lv_obj_set_size(s_barObjects[i], 11, barHeight);
                lv_obj_align(s_barObjects[i], LV_ALIGN_TOP_LEFT, xPos, 175 - barHeight);
            }
            if (s_barDateLabels[i]) {
                // Show day e.g. "14"
                const char* dayOnly = dateStr;
                if (strlen(dateStr) >= 2) {
                    dayOnly = dateStr; // e.g. "14/08" -> show first 2 chars
                }
                char dBuf[8] = { 0 };
                strncpy(dBuf, dateStr, 2);
                lv_label_set_text(s_barDateLabels[i], dBuf[0] ? dBuf : dateStr);
            }
        }
    }
}

void update_page_gcp(JsonVariantConst data) {
    if (!s_cachedGcpDoc) {
        s_cachedGcpDoc = new DynamicJsonDocument(8192);
    }
    s_cachedGcpDoc->set(data);
    render_current_project();
}

void cycle_gcp_project() {
    if (!s_cachedGcpDoc) return;
    JsonArrayConst projects = (*s_cachedGcpDoc)["projects"].as<JsonArrayConst>();
    if (projects.isNull() || projects.size() <= 1) return;

    s_activeProjectIdx = (s_activeProjectIdx + 1) % projects.size();
    Serial.printf("🔄 Switched to GCP Project index %d\n", s_activeProjectIdx);
    render_current_project();
}

void destroy_page_gcp() {
    s_projectTitleLabel = nullptr;
    s_switchHintLabel   = nullptr;
    s_mtdCostLabel      = nullptr;
    s_forecastLabel     = nullptr;
    s_chartContainer    = nullptr;
    s_chartMaxLabel     = nullptr;
    for (int i = 0; i < 4; i++) {
        s_serviceLabels[i] = nullptr;
    }
    for (int i = 0; i < 10; i++) {
        s_barObjects[i] = nullptr;
        s_barDateLabels[i] = nullptr;
    }
}
