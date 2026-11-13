/**
 * @file page_antigravity.cpp
 * @brief Source for Screen 1 (Antigravity Models & AI Quotas) of the AuraDeck dashboard.
 *
 * Implements high-contrast monochrome card layout for Waveshare 4.2" ST7305 RLCD (400x300,
 * 400x274 active viewport) rendering multi-model AI quotas:
 *   - Card 1 (y: 4..36):  Plan Name & Available AI Credits
 *   - Card 2 (y: 40..112): Gemini Models (5h & Weekly Quotas with Countdowns)
 *   - Card 3 (y: 116..188): Claude & GPT Models 3P (5h & Weekly Quotas)
 *   - Card 4 (y: 192..242): Dynamic Models (e.g. Gemini 4 Argon with Countdown)
 *   - Footer (y: 248..270): Action & Status Hint
 */

#include "page_antigravity.h"
#include <Arduino.h>

// Static widget pointers for Screen 1 (Antigravity)
static lv_obj_t* s_cardPlan          = nullptr;
static lv_obj_t* s_planLabel         = nullptr;
static lv_obj_t* s_creditsLabel      = nullptr;

static lv_obj_t* s_cardGemini        = nullptr;
static lv_obj_t* s_gemini5hBar       = nullptr;
static lv_obj_t* s_gemini5hPctLabel  = nullptr;
static lv_obj_t* s_gemini5hResetLabel= nullptr;
static lv_obj_t* s_geminiWkBar       = nullptr;
static lv_obj_t* s_geminiWkPctLabel  = nullptr;
static lv_obj_t* s_geminiWkResetLabel= nullptr;

static lv_obj_t* s_cardThreeP        = nullptr;
static lv_obj_t* s_threeP5hBar       = nullptr;
static lv_obj_t* s_threeP5hPctLabel  = nullptr;
static lv_obj_t* s_threeP5hResetLabel= nullptr;
static lv_obj_t* s_threePWkBar       = nullptr;
static lv_obj_t* s_threePWkPctLabel  = nullptr;
static lv_obj_t* s_threePWkResetLabel= nullptr;

static lv_obj_t* s_cardDyn           = nullptr;
static lv_obj_t* s_dynTitleLabel     = nullptr;
static lv_obj_t* s_dynPeriodLabel    = nullptr;
static lv_obj_t* s_dynBar            = nullptr;
static lv_obj_t* s_dynPctLabel       = nullptr;
static lv_obj_t* s_dynResetLabel     = nullptr;

static lv_obj_t* s_footerLabel       = nullptr;

