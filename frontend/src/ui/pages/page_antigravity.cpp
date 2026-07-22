/**
 * @file page_antigravity.cpp
 * @brief Source for Screen 1 (Antigravity Credits Screen) of the AuraDeck dashboard.
 */

#include "page_antigravity.h"

static lv_obj_t* s_creditsLabel = nullptr;
static lv_obj_t* s_bar = nullptr;
static lv_obj_t* s_percentLabel = nullptr;
static lv_obj_t* s_resetLabel = nullptr;

void create_page_antigravity(lv_obj_t* parent) {
    // 1. Screen Title
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);
    lv_label_set_text(title, "Antigravity Credits");

    // 2. Remaining Credit Hours
    s_creditsLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_creditsLabel, &lv_font_montserrat_32, 0);
    lv_obj_align(s_creditsLabel, LV_ALIGN_CENTER, 0, -35);
    lv_label_set_text(s_creditsLabel, "0.0 Hrs Left");

    // 3. Quota Progress Bar
    s_bar = lv_bar_create(parent);
    lv_obj_set_size(s_bar, 280, 16);
    lv_obj_align(s_bar, LV_ALIGN_CENTER, 0, 15);
    lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);

    // 4. Quota Percentage Text
    s_percentLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_percentLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_percentLabel, LV_ALIGN_CENTER, 0, 45);
    lv_label_set_text(s_percentLabel, "Quota Used: 0%");

    // 5. Reset period countdown footer
    s_resetLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_resetLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_resetLabel, LV_ALIGN_BOTTOM_MID, 0, -25);
    lv_label_set_text(s_resetLabel, "API Quota Resets in 3 days");
}

void update_page_antigravity(const JsonDocument& doc) {
    // Check for Antigravity specific payload keys
    if (doc.containsKey("credit_hours_remaining")) {
        float remaining = doc["credit_hours_remaining"] | 0.0;
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f Hrs Left", remaining);
        if (s_creditsLabel) lv_label_set_text(s_creditsLabel, buf);
    }

    if (doc.containsKey("percent_quota_used")) {
        float percent = doc["percent_quota_used"] | 0.0;
        int pct_val = (int)percent;
        if (pct_val < 0) pct_val = 0;
        if (pct_val > 100) pct_val = 100;

        if (s_bar) lv_bar_set_value(s_bar, pct_val, LV_ANIM_OFF);

        char buf[32];
        snprintf(buf, sizeof(buf), "Quota Used: %d%%", pct_val);
        if (s_percentLabel) lv_label_set_text(s_percentLabel, buf);
    }
}
