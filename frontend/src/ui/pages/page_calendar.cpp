/**
 * @file page_calendar.cpp
 * @brief Source for Screen 4 (Calendar Screen) of the AuraDeck dashboard.
 */

#include "page_calendar.h"
#include "ui/thai_reshaper.h"

static lv_obj_t* s_eventRows[3] = { nullptr };

void create_page_calendar(lv_obj_t* parent) {
    // 1. Screen Title
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);
    lv_label_set_text(title, "Upcoming Agendas");

    // 2. Setup 3 fixed event items
    const int startY = 70;
    const int rowSpacing = 55;

    for (int i = 0; i < 3; i++) {
        s_eventRows[i] = lv_label_create(parent);
        lv_obj_set_style_text_font(s_eventRows[i], &lv_font_montserrat_16, 0);
        lv_obj_set_width(s_eventRows[i], 360);
        lv_label_set_long_mode(s_eventRows[i], LV_LABEL_LONG_DOT);
        lv_obj_align(s_eventRows[i], LV_ALIGN_TOP_LEFT, 20, startY + (i * rowSpacing));
        
        // Default placeholder text
        lv_label_set_text(s_eventRows[i], "- No agendas scheduled");
    }
}

void update_page_calendar(const JsonDocument& doc) {
    if (!doc.containsKey("events")) return;

    JsonArrayConst events = doc["events"].as<JsonArrayConst>();
    int idx = 0;

    for (JsonObjectConst event : events) {
        if (idx >= 3) break;

        const char* time = event["time"] | "All-day";
        const char* title = event["title"] | "Untitled Event";
        bool is_today = event["is_today"] | false;

        // Apply Thai Unicode Reshaper to prevent floating vowel overlays
        String reshapedTitle = ThaiReshaper::reshape(title);

        char buf[128];
        snprintf(buf, sizeof(buf), "[%s] %s (%s)", time, reshapedTitle.c_str(), is_today ? "Today" : "Tomorrow");

        if (s_eventRows[idx]) {
            lv_label_set_text(s_eventRows[idx], buf);
        }
        idx++;
    }

    // Clear remaining rows if checklist contains fewer than 3 events
    for (int i = idx; i < 3; i++) {
        if (s_eventRows[i]) {
            lv_label_set_text(s_eventRows[i], "");
        }
    }
}
