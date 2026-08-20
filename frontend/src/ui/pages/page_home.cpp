/**
 * @file page_home.cpp
 * @brief Source for Screen 0 (Home Screen) of the AuraDeck dashboard.
 * Features 3-tier layout:
 * 1. Big Digital Clock + Dual-language Date (English & Thai BE +543)
 * 2. Indoor Environmental Sensor (SHTC3) vs Outdoor Weather Condition Summary
 * 3. 6-Hour Horizontal Hourly Rain Forecast Strip with Graphical Weather Icons & Rain Probability (%)
 */

#include "page_home.h"
#include "ui/thai_reshaper.h"
#include "ui/weather_icons.h"
#include "ui/fonts/lv_font_prompt.h"
#include <Arduino.h>

// Top Tier: Clock and Dual-Language Dates
static lv_obj_t* s_clockLabel = nullptr;
static lv_obj_t* s_dateEnLabel = nullptr;
static lv_obj_t* s_dateThLabel = nullptr;

// Middle Tier: Indoor SHTC3 vs Outdoor Weather
static lv_obj_t* s_indoorBox = nullptr;
static lv_obj_t* s_indoorSensorLabel = nullptr;
static lv_obj_t* s_outdoorBox = nullptr;
static lv_obj_t* s_outdoorWeatherLabel = nullptr;

// Bottom Tier: 6-Hour Rain Forecast Grid
static lv_obj_t* s_forecastBox = nullptr;
static lv_obj_t* s_forecastTitleLabel = nullptr;

static lv_obj_t* s_hourlyTimeLabels[6] = { nullptr };
static lv_obj_t* s_hourlyIconImgs[6]   = { nullptr };
static lv_obj_t* s_hourlyProbLabels[6] = { nullptr };
static lv_obj_t* s_hourlyTempLabels[6] = { nullptr };

