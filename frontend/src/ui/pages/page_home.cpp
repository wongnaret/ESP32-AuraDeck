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
#include "drivers/rtc_pcf85063.h"
#include <Arduino.h>

extern PCF85063RTC g_rtc;

// Top Tier: Clock and Dual-Language Dates
static lv_obj_t* s_clockLabel = nullptr;
static lv_obj_t* s_dateEnLabel = nullptr;
static lv_obj_t* s_dateThLabel = nullptr;

// Middle Tier: Indoor SHTC3 vs Outdoor Weather
static lv_obj_t* s_indoorBox = nullptr;
static lv_obj_t* s_indoorSensorLabel = nullptr;
static lv_obj_t* s_outdoorBox = nullptr;
static lv_obj_t* s_outdoorWeatherLabel = nullptr;
static lv_obj_t* s_outdoorIconImg = nullptr;

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
    s_outdoorIconImg       = nullptr;
    s_forecastBox          = nullptr;
    s_forecastTitleLabel   = nullptr;
    for (int i = 0; i < 6; i++) {
        s_hourlyTimeLabels[i] = nullptr;
        s_hourlyIconImgs[i]   = nullptr;
        s_hourlyProbLabels[i] = nullptr;
        s_hourlyTempLabels[i] = nullptr;
    }

    // Read current real-time clock from hardware RTC for instant zero-lag rendering
    DateTime dt = g_rtc.now();
    static const char* EN_DAYS[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    static const char* EN_MONTHS[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    static const char* TH_DAYS[] = {"วันอาทิตย์", "วันจันทร์", "วันอังคาร", "วันพุธ", "วันพฤหัสบดี", "วันศุกร์", "วันเสาร์"};
    static const char* TH_MONTHS[] = {"มกราคม", "กุมภาพันธ์", "มีนาคม", "เมษายน", "พฤษภาคม", "มิถุนายน", "กรกฎาคม", "สิงหาคม", "กันยายน", "ตุลาคม", "พฤศจิกายน", "ธันวาคม"};

    int wDay = dt.dayOfTheWeek() % 7;
    int mIdx = (dt.month() >= 1 && dt.month() <= 12) ? dt.month() - 1 : 0;
    int yearCE = dt.year();
    int yearBE = yearCE + 543;

    char dateEnBuf[128];
    snprintf(dateEnBuf, sizeof(dateEnBuf), "%s, %d %s %d", EN_DAYS[wDay], dt.day(), EN_MONTHS[mIdx], yearCE);

    char dateThBuf[128];
    snprintf(dateThBuf, sizeof(dateThBuf), "%sที่ %d %s %d", TH_DAYS[wDay], dt.day(), TH_MONTHS[mIdx], yearBE);

    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", dt.hour(), dt.minute());

    // ==========================================
    // Tier 1: Digital Clock + Dual-Language Date
    // ==========================================
    s_clockLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_clockLabel, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(s_clockLabel, lv_color_black(), 0);
    lv_obj_align(s_clockLabel, LV_ALIGN_TOP_LEFT, 15, 6);
    lv_label_set_text(s_clockLabel, timeBuf);

    // English Date (Larger, Montserrat 16)
    s_dateEnLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_dateEnLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_dateEnLabel, lv_color_black(), 0);
    lv_obj_set_width(s_dateEnLabel, 265);
    lv_label_set_long_mode(s_dateEnLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(s_dateEnLabel, LV_ALIGN_TOP_LEFT, 120, 6);
    lv_label_set_text(s_dateEnLabel, dateEnBuf);

    // Thai Date (Prompt 16 with ThaiReshaper)
    s_dateThLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_dateThLabel, &lv_font_prompt_16, 0);
    lv_obj_set_style_text_color(s_dateThLabel, lv_color_black(), 0);
    lv_obj_set_width(s_dateThLabel, 265);
    lv_label_set_long_mode(s_dateThLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(s_dateThLabel, LV_ALIGN_TOP_LEFT, 120, 28);
    lv_label_set_text(s_dateThLabel, ThaiReshaper::reshape(dateThBuf).c_str());

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
    lv_obj_align(indoorHdr, LV_ALIGN_TOP_LEFT, 4, 3);
    lv_label_set_text(indoorHdr, "Indoor (SHTC3)");

    s_indoorSensorLabel = lv_label_create(s_indoorBox);
    lv_obj_set_style_text_font(s_indoorSensorLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_indoorSensorLabel, lv_color_black(), 0);
    lv_obj_align(s_indoorSensorLabel, LV_ALIGN_BOTTOM_LEFT, 4, -4);
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
    lv_obj_align(outdoorHdr, LV_ALIGN_TOP_LEFT, 4, 3);
    lv_label_set_text(outdoorHdr, "Outdoor Weather");

    // Weather Icon at Top-Right (24x24 pixel monochrome icon)
    s_outdoorIconImg = lv_img_create(s_outdoorBox);
    lv_obj_align(s_outdoorIconImg, LV_ALIGN_TOP_RIGHT, -4, 2);
    lv_img_set_src(s_outdoorIconImg, get_weather_icon_dsc("CLOUD"));

    // Outdoor Weather Label (Temperature & Condition text across full bottom width)
    s_outdoorWeatherLabel = lv_label_create(s_outdoorBox);
    lv_obj_set_style_text_font(s_outdoorWeatherLabel, &lv_font_prompt_16, 0);
    lv_obj_set_style_text_color(s_outdoorWeatherLabel, lv_color_black(), 0);
    lv_obj_set_width(s_outdoorWeatherLabel, 172);
    lv_label_set_long_mode(s_outdoorWeatherLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(s_outdoorWeatherLabel, LV_ALIGN_BOTTOM_LEFT, 4, -4);
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
    int curHour = dt.hour();
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

        // Hour Label (e.g. 09:00, 10:00...)
        s_hourlyTimeLabels[i] = lv_label_create(colBox);
        lv_obj_set_style_text_font(s_hourlyTimeLabels[i], &lv_font_montserrat_12, 0);
        lv_obj_align(s_hourlyTimeLabels[i], LV_ALIGN_TOP_MID, 0, 1);
        lv_label_set_text_fmt(s_hourlyTimeLabels[i], "%02d:00", (curHour + i) % 24);

        // Weather Icon (24x24 pixel-perfect graphical icon)
        s_hourlyIconImgs[i] = lv_img_create(colBox);
        lv_obj_align(s_hourlyIconImgs[i], LV_ALIGN_TOP_MID, 0, 16);
        lv_img_set_src(s_hourlyIconImgs[i], get_weather_icon_dsc((i < 3) ? "RAIN" : "CLOUD"));

        // Rain Probability (e.g. 50%)
        s_hourlyProbLabels[i] = lv_label_create(colBox);
        lv_obj_set_style_text_font(s_hourlyProbLabels[i], &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(s_hourlyProbLabels[i], lv_color_black(), 0);
        lv_obj_align(s_hourlyProbLabels[i], LV_ALIGN_TOP_MID, 0, 44);
        lv_label_set_text_fmt(s_hourlyProbLabels[i], "%d%%", max(0, 60 - i * 10));

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
    const char* dateEn = nullptr;
    if (data.containsKey("date_en")) {
        dateEn = data["date_en"].as<const char*>();
    } else if (data.containsKey("date")) {
        dateEn = data["date"].as<const char*>();
    }
    if (dateEn && s_dateEnLabel) {
        lv_label_set_text(s_dateEnLabel, dateEn);
    }

    const char* dateTh = nullptr;
    if (data.containsKey("date_th")) {
        dateTh = data["date_th"].as<const char*>();
    }
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

    // 4. Outdoor Weather Summary & Icon
    float curOutTemp = -999.0f;
    if (data.containsKey("current_temp")) {
        curOutTemp = data["current_temp"].as<float>();
    }

    const char* curCondition = nullptr;
    if (data.containsKey("current_condition")) {
        curCondition = data["current_condition"].as<const char*>();
    } else if (data.containsKey("condition")) {
        curCondition = data["condition"].as<const char*>();
    } else if (data.containsKey("name_th")) {
        curCondition = data["name_th"].as<const char*>();
    }

    const char* curIcon = nullptr;
    if (data.containsKey("current_icon")) {
        curIcon = data["current_icon"].as<const char*>();
    } else if (data.containsKey("icon")) {
        curIcon = data["icon"].as<const char*>();
    }

    if (s_outdoorIconImg) {
        if (curIcon && strlen(curIcon) > 0) {
            lv_img_set_src(s_outdoorIconImg, get_weather_icon_dsc(curIcon));
        } else if (curCondition && strlen(curCondition) > 0) {
            lv_img_set_src(s_outdoorIconImg, get_weather_icon_dsc(curCondition));
        }
    }

    if (s_outdoorWeatherLabel && (curOutTemp > -999.0f || (curCondition != nullptr && strlen(curCondition) > 0))) {
        char outBuf[128];
        if (curOutTemp > -999.0f && curCondition != nullptr && strlen(curCondition) > 0) {
            snprintf(outBuf, sizeof(outBuf), "%.1f C | %s", curOutTemp, curCondition);
        } else if (curCondition != nullptr && strlen(curCondition) > 0) {
            snprintf(outBuf, sizeof(outBuf), "%s", curCondition);
        } else if (curOutTemp > -999.0f) {
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
            const char* hTime = slot["time"].as<const char*>();
            if (!hTime) hTime = "";

            const char* hIcon = nullptr;
            if (slot.containsKey("icon")) {
                hIcon = slot["icon"].as<const char*>();
            } else if (slot.containsKey("condition")) {
                hIcon = slot["condition"].as<const char*>();
            }
            if (!hIcon) hIcon = "CLOUD";

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
    s_outdoorIconImg       = nullptr;
    s_forecastBox          = nullptr;
    s_forecastTitleLabel   = nullptr;
    for (int i = 0; i < 6; i++) {
        s_hourlyTimeLabels[i] = nullptr;
        s_hourlyIconImgs[i]   = nullptr;
        s_hourlyProbLabels[i] = nullptr;
        s_hourlyTempLabels[i] = nullptr;
    }
}