// Helper: Creates high-contrast monochrome bounding card
static lv_obj_t* create_card(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* box = lv_obj_create(parent);
    lv_obj_set_pos(box, x, y);
    lv_obj_set_size(box, w, h);
    lv_obj_set_style_border_color(box, lv_color_black(), 0);
    lv_obj_set_style_border_width(box, 1, 0);
    lv_obj_set_style_radius(box, 4, 0);
    lv_obj_set_style_bg_color(box, lv_color_white(), 0);
    lv_obj_set_style_pad_all(box, 0, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    return box;
}

// Helper: Creates high-contrast monochrome progress bar
static lv_obj_t* create_quota_bar(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* bar = lv_bar_create(parent);
    lv_obj_set_pos(bar, x, y);
    lv_obj_set_size(bar, w, h);
    lv_obj_set_style_bg_color(bar, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_color(bar, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(bar, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_black(), LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 2, LV_PART_INDICATOR);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    return bar;
}

void create_page_antigravity(lv_obj_t* parent) {
    // Invalidate static pointers first for clean state
    destroy_page_antigravity();

    // -------------------------------------------------------------
    // Card 1: Plan Name and AI Credits Header (376 x 32 px at y=4)
    // -------------------------------------------------------------
    s_cardPlan = create_card(parent, 12, 4, 376, 32);

    s_planLabel = lv_label_create(s_cardPlan);
    lv_obj_set_style_text_font(s_planLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(s_planLabel, 8, 7);
    lv_label_set_text(s_planLabel, "Plan: Google AI Pro");

    s_creditsLabel = lv_label_create(s_cardPlan);
    lv_obj_set_style_text_font(s_creditsLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_creditsLabel, LV_ALIGN_RIGHT_MID, -8, 0);
    lv_label_set_text(s_creditsLabel, "--- AI Credits");

    // -------------------------------------------------------------
    // Card 2: Gemini Models Quota Panel (376 x 72 px at y=40)
    // -------------------------------------------------------------
    s_cardGemini = create_card(parent, 12, 40, 376, 72);

    lv_obj_t* geminiTitle = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(geminiTitle, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(geminiTitle, 8, 4);
    lv_label_set_text(geminiTitle, "Gemini Models");

    // Row 1: Five-Hour Limit
    lv_obj_t* lblG5h = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(lblG5h, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lblG5h, 8, 22);
    lv_label_set_text(lblG5h, "5h Limit");

    s_gemini5hBar = create_quota_bar(s_cardGemini, 66, 25, 144, 10);

    s_gemini5hPctLabel = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(s_gemini5hPctLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_gemini5hPctLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_gemini5hPctLabel, 216, 22);
    lv_obj_set_width(s_gemini5hPctLabel, 44);
    lv_label_set_text(s_gemini5hPctLabel, "---%");

    s_gemini5hResetLabel = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(s_gemini5hResetLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_gemini5hResetLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_gemini5hResetLabel, 264, 22);
    lv_obj_set_width(s_gemini5hResetLabel, 104);
    lv_label_set_text(s_gemini5hResetLabel, "---");

    // Row 2: Weekly Limit
    lv_obj_t* lblGWk = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(lblGWk, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lblGWk, 8, 46);
    lv_label_set_text(lblGWk, "Weekly");

    s_geminiWkBar = create_quota_bar(s_cardGemini, 66, 49, 144, 10);

    s_geminiWkPctLabel = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(s_geminiWkPctLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_geminiWkPctLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_geminiWkPctLabel, 216, 46);
    lv_obj_set_width(s_geminiWkPctLabel, 44);
    lv_label_set_text(s_geminiWkPctLabel, "---%");

    s_geminiWkResetLabel = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(s_geminiWkResetLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_geminiWkResetLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_geminiWkResetLabel, 264, 46);
    lv_obj_set_width(s_geminiWkResetLabel, 104);
    lv_label_set_text(s_geminiWkResetLabel, "---");

    // -------------------------------------------------------------
    // Card 3: Claude & GPT 3P Quota Panel (376 x 72 px at y=116)
    // -------------------------------------------------------------
    s_cardThreeP = create_card(parent, 12, 116, 376, 72);

    lv_obj_t* threePTitle = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(threePTitle, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(threePTitle, 8, 4);
    lv_label_set_text(threePTitle, "Claude & GPT Models (3P)");

    // Row 1: Five-Hour Limit
    lv_obj_t* lbl3p5h = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(lbl3p5h, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lbl3p5h, 8, 22);
    lv_label_set_text(lbl3p5h, "5h Limit");

    s_threeP5hBar = create_quota_bar(s_cardThreeP, 66, 25, 144, 10);

    s_threeP5hPctLabel = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(s_threeP5hPctLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_threeP5hPctLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_threeP5hPctLabel, 216, 22);
    lv_obj_set_width(s_threeP5hPctLabel, 44);
    lv_label_set_text(s_threeP5hPctLabel, "---%");

    s_threeP5hResetLabel = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(s_threeP5hResetLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_threeP5hResetLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_threeP5hResetLabel, 264, 22);
    lv_obj_set_width(s_threeP5hResetLabel, 104);
    lv_label_set_text(s_threeP5hResetLabel, "Full");

    // Row 2: Weekly Limit
    lv_obj_t* lbl3pWk = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(lbl3pWk, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lbl3pWk, 8, 46);
    lv_label_set_text(lbl3pWk, "Weekly");

    s_threePWkBar = create_quota_bar(s_cardThreeP, 66, 49, 144, 10);

    s_threePWkPctLabel = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(s_threePWkPctLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_threePWkPctLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_threePWkPctLabel, 216, 46);
    lv_obj_set_width(s_threePWkPctLabel, 44);
    lv_label_set_text(s_threePWkPctLabel, "---%");

    s_threePWkResetLabel = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(s_threePWkResetLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_threePWkResetLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_threePWkResetLabel, 264, 46);
    lv_obj_set_width(s_threePWkResetLabel, 104);
    lv_label_set_text(s_threePWkResetLabel, "Full");

    // -------------------------------------------------------------
    // Card 4: Dynamic Models Panel (376 x 50 px at y=192)
    // -------------------------------------------------------------
    s_cardDyn = create_card(parent, 12, 192, 376, 50);

    s_dynTitleLabel = lv_label_create(s_cardDyn);
    lv_obj_set_style_text_font(s_dynTitleLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(s_dynTitleLabel, 8, 4);
    lv_label_set_text(s_dynTitleLabel, "Gemini 4 Argon");

    s_dynPeriodLabel = lv_label_create(s_cardDyn);
    lv_obj_set_style_text_font(s_dynPeriodLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(s_dynPeriodLabel, 8, 24);
    lv_label_set_text(s_dynPeriodLabel, "Weekly");

    s_dynBar = create_quota_bar(s_cardDyn, 66, 27, 144, 10);

    s_dynPctLabel = lv_label_create(s_cardDyn);
    lv_obj_set_style_text_font(s_dynPctLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_dynPctLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_dynPctLabel, 216, 24);
    lv_obj_set_width(s_dynPctLabel, 44);
    lv_label_set_text(s_dynPctLabel, "---%");

    s_dynResetLabel = lv_label_create(s_cardDyn);
    lv_obj_set_style_text_font(s_dynResetLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_dynResetLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_dynResetLabel, 264, 24);
    lv_obj_set_width(s_dynResetLabel, 104);
    lv_label_set_text(s_dynResetLabel, "---");

    // -------------------------------------------------------------
    // Zone 5: Action & Status Footer (y=248..270)
    // -------------------------------------------------------------
    s_footerLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_footerLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_footerLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_footerLabel, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_label_set_text(s_footerLabel, "Press BOOT to Sync Quota");
}

void update_page_antigravity(JsonVariantConst data) {
    if (data.isNull()) {
        if (s_footerLabel) lv_label_set_text(s_footerLabel, "Awaiting Data...");
        return;
    }

    // 1. Check for error or unauthenticated status
    const char* status = data["status"] | "ok";
    if (status && (strcmp(status, "unauthenticated") == 0 || strcmp(status, "error") == 0)) {
        if (s_planLabel) lv_label_set_text(s_planLabel, "Status: Auth Required");
        if (s_creditsLabel) lv_label_set_text(s_creditsLabel, "0 AI Credits");
        
        if (s_gemini5hBar) lv_bar_set_value(s_gemini5hBar, 0, LV_ANIM_OFF);
        if (s_gemini5hPctLabel) lv_label_set_text(s_gemini5hPctLabel, "N/A");
        if (s_gemini5hResetLabel) lv_label_set_text(s_gemini5hResetLabel, "Offline");

        if (s_geminiWkBar) lv_bar_set_value(s_geminiWkBar, 0, LV_ANIM_OFF);
        if (s_geminiWkPctLabel) lv_label_set_text(s_geminiWkPctLabel, "N/A");
        if (s_geminiWkResetLabel) lv_label_set_text(s_geminiWkResetLabel, "Offline");

        if (s_threeP5hBar) lv_bar_set_value(s_threeP5hBar, 0, LV_ANIM_OFF);
        if (s_threeP5hPctLabel) lv_label_set_text(s_threeP5hPctLabel, "N/A");

        if (s_threePWkBar) lv_bar_set_value(s_threePWkBar, 0, LV_ANIM_OFF);
        if (s_threePWkPctLabel) lv_label_set_text(s_threePWkPctLabel, "N/A");

        if (s_dynBar) lv_bar_set_value(s_dynBar, 0, LV_ANIM_OFF);
        if (s_dynPctLabel) lv_label_set_text(s_dynPctLabel, "N/A");

        const char* err = data["error_message"] | "Check Antigravity CLI Login";
        if (s_footerLabel) lv_label_set_text(s_footerLabel, err);
        return;
    }

    // 2. Plan Name and Available Credits
    const char* plan = data["plan"] | "Google AI Pro";
    if (s_planLabel) {
        char buf[48];
        snprintf(buf, sizeof(buf), "Plan: %s", plan);
        lv_label_set_text(s_planLabel, buf);
    }

    int credits = data["available_credits"] | data["ai_credits"] | data["credits"] | -1;
    float remHours = data["credit_hours_remaining"] | -1.0f;
    if (s_creditsLabel) {
        char buf[32];
        if (credits >= 0) {
            snprintf(buf, sizeof(buf), "%d AI Credits", credits);
        } else if (remHours >= 0.0f) {
            snprintf(buf, sizeof(buf), "%.1f Hrs Left", remHours);
        } else {
            snprintf(buf, sizeof(buf), "--- AI Credits");
        }
        lv_label_set_text(s_creditsLabel, buf);
    }

    // 3. Gemini Models Quotas (5h and Weekly)
    float g5h = -1.0f;
    const char* g5hRst = nullptr;
    if (data.containsKey("gemini_models")) {
        g5h = data["gemini_models"]["five_hour_limit_percent"] | -1.0f;
        g5hRst = data["gemini_models"]["next_reset_5h"] | nullptr;
    }
    if (g5h < 0.0f && data.containsKey("limit_5h")) {
        // Fallback from legacy used percentage: remaining = 100 - used
        float used = data["limit_5h"]["percentage"] | data["limit_5h"]["used"] | -1.0f;
        if (used >= 0.0f) g5h = 100.0f - used;
    }
    if (g5h < 0.0f) {
        float usedFlat = data["percent_quota_used"] | -1.0f;
        if (usedFlat >= 0.0f) g5h = 100.0f - usedFlat;
    }
    if (g5h < 0.0f) g5h = 100.0f;

    int g5hVal = (int)round(g5h);
    if (g5hVal < 0) g5hVal = 0;
    if (g5hVal > 100) g5hVal = 100;
    if (s_gemini5hBar) lv_bar_set_value(s_gemini5hBar, g5hVal, LV_ANIM_OFF);
    if (s_gemini5hPctLabel) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", g5hVal);
        lv_label_set_text(s_gemini5hPctLabel, buf);
    }
    if (s_gemini5hResetLabel) {
        if (!g5hRst) g5hRst = data["next_reset"] | "Full";
        lv_label_set_text(s_gemini5hResetLabel, g5hRst);
    }

    float gWk = -1.0f;
    const char* gWkRst = nullptr;
    if (data.containsKey("gemini_models")) {
        gWk = data["gemini_models"]["weekly_limit_percent"] | -1.0f;
        gWkRst = data["gemini_models"]["next_reset_weekly"] | nullptr;
    }
    if (gWk < 0.0f && data.containsKey("limit_weekly")) {
        float used = data["limit_weekly"]["percentage"] | data["limit_weekly"]["used"] | -1.0f;
        if (used >= 0.0f) gWk = 100.0f - used;
    }
    if (gWk < 0.0f) gWk = 100.0f;

    int gWkVal = (int)round(gWk);
    if (gWkVal < 0) gWkVal = 0;
    if (gWkVal > 100) gWkVal = 100;
    if (s_geminiWkBar) lv_bar_set_value(s_geminiWkBar, gWkVal, LV_ANIM_OFF);
    if (s_geminiWkPctLabel) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", gWkVal);
        lv_label_set_text(s_geminiWkPctLabel, buf);
    }
    if (s_geminiWkResetLabel) {
        lv_label_set_text(s_geminiWkResetLabel, gWkRst ? gWkRst : "Full");
    }

    // 4. Claude & GPT Models (3P) Quotas
    float tp5h = 100.0f;
    const char* tp5hRst = "Full";
    float tpWk = 100.0f;
    const char* tpWkRst = "Full";
    if (data.containsKey("claude_gpt_models")) {
        tp5h = data["claude_gpt_models"]["five_hour_limit_percent"] | 100.0f;
        tp5hRst = data["claude_gpt_models"]["next_reset_5h"] | "Full";
        tpWk = data["claude_gpt_models"]["weekly_limit_percent"] | 100.0f;
        tpWkRst = data["claude_gpt_models"]["next_reset_weekly"] | "Full";
    }
    int tp5hVal = (int)round(tp5h);
    if (tp5hVal < 0) tp5hVal = 0;
    if (tp5hVal > 100) tp5hVal = 100;
    if (s_threeP5hBar) lv_bar_set_value(s_threeP5hBar, tp5hVal, LV_ANIM_OFF);
    if (s_threeP5hPctLabel) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", tp5hVal);
        lv_label_set_text(s_threeP5hPctLabel, buf);
    }
    if (s_threeP5hResetLabel) {
        lv_label_set_text(s_threeP5hResetLabel, tp5hRst);
    }

    int tpWkVal = (int)round(tpWk);
    if (tpWkVal < 0) tpWkVal = 0;
    if (tpWkVal > 100) tpWkVal = 100;
    if (s_threePWkBar) lv_bar_set_value(s_threePWkBar, tpWkVal, LV_ANIM_OFF);
    if (s_threePWkPctLabel) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", tpWkVal);
        lv_label_set_text(s_threePWkPctLabel, buf);
    }
    if (s_threePWkResetLabel) {
        lv_label_set_text(s_threePWkResetLabel, tpWkRst);
    }

    // 5. Dynamic Models Quotas (e.g. Gemini 4 Argon)
    bool hasDynamic = false;
    const char* dynName = "Gemini 4 Argon";
    float dynPct = 0.0f;
    const char* dynRst = "Full";

    // Priority 1: Check dynamic_models array
    if (data.containsKey("dynamic_models") && data["dynamic_models"].is<JsonArrayConst>()) {
        JsonArrayConst dynArr = data["dynamic_models"].as<JsonArrayConst>();
        if (dynArr.size() > 0) {
            JsonObjectConst firstDyn = dynArr[0];
            dynName = firstDyn["name"] | firstDyn["display_name"] | "Gemini 4 Argon";
            dynPct = firstDyn["remaining_percent"] | firstDyn["weekly_limit_percent"] | 0.0f;
            dynRst = firstDyn["next_reset"] | "Full";
            hasDynamic = true;
        }
    }
    
    // Priority 2: Check additional_models dictionary or array
    if (!hasDynamic && data.containsKey("additional_models")) {
        if (data["additional_models"].is<JsonObjectConst>()) {
            JsonObjectConst addObj = data["additional_models"].as<JsonObjectConst>();
            for (JsonPairConst kv : addObj) {
                JsonObjectConst m = kv.value().as<JsonObjectConst>();
                dynName = m["name"] | kv.key().c_str();
                dynPct = m["weekly_limit_percent"] | m["remaining_percent"] | 0.0f;
                dynRst = m["next_reset"] | "Full";
                hasDynamic = true;
                break;
            }
        } else if (data["additional_models"].is<JsonArrayConst>()) {
            JsonArrayConst addArr = data["additional_models"].as<JsonArrayConst>();
            if (addArr.size() > 0) {
                JsonObjectConst m = addArr[0];
                dynName = m["name"] | m["display_name"] | "Gemini 4 Argon";
                dynPct = m["weekly_limit_percent"] | m["remaining_percent"] | 0.0f;
                dynRst = m["next_reset"] | "Full";
                hasDynamic = true;
            }
        }
    }

    // Priority 3: Check raw quota bucket
    if (!hasDynamic && data.containsKey("quota") && data["quota"].containsKey("gemini-4-argon")) {
        JsonObjectConst argon = data["quota"]["gemini-4-argon"];
        dynName = "Gemini 4 Argon";
        dynPct = argon["remaining_percent"] | 0.0f;
        dynRst = argon["next_reset"] | "Full";
        hasDynamic = true;
    }

    if (hasDynamic) {
        int dynVal = (int)round(dynPct);
        if (dynVal < 0) dynVal = 0;
        if (dynVal > 100) dynVal = 100;

        if (s_dynTitleLabel) lv_label_set_text(s_dynTitleLabel, dynName);
        if (s_dynPeriodLabel) lv_label_set_text(s_dynPeriodLabel, "Weekly");
        if (s_dynBar) lv_bar_set_value(s_dynBar, dynVal, LV_ANIM_OFF);
        if (s_dynPctLabel) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d%%", dynVal);
            lv_label_set_text(s_dynPctLabel, buf);
        }
        if (s_dynResetLabel) lv_label_set_text(s_dynResetLabel, dynRst);
    } else {
        // Neutral fallback for dynamic box
        if (s_dynTitleLabel) lv_label_set_text(s_dynTitleLabel, "Additional Models");
        if (s_dynPeriodLabel) lv_label_set_text(s_dynPeriodLabel, "Weekly");
        if (s_dynBar) lv_bar_set_value(s_dynBar, 0, LV_ANIM_OFF);
        if (s_dynPctLabel) lv_label_set_text(s_dynPctLabel, "---%");
        if (s_dynResetLabel) lv_label_set_text(s_dynResetLabel, "None");
    }

    if (s_footerLabel) {
        lv_label_set_text(s_footerLabel, "Press BOOT to Sync Quota");
    }
}

void destroy_page_antigravity() {
    // Invalidate all static widget pointers before LVGL frees the parent container.
    s_cardPlan           = nullptr;
    s_planLabel          = nullptr;
    s_creditsLabel       = nullptr;

    s_cardGemini         = nullptr;
    s_gemini5hBar        = nullptr;
    s_gemini5hPctLabel   = nullptr;
    s_gemini5hResetLabel = nullptr;
    s_geminiWkBar        = nullptr;
    s_geminiWkPctLabel   = nullptr;
    s_geminiWkResetLabel = nullptr;

    s_cardThreeP         = nullptr;
    s_threeP5hBar        = nullptr;
    s_threeP5hPctLabel   = nullptr;
    s_threeP5hResetLabel = nullptr;
    s_threePWkBar        = nullptr;
    s_threePWkPctLabel   = nullptr;
    s_threePWkResetLabel = nullptr;

    s_cardDyn            = nullptr;
    s_dynTitleLabel      = nullptr;
    s_dynPeriodLabel     = nullptr;
    s_dynBar             = nullptr;
    s_dynPctLabel        = nullptr;
    s_dynResetLabel      = nullptr;

    s_footerLabel        = nullptr;
}
