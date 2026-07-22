/**
 * @file page_home.cpp
 * @brief Source for Screen 0 (Home Screen) of the AuraDeck dashboard.
 */

#include "page_home.h"

static lv_obj_t* s_clockLabel = nullptr;
static lv_obj_t* s_dateLabel = nullptr;
static lv_obj_t* s_sensorLabel = nullptr;
static lv_obj_t* s_statusLabel = nullptr;

void create_page_home(lv_obj_t* parent) {
    // 1. Digital Clock (Large Bold Typography)
    s_clockLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_clockLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(s_clockLabel, LV_ALIGN_CENTER, 0, -40);
    lv_label_set_text(s_clockLabel, "12:00");

    // 2. Calendar Date Label
    s_dateLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_dateLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_dateLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(s_dateLabel, "Wednesday, July 22");

    // 3. Horizontal Separator Bar
    lv_obj_t* line = lv_line_create(parent);
    static lv_point_t line_points[] = { {30, 0}, {370, 0} };
    lv_line_set_points(line, line_points, 2);
    lv_obj_align(line, LV_ALIGN_CENTER, 0, 25);

    // 4. SHTC3 Live Environmental Sensor Readings Card
    s_sensorLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_sensorLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_sensorLabel, LV_ALIGN_CENTER, 0, 45);
    lv_label_set_text(s_sensorLabel, "Temp: 26.5 °C  |  Humidity: 55 %");

    // 5. System Status Footer Info
    s_statusLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_statusLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_statusLabel, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_label_set_text(s_statusLabel, "System Gateway Active • AP Mode Connected");
}

void update_page_home(const JsonDocument& doc) {
    // Update local date/time from document telemetry if provided
    if (doc.containsKey("time")) {
        const char* timeStr = doc["time"];
        if (s_clockLabel) lv_label_set_text(s_clockLabel, timeStr);
    }
    if (doc.containsKey("date")) {
        const char* dateStr = doc["date"];
        if (s_dateLabel) lv_label_set_text(s_dateLabel, dateStr);
    }

    // Update environmental readouts if live sensor payload is passed
    if (doc.containsKey("temp") && doc.containsKey("humidity")) {
        float temp = doc["temp"] | 0.0;
        float hum = doc["humidity"] | 0.0;
        char buf[48];
        snprintf(buf, sizeof(buf), "Temp: %.1f °C  |  Humidity: %.1f %%", temp, hum);
        if (s_sensorLabel) lv_label_set_text(s_sensorLabel, buf);
    }
}
