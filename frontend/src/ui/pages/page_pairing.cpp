/**
 * @file page_pairing.cpp
 * @brief TV-style pairing screen implementation.
 *
 * Layout (400x240 display minus 26px header):
 *   - Title: "AuraDeck Setup"            (Montserrat 24, center)
 *   - Instruction: "Enter this PIN..."   (Montserrat 12, center)
 *   - PIN code: "6 4 5 9 0 2"           (Montserrat 48, bold, center)
 *   - URL hint: "http://{ip}:8000"       (Montserrat 12, center)
 *   - Status: "Waiting for pairing..."   (Montserrat 12, animated dots)
 */

#include "page_pairing.h"
#include <Arduino.h>

// Static widget pointers — must be reset in destroy_page_pairing()
static lv_obj_t* s_pinLabel      = nullptr;
static lv_obj_t* s_statusLabel   = nullptr;
static lv_obj_t* s_urlLabel      = nullptr;

// Dot animation state
static uint8_t   s_dotState      = 0;
static uint32_t  s_lastDotMs     = 0;

void create_page_pairing(lv_obj_t* parent, const char* pin, const char* gatewayIp) {
    // Reset static pointers first (safety guard for re-entry)
    s_pinLabel    = nullptr;
    s_statusLabel = nullptr;
    s_urlLabel    = nullptr;
    s_dotState    = 0;
    s_lastDotMs   = 0;

    // 1. Screen Title
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);
    lv_label_set_text(title, "AuraDeck Setup");

    // 2. Instruction line
    lv_obj_t* instr = lv_label_create(parent);
    lv_obj_set_style_text_font(instr, &lv_font_montserrat_12, 0);
    lv_label_set_long_mode(instr, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(instr, 360);
    lv_obj_align(instr, LV_ALIGN_TOP_MID, 0, 48);
    lv_label_set_text(instr, "Enter this PIN in the AuraDeck Web Portal:");

    // 3. Large PIN Code display
    s_pinLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_pinLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(s_pinLabel, LV_ALIGN_CENTER, 0, -10);
    lv_label_set_text(s_pinLabel, pin && pin[0] != '\0' ? pin : "------");

    // 4. URL hint label
    s_urlLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_urlLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_urlLabel, LV_ALIGN_CENTER, 0, 52);
    char urlBuf[48];
    if (gatewayIp && gatewayIp[0] != '\0') {
        snprintf(urlBuf, sizeof(urlBuf), "Go to: http://%s:8000", gatewayIp);
    } else {
        snprintf(urlBuf, sizeof(urlBuf), "Go to: http://10.42.0.1:8000");
    }
    lv_label_set_text(s_urlLabel, urlBuf);

    // 5. Waiting status label (animated dots updated by update_page_pairing_tick)
    s_statusLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_statusLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_statusLabel, LV_ALIGN_BOTTOM_MID, 0, -12);
    lv_label_set_text(s_statusLabel, "Waiting for pairing.");
}

void destroy_page_pairing() {
    // Invalidate all static widget pointers before LVGL frees the parent container.
    s_pinLabel    = nullptr;
    s_statusLabel = nullptr;
    s_urlLabel    = nullptr;
}

void update_page_pairing_tick() {
    if (!s_statusLabel) return;

    uint32_t now = millis();
    if (now - s_lastDotMs < 600) return; // animate every 600ms
    s_lastDotMs = now;

    s_dotState = (s_dotState + 1) % 4;
    switch (s_dotState) {
        case 0: lv_label_set_text(s_statusLabel, "Waiting for pairing.");    break;
        case 1: lv_label_set_text(s_statusLabel, "Waiting for pairing..");   break;
        case 2: lv_label_set_text(s_statusLabel, "Waiting for pairing..."); break;
        case 3: lv_label_set_text(s_statusLabel, "Waiting for pairing.");    break;
    }
}