void create_page_home(lv_obj_t* parent) {
    // Reset static pointers first
    s_clockLabel           = nullptr;
    s_dateEnLabel          = nullptr;
    s_dateThLabel          = nullptr;
    s_indoorBox            = nullptr;
    s_indoorSensorLabel    = nullptr;
    s_outdoorBox           = nullptr;
    s_outdoorWeatherLabel  = nullptr;
    s_forecastBox          = nullptr;
    s_forecastTitleLabel   = nullptr;
    for (int i = 0; i < 6; i++) {
        s_hourlyTimeLabels[i] = nullptr;
        s_hourlyIconImgs[i]   = nullptr;
        s_hourlyProbLabels[i] = nullptr;
        s_hourlyTempLabels[i] = nullptr;
    }

    // ==========================================
    // Tier 1: Digital Clock + Dual-Language Date
    // ==========================================
    s_clockLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_clockLabel, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(s_clockLabel, lv_color_black(), 0);
    lv_obj_align(s_clockLabel, LV_ALIGN_TOP_LEFT, 15, 6);
    lv_label_set_text(s_clockLabel, "12:00");

    // English Date (Larger, Montserrat 16)
    s_dateEnLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_dateEnLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_dateEnLabel, lv_color_black(), 0);
    lv_obj_set_width(s_dateEnLabel, 235);
    lv_label_set_long_mode(s_dateEnLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(s_dateEnLabel, LV_ALIGN_TOP_LEFT, 145, 6);
    lv_label_set_text(s_dateEnLabel, "Thursday, 20 August 2026");

    // Thai Date (Prompt 16 with ThaiReshaper)
    s_dateThLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_dateThLabel, &lv_font_prompt_16, 0);
    lv_obj_set_style_text_color(s_dateThLabel, lv_color_black(), 0);
    lv_obj_set_width(s_dateThLabel, 235);
    lv_label_set_long_mode(s_dateThLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(s_dateThLabel, LV_ALIGN_TOP_LEFT, 145, 28);
    lv_label_set_text(s_dateThLabel, ThaiReshaper::reshape("วันพฤหัสบดีที่ 20 สิงหาคม 2569").c_str());

    // ==========================================
    // Tier 2: Indoor (SHTC3) vs Outdoor Weather
    // ==========================================
    // Left Box: Indoor Sensor
    s_indoorBox = lv_obj_create(parent);
    lv_obj_set_size(s_indoorBox, 178, 56);
    lv_obj_align(s_indoorBox, LV_ALIGN_TOP_LEFT, 15, 56);
    lv_obj_set_style_radius(s_indoorBox, 4, 0);
    lv_obj_set_style_border_color(s_indoorBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_indoorBox, 1, 0);
    lv_obj_set_style_bg_color(s_indoorBox, lv_color_white(), 0);
    lv_obj_set_style_pad_all(s_indoorBox, 3, 0);
    lv_obj_clear_flag(s_indoorBox, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* indoorHdr = lv_label_create(s_indoorBox);
    lv_obj_set_style_text_font(indoorHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(indoorHdr, LV_ALIGN_TOP_LEFT, 4, 2);
    lv_label_set_text(indoorHdr, "Indoor (SHTC3)");

    s_indoorSensorLabel = lv_label_create(s_indoorBox);
    lv_obj_set_style_text_font(s_indoorSensorLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_indoorSensorLabel, lv_color_black(), 0);
    lv_obj_align(s_indoorSensorLabel, LV_ALIGN_BOTTOM_LEFT, 4, -2);
    lv_label_set_text(s_indoorSensorLabel, "26.5 C | 55 %");

    // Right Box: Outdoor Weather Condition
    s_outdoorBox = lv_obj_create(parent);
    lv_obj_set_size(s_outdoorBox, 182, 56);
    lv_obj_align(s_outdoorBox, LV_ALIGN_TOP_RIGHT, -15, 56);
    lv_obj_set_style_radius(s_outdoorBox, 4, 0);
    lv_obj_set_style_border_color(s_outdoorBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_outdoorBox, 1, 0);
    lv_obj_set_style_bg_color(s_outdoorBox, lv_color_white(), 0);
    lv_obj_set_style_pad_all(s_outdoorBox, 3, 0);
    lv_obj_clear_flag(s_outdoorBox, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* outdoorHdr = lv_label_create(s_outdoorBox);
    lv_obj_set_style_text_font(outdoorHdr, &lv_font_montserrat_12, 0);
    lv_obj_align(outdoorHdr, LV_ALIGN_TOP_LEFT, 4, 2);
    lv_label_set_text(outdoorHdr, "Outdoor Weather");

    s_outdoorWeatherLabel = lv_label_create(s_outdoorBox);
    lv_obj_set_style_text_font(s_outdoorWeatherLabel, &lv_font_prompt_16, 0);
    lv_obj_set_style_text_color(s_outdoorWeatherLabel, lv_color_black(), 0);
    lv_obj_set_width(s_outdoorWeatherLabel, 172);
    lv_label_set_long_mode(s_outdoorWeatherLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(s_outdoorWeatherLabel, LV_ALIGN_BOTTOM_LEFT, 4, -2);
    lv_label_set_text(s_outdoorWeatherLabel, ThaiReshaper::reshape("28.5 C | มีเมฆมาก").c_str());

    // ==========================================
    // Tier 3: 6-Hour Rain Forecast Strip
    // ==========================================
    s_forecastBox = lv_obj_create(parent);
    lv_obj_set_size(s_forecastBox, 370, 144);
    lv_obj_align(s_forecastBox, LV_ALIGN_TOP_MID, 0, 118);
    lv_obj_set_style_radius(s_forecastBox, 4, 0);
    lv_obj_set_style_border_color(s_forecastBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_forecastBox, 1, 0);
    lv_obj_set_style_bg_color(s_forecastBox, lv_color_white(), 0);
    lv_obj_set_style_pad_all(s_forecastBox, 4, 0);
    lv_obj_clear_flag(s_forecastBox, LV_OBJ_FLAG_SCROLLABLE);

    s_forecastTitleLabel = lv_label_create(s_forecastBox);
    lv_obj_set_style_text_font(s_forecastTitleLabel, &lv_font_prompt_12, 0);
    lv_obj_align(s_forecastTitleLabel, LV_ALIGN_TOP_LEFT, 4, 2);
    lv_label_set_text(s_forecastTitleLabel, ThaiReshaper::reshape("Hourly Rain Forecast (พยากรณ์ฝน 6 ชม.)").c_str());

    // 6-Column Grid
    for (int i = 0; i < 6; i++) {
        lv_obj_t* colBox = lv_obj_create(s_forecastBox);
        lv_obj_set_size(colBox, 56, 110);
        lv_obj_align(colBox, LV_ALIGN_TOP_LEFT, 4 + (i * 59), 20);
        lv_obj_set_style_radius(colBox, 3, 0);
        lv_obj_set_style_border_color(colBox, lv_color_black(), 0);
        lv_obj_set_style_border_width(colBox, 1, 0);
        lv_obj_set_style_bg_color(colBox, lv_color_white(), 0);
        lv_obj_set_style_pad_all(colBox, 2, 0);
        lv_obj_clear_flag(colBox, LV_OBJ_FLAG_SCROLLABLE);

        // Hour Label (e.g. 18:00)
        s_hourlyTimeLabels[i] = lv_label_create(colBox);
        lv_obj_set_style_text_font(s_hourlyTimeLabels[i], &lv_font_montserrat_12, 0);
        lv_obj_align(s_hourlyTimeLabels[i], LV_ALIGN_TOP_MID, 0, 1);
        lv_label_set_text_fmt(s_hourlyTimeLabels[i], "%02d:00", (18 + i) % 24);

        // Weather Icon (24x24 pixel-perfect graphical icon)
        s_hourlyIconImgs[i] = lv_img_create(colBox);
        lv_obj_align(s_hourlyIconImgs[i], LV_ALIGN_TOP_MID, 0, 16);
        lv_img_set_src(s_hourlyIconImgs[i], get_weather_icon_dsc((i < 3) ? "RAIN" : "CLOUD"));

        // Rain Probability (e.g. 80%)
        s_hourlyProbLabels[i] = lv_label_create(colBox);
        lv_obj_set_style_text_font(s_hourlyProbLabels[i], &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(s_hourlyProbLabels[i], lv_color_black(), 0);
        lv_obj_align(s_hourlyProbLabels[i], LV_ALIGN_TOP_MID, 0, 44);
        lv_label_set_text_fmt(s_hourlyProbLabels[i], "%d%%", max(0, 80 - i * 15));

        // Temperature (e.g. 28 C)
        s_hourlyTempLabels[i] = lv_label_create(colBox);
        lv_obj_set_style_text_font(s_hourlyTempLabels[i], &lv_font_montserrat_12, 0);
        lv_obj_align(s_hourlyTempLabels[i], LV_ALIGN_BOTTOM_MID, 0, -1);
        lv_label_set_text_fmt(s_hourlyTempLabels[i], "%d C", 28 - (i / 2));
    }
}

void update_page_home(JsonVariantConst data) {
    // 1. Digital Clock
    const char* timeStr = data["time"] | nullptr;
    if (timeStr && s_clockLabel) {
        lv_label_set_text(s_clockLabel, timeStr);
    }

    // 2. Dual-Language Dates
    const char* dateEn = data["date_en"] | data["date"] | nullptr;
    if (dateEn && s_dateEnLabel) {
        lv_label_set_text(s_dateEnLabel, dateEn);
    }

    const char* dateTh = data["date_th"] | nullptr;
    if (dateTh && s_dateThLabel) {
        String reshaped = ThaiReshaper::reshape(dateTh);
        lv_label_set_text(s_dateThLabel, reshaped.c_str());
    }

    // 3. SHTC3 Indoor Environmental Sensor Readings
    float temp = data["temp"] | -999.0f;
    float hum  = data["humidity"] | -999.0f;
    if (temp > -999.0f && hum > -999.0f) {
        char buf[48];
        snprintf(buf, sizeof(buf), "%.1f C | %.0f %%", temp, hum);
        if (s_indoorSensorLabel) {
            lv_label_set_text(s_indoorSensorLabel, buf);
        }
    }

    // 4. Outdoor Weather Summary
    float curOutTemp = data["current_temp"] | -999.0f;
    const char* curCondition = data["current_condition"] | nullptr;
    if (s_outdoorWeatherLabel && (curOutTemp > -999.0f || curCondition != nullptr)) {
        char outBuf[64];
        if (curOutTemp > -999.0f && curCondition != nullptr) {
            snprintf(outBuf, sizeof(outBuf), "%.1f C | %s", curOutTemp, curCondition);
        } else if (curCondition != nullptr) {
            snprintf(outBuf, sizeof(outBuf), "%s", curCondition);
        } else {
            snprintf(outBuf, sizeof(outBuf), "%.1f C", curOutTemp);
        }
        String reshaped = ThaiReshaper::reshape(outBuf);
        lv_label_set_text(s_outdoorWeatherLabel, reshaped.c_str());
    }

    // 5. 6-Hour Hourly Rain Forecast Grid
    JsonArrayConst hourly = data["hourly"].as<JsonArrayConst>();
    if (!hourly.isNull() && hourly.size() > 0) {
        int count = min((int)hourly.size(), 6);
        for (int i = 0; i < count; i++) {
            JsonObjectConst slot = hourly[i];
            const char* hTime = slot["time"] | "";
            const char* hIcon = slot["icon"] | slot["condition"] | "CLOUD";
            int hProb = slot["rain_prob"] | 0;
            float hTemp = slot["temp"] | -999.0f;

            if (s_hourlyTimeLabels[i]) {
                lv_label_set_text(s_hourlyTimeLabels[i], hTime);
            }
            if (s_hourlyIconImgs[i]) {
                lv_img_set_src(s_hourlyIconImgs[i], get_weather_icon_dsc(hIcon));
            }
            if (s_hourlyProbLabels[i]) {
                char pBuf[16];
                snprintf(pBuf, sizeof(pBuf), "%d%%", hProb);
                lv_label_set_text(s_hourlyProbLabels[i], pBuf);
            }
            if (s_hourlyTempLabels[i] && hTemp > -999.0f) {
                char tBuf[16];
                snprintf(tBuf, sizeof(tBuf), "%.0f C", hTemp);
                lv_label_set_text(s_hourlyTempLabels[i], tBuf);
            }
        }
    }
}

void destroy_page_home() {
    s_clockLabel           = nullptr;
    s_dateEnLabel          = nullptr;
    s_dateThLabel          = nullptr;
    s_indoorBox            = nullptr;
    s_indoorSensorLabel    = nullptr;
    s_outdoorBox           = nullptr;
    s_outdoorWeatherLabel  = nullptr;
    s_forecastBox          = nullptr;
    s_forecastTitleLabel   = nullptr;
    for (int i = 0; i < 6; i++) {
        s_hourlyTimeLabels[i] = nullptr;
        s_hourlyIconImgs[i]   = nullptr;
        s_hourlyProbLabels[i] = nullptr;
        s_hourlyTempLabels[i] = nullptr;
    }
}
