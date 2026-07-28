/**
 * @file page_analytics.cpp
 * @brief Source for Screen 6 (GCP Billing & GA4 Analytics) of the AuraDeck dashboard.
 */

#include "page_analytics.h"

static lv_obj_t* s_visitorsLabel = nullptr;
static lv_obj_t* s_billingLabel = nullptr;

void create_page_analytics(lv_obj_t* parent) {
    // Reset static pointers first (safety guard for re-entry)
    s_visitorsLabel = nullptr;
    s_billingLabel  = nullptr;

    // 1. Screen Title
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);
    lv_label_set_text(title, "Cloud Console");

    // 2. GA4 Active Visitors Section Label
    lv_obj_t* vTitle = lv_label_create(parent);
    lv_obj_set_style_text_font(vTitle, &lv_font_montserrat_12, 0);
    lv_obj_align(vTitle, LV_ALIGN_CENTER, -85, -35);
    lv_label_set_text(vTitle, "ACTIVE VISITORS");

    // Massive real-time visitors counter (48px)
    s_visitorsLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_visitorsLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(s_visitorsLabel, LV_ALIGN_CENTER, -85, 10);
    lv_label_set_text(s_visitorsLabel, "00");

    // 3. Middle vertical separator line
    lv_obj_t* line = lv_line_create(parent);
    static lv_point_t line_points[] = { {0, 60}, {0, 220} };
    lv_line_set_points(line, line_points, 2);
    lv_obj_align(line, LV_ALIGN_CENTER, 10, 0);

    // 4. GCP Cloud Billing Section Label
    lv_obj_t* bTitle = lv_label_create(parent);
    lv_obj_set_style_text_font(bTitle, &lv_font_montserrat_12, 0);
    lv_obj_align(bTitle, LV_ALIGN_CENTER, 100, -35);
    lv_label_set_text(bTitle, "GCP CLOUD BILLING");

    // Medium billing cost spend display (24px)
    s_billingLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_billingLabel, &lv_font_montserrat_24, 0);
    lv_obj_align(s_billingLabel, LV_ALIGN_CENTER, 100, 10);
    lv_label_set_text(s_billingLabel, "$0.00 MTD");

    // 5. System footnote info
    lv_obj_t* foot = lv_label_create(parent);
    lv_obj_set_style_text_font(foot, &lv_font_montserrat_12, 0);
    lv_obj_align(foot, LV_ALIGN_BOTTOM_MID, 0, -25);
    lv_label_set_text(foot, "Google Cloud Platform Connected");
}

void update_page_analytics(const JsonDocument& doc) {
    // Backend payload uses "ga4_active_users" (not "active_users")
    // and "gcp_billing" array (not flat "mtd_billing")
    // Field name reference: backend/app/services/analytics.py get_combined_analytics()

    // Update GA4 active visitors count
    int visitors = 0;
    if (doc.containsKey("ga4_active_users")) {
        visitors = doc["ga4_active_users"] | 0;
    } else if (doc.containsKey("active_users")) {
        visitors = doc["active_users"] | 0; // legacy fallback
    }
    if (s_visitorsLabel) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d", visitors);
        lv_label_set_text(s_visitorsLabel, buf);
    }

    // Update GCP billing — backend sends array, we sum all projects for a single total
    if (doc.containsKey("gcp_billing")) {
        float totalMtd = 0.0;
        JsonArrayConst billing = doc["gcp_billing"].as<JsonArrayConst>();
        for (JsonObjectConst project : billing) {
            totalMtd += project["cost_mtd"] | 0.0f;
        }
        if (s_billingLabel) {
            char buf[32];
            snprintf(buf, sizeof(buf), "$%.2f MTD", totalMtd);
            lv_label_set_text(s_billingLabel, buf);
        }
    } else if (doc.containsKey("mtd_billing")) {
        // Legacy flat field fallback
        float billing = doc["mtd_billing"] | 0.0;
        if (s_billingLabel) {
            char buf[32];
            snprintf(buf, sizeof(buf), "$%.2f MTD", billing);
            lv_label_set_text(s_billingLabel, buf);
        }
    }
}

void destroy_page_analytics() {
    // Invalidate all static widget pointers before LVGL frees the parent container.
    s_visitorsLabel = nullptr;
    s_billingLabel  = nullptr;
}
