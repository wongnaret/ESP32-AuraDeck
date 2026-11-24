/**
 * @file page_antigravity.cpp
 * @brief Source for Screen 1 (Antigravity Models & AI Quotas) of the AuraDeck dashboard.
 *
 * Implements high-contrast monochrome 3-card layout for Waveshare 4.2" ST7305 RLCD (400x300,
 * 400x274 active viewport) aligned with Antigravity's unified 2-group quota model:
 *   - Card 1 (y: 6..42):    Plan Name & Available AI Credits
 *   - Card 2 (y: 50..142):  Gemini Models (All Gemini 4 & 3.8: 5h & Weekly Quotas + Countdowns)
 *   - Card 3 (y: 150..242): Claude & GPT Models 3P (5h & Weekly Quotas + Countdowns)
 *   - Footer (y: 252..268): Live Snapshot Age & Sync Hint
 */

#include "page_antigravity.h"
#include <Arduino.h>
#include <time.h>

// Live countdown snapshot state
struct AntigravityLiveState {
    bool hasData = false;
    bool isError = false;
    uint32_t rxMillis = 0;
    int64_t snapshotTs = 0;
    int64_t serverTime = 0;

    float g5hPct = 100.0f;
    float gWkPct = 100.0f;
    int32_t g5hSec = 0;
    int32_t gWkSec = 0;
    char g5hRst[24] = "Full";
    char gWkRst[24] = "Full";

    float tp5hPct = 100.0f;
    float tpWkPct = 100.0f;
    int32_t tp5hSec = 0;
    int32_t tpWkSec = 0;
    char tp5hRst[24] = "Full";
    char tpWkRst[24] = "Full";
};

static AntigravityLiveState s_live;
static lv_timer_t* s_countdownTimer = nullptr;
static void antigravity_refresh_countdowns();

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
    // Card 1: Plan Name and AI Credits Header (376 x 36 px at y=6)
    // -------------------------------------------------------------
    s_cardPlan = create_card(parent, 12, 6, 376, 36);

    s_planLabel = lv_label_create(s_cardPlan);
    lv_obj_set_style_text_font(s_planLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_planLabel, LV_ALIGN_LEFT_MID, 10, 0);
    lv_label_set_text(s_planLabel, "Plan: Google AI Pro");

    s_creditsLabel = lv_label_create(s_cardPlan);
    lv_obj_set_style_text_font(s_creditsLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(s_creditsLabel, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_label_set_text(s_creditsLabel, "--- AI Credits");

    // -------------------------------------------------------------
    // Card 2: Gemini Models Quota Panel (376 x 92 px at y=50)
    // -------------------------------------------------------------
    s_cardGemini = create_card(parent, 12, 50, 376, 92);

    lv_obj_t* geminiTitle = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(geminiTitle, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(geminiTitle, 10, 8);
    lv_label_set_text(geminiTitle, "Gemini Models");

    lv_obj_t* geminiSub = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(geminiSub, &lv_font_montserrat_12, 0);
    lv_obj_align(geminiSub, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_label_set_text(geminiSub, "All Gemini 4 & 3.8");

    // Row 1: Five-Hour Limit
    lv_obj_t* lblG5h = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(lblG5h, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lblG5h, 10, 38);
    lv_label_set_text(lblG5h, "5h Limit");

    s_gemini5hBar = create_quota_bar(s_cardGemini, 68, 38, 140, 14);

    s_gemini5hPctLabel = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(s_gemini5hPctLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(s_gemini5hPctLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_gemini5hPctLabel, 212, 35);
    lv_obj_set_width(s_gemini5hPctLabel, 50);
    lv_label_set_text(s_gemini5hPctLabel, "---%");

    s_gemini5hResetLabel = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(s_gemini5hResetLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_gemini5hResetLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_gemini5hResetLabel, 264, 38);
    lv_obj_set_width(s_gemini5hResetLabel, 104);
    lv_label_set_text(s_gemini5hResetLabel, "---");

    // Row 2: Weekly Limit
    lv_obj_t* lblGWk = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(lblGWk, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lblGWk, 10, 66);
    lv_label_set_text(lblGWk, "Weekly");

    s_geminiWkBar = create_quota_bar(s_cardGemini, 68, 66, 140, 14);

    s_geminiWkPctLabel = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(s_geminiWkPctLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(s_geminiWkPctLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_geminiWkPctLabel, 212, 63);
    lv_obj_set_width(s_geminiWkPctLabel, 50);
    lv_label_set_text(s_geminiWkPctLabel, "---%");

    s_geminiWkResetLabel = lv_label_create(s_cardGemini);
    lv_obj_set_style_text_font(s_geminiWkResetLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_geminiWkResetLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_geminiWkResetLabel, 264, 66);
    lv_obj_set_width(s_geminiWkResetLabel, 104);
    lv_label_set_text(s_geminiWkResetLabel, "---");

    // -------------------------------------------------------------
    // Card 3: Claude & GPT 3P Quota Panel (376 x 92 px at y=150)
    // -------------------------------------------------------------
    s_cardThreeP = create_card(parent, 12, 150, 376, 92);

    lv_obj_t* threePTitle = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(threePTitle, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(threePTitle, 10, 8);
    lv_label_set_text(threePTitle, "Claude & GPT Models");

    lv_obj_t* threePSub = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(threePSub, &lv_font_montserrat_12, 0);
    lv_obj_align(threePSub, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_label_set_text(threePSub, "3P Models");

    // Row 1: Five-Hour Limit
    lv_obj_t* lbl3p5h = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(lbl3p5h, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lbl3p5h, 10, 38);
    lv_label_set_text(lbl3p5h, "5h Limit");

    s_threeP5hBar = create_quota_bar(s_cardThreeP, 68, 38, 140, 14);

    s_threeP5hPctLabel = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(s_threeP5hPctLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(s_threeP5hPctLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_threeP5hPctLabel, 212, 35);
    lv_obj_set_width(s_threeP5hPctLabel, 50);
    lv_label_set_text(s_threeP5hPctLabel, "---%");

    s_threeP5hResetLabel = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(s_threeP5hResetLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_threeP5hResetLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_threeP5hResetLabel, 264, 38);
    lv_obj_set_width(s_threeP5hResetLabel, 104);
    lv_label_set_text(s_threeP5hResetLabel, "Full");

    // Row 2: Weekly Limit
    lv_obj_t* lbl3pWk = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(lbl3pWk, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lbl3pWk, 10, 66);
    lv_label_set_text(lbl3pWk, "Weekly");

    s_threePWkBar = create_quota_bar(s_cardThreeP, 68, 66, 140, 14);

    s_threePWkPctLabel = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(s_threePWkPctLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(s_threePWkPctLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_threePWkPctLabel, 212, 63);
    lv_obj_set_width(s_threePWkPctLabel, 50);
    lv_label_set_text(s_threePWkPctLabel, "---%");

    s_threePWkResetLabel = lv_label_create(s_cardThreeP);
    lv_obj_set_style_text_font(s_threePWkResetLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_threePWkResetLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_threePWkResetLabel, 264, 66);
    lv_obj_set_width(s_threePWkResetLabel, 104);
    lv_label_set_text(s_threePWkResetLabel, "Full");

    // -------------------------------------------------------------
    // Footer: Action & Live Snapshot Age Hint (y=252..268)
    // -------------------------------------------------------------
    s_footerLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_footerLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(s_footerLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_footerLabel, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_label_set_text(s_footerLabel, "Press BOOT to Sync Quota");

    if (s_countdownTimer) {
        lv_timer_del(s_countdownTimer);
        s_countdownTimer = nullptr;
    }
    s_countdownTimer = lv_timer_create([](lv_timer_t* timer) {
        (void)timer;
        antigravity_refresh_countdowns();
    }, 60000, nullptr);
}

static void format_countdown_str(int32_t seconds, char* buf, size_t len) {
    if (seconds <= 0) {
        snprintf(buf, len, "Full");
        return;
    }
    if (seconds < 60) {
        snprintf(buf, len, "<1m");
        return;
    }
    if (seconds < 3600) {
        snprintf(buf, len, "%dm", (int)(seconds / 60));
        return;
    }
    if (seconds < 86400) {
        int h = (int)(seconds / 3600);
        int m = (int)((seconds % 3600) / 60);
        snprintf(buf, len, "%02dh %02dm", h, m);
        return;
    }
    int days = (int)(seconds / 86400);
    int remHours = (int)((seconds % 86400) / 3600);
    if (days == 1 && remHours == 1) {
        snprintf(buf, len, "1 day, 1 hour");
    } else if (days == 1) {
        if (remHours > 0) snprintf(buf, len, "1 day, %d hours", remHours);
        else snprintf(buf, len, "1 day");
    } else if (remHours > 0) {
        snprintf(buf, len, "%d days, %d hours", days, remHours);
    } else {
        snprintf(buf, len, "%d days", days);
    }
}

static void apply_pair_countdown(
    float init5hPct, int32_t init5hSec, const char* init5hRst,
    float initWkPct, int32_t initWkSec, const char* initWkRst,
    uint32_t elapsed,
    lv_obj_t* bar5h, lv_obj_t* pct5hLbl, lv_obj_t* rst5hLbl,
    lv_obj_t* barWk, lv_obj_t* pctWkLbl, lv_obj_t* rstWkLbl
) {
    float curWkPct = initWkPct;
    char wkRstBuf[24];
    if (initWkSec > 0) {
        int32_t remWk = initWkSec - (int32_t)elapsed;
        if (remWk <= 0) {
            curWkPct = 100.0f;
            snprintf(wkRstBuf, sizeof(wkRstBuf), "Full");
        } else {
            format_countdown_str(remWk, wkRstBuf, sizeof(wkRstBuf));
        }
    } else {
        snprintf(wkRstBuf, sizeof(wkRstBuf), "%s", initWkRst ? initWkRst : "Full");
    }

    float cur5hPct = init5hPct;
    char fiveHRstBuf[24];
    if (init5hSec > 0) {
        int32_t rem5h = init5hSec - (int32_t)elapsed;
        if (rem5h <= 0) {
            if (curWkPct > 0.0f) {
                cur5hPct = 100.0f;
                snprintf(fiveHRstBuf, sizeof(fiveHRstBuf), "Full");
            } else {
                snprintf(fiveHRstBuf, sizeof(fiveHRstBuf), "0m");
            }
        } else {
            format_countdown_str(rem5h, fiveHRstBuf, sizeof(fiveHRstBuf));
        }
    } else {
        snprintf(fiveHRstBuf, sizeof(fiveHRstBuf), "%s", init5hRst ? init5hRst : "Full");
    }

    int val5h = (int)round(cur5hPct);
    if (val5h < 0) val5h = 0;
    if (val5h > 100) val5h = 100;
    if (bar5h) lv_bar_set_value(bar5h, val5h, LV_ANIM_OFF);
    if (pct5hLbl) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", val5h);
        lv_label_set_text(pct5hLbl, buf);
    }
    if (rst5hLbl) lv_label_set_text(rst5hLbl, fiveHRstBuf);

    int valWk = (int)round(curWkPct);
    if (valWk < 0) valWk = 0;
    if (valWk > 100) valWk = 100;
    if (barWk) lv_bar_set_value(barWk, valWk, LV_ANIM_OFF);
    if (pctWkLbl) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", valWk);
        lv_label_set_text(pctWkLbl, buf);
    }
    if (rstWkLbl) lv_label_set_text(rstWkLbl, wkRstBuf);
}

static void antigravity_refresh_countdowns() {
    if (!s_live.hasData || s_live.isError) return;

    uint32_t elapsed = (uint32_t)((millis() - s_live.rxMillis) / 1000UL);

    apply_pair_countdown(
        s_live.g5hPct, s_live.g5hSec, s_live.g5hRst,
        s_live.gWkPct, s_live.gWkSec, s_live.gWkRst,
        elapsed,
        s_gemini5hBar, s_gemini5hPctLabel, s_gemini5hResetLabel,
        s_geminiWkBar, s_geminiWkPctLabel, s_geminiWkResetLabel
    );

    apply_pair_countdown(
        s_live.tp5hPct, s_live.tp5hSec, s_live.tp5hRst,
        s_live.tpWkPct, s_live.tpWkSec, s_live.tpWkRst,
        elapsed,
        s_threeP5hBar, s_threeP5hPctLabel, s_threeP5hResetLabel,
        s_threePWkBar, s_threePWkPctLabel, s_threePWkResetLabel
    );

    if (s_footerLabel) {
        if (s_live.snapshotTs > 0) {
            int64_t ageSec = 0;
            if (s_live.serverTime >= s_live.snapshotTs) {
                ageSec = (s_live.serverTime - s_live.snapshotTs) + (int64_t)elapsed;
            } else {
                time_t nowEpoch = time(nullptr);
                if (nowEpoch > 1700000000 && nowEpoch >= s_live.snapshotTs) {
                    ageSec = (int64_t)nowEpoch - s_live.snapshotTs;
                } else {
                    ageSec = (int64_t)elapsed;
                }
            }
            if (ageSec < 0) ageSec = 0;

            char footerBuf[48];
            if (ageSec < 60) {
                snprintf(footerBuf, sizeof(footerBuf), "Updated just now | BOOT: Sync");
            } else if (ageSec < 3600) {
                snprintf(footerBuf, sizeof(footerBuf), "Updated %ldm ago | BOOT: Sync", (long)(ageSec / 60));
            } else if (ageSec < 86400) {
                snprintf(footerBuf, sizeof(footerBuf), "Updated %ldh ago | BOOT: Sync", (long)(ageSec / 3600));
            } else {
                snprintf(footerBuf, sizeof(footerBuf), "Updated %ldd ago | BOOT: Sync", (long)(ageSec / 86400));
            }
            lv_label_set_text(s_footerLabel, footerBuf);
        } else {
            lv_label_set_text(s_footerLabel, "Press BOOT to Sync Quota");
        }
    }
}

void update_page_antigravity(JsonVariantConst data) {
    if (data.isNull()) {
        if (s_footerLabel) lv_label_set_text(s_footerLabel, "Awaiting Data...");
        return;
    }

    // 1. Check for error or unauthenticated status
    const char* status = data["status"] | "ok";
    if (status && (strcmp(status, "unauthenticated") == 0 || strcmp(status, "error") == 0)) {
        s_live.hasData = true;
        s_live.isError = true;
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
        if (s_threeP5hResetLabel) lv_label_set_text(s_threeP5hResetLabel, "Offline");

        if (s_threePWkBar) lv_bar_set_value(s_threePWkBar, 0, LV_ANIM_OFF);
        if (s_threePWkPctLabel) lv_label_set_text(s_threePWkPctLabel, "N/A");
        if (s_threePWkResetLabel) lv_label_set_text(s_threePWkResetLabel, "Offline");

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

    // 3. Gemini Models Quotas (5h and Weekly — unified group including Argon, Neon, Flash)
    s_live.hasData = true;
    s_live.isError = false;
    s_live.rxMillis = millis();
    s_live.snapshotTs = data["timestamp"] | (int64_t)0;
    s_live.serverTime = data["server_time"] | (int64_t)0;

    float g5h = -1.0f;
    const char* g5hRst = nullptr;
    int32_t g5hSec = 0;
    if (data.containsKey("gemini_models")) {
        g5h = data["gemini_models"]["five_hour_limit_percent"] | -1.0f;
        g5hRst = data["gemini_models"]["next_reset_5h"] | nullptr;
        g5hSec = data["gemini_models"]["reset_in_seconds_5h"] | 0;
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
    if (!g5hRst) g5hRst = data["next_reset"] | "Full";

    float gWk = -1.0f;
    const char* gWkRst = nullptr;
    int32_t gWkSec = 0;
    if (data.containsKey("gemini_models")) {
        gWk = data["gemini_models"]["weekly_limit_percent"] | -1.0f;
        gWkRst = data["gemini_models"]["next_reset_weekly"] | nullptr;
        gWkSec = data["gemini_models"]["reset_in_seconds_weekly"] | 0;
    }
    if (gWk < 0.0f && data.containsKey("limit_weekly")) {
        float used = data["limit_weekly"]["percentage"] | data["limit_weekly"]["used"] | -1.0f;
        if (used >= 0.0f) gWk = 100.0f - used;
    }
    if (gWk < 0.0f) gWk = 100.0f;

    s_live.g5hPct = g5h;
    s_live.g5hSec = g5hSec;
    snprintf(s_live.g5hRst, sizeof(s_live.g5hRst), "%s", g5hRst ? g5hRst : "Full");
    s_live.gWkPct = gWk;
    s_live.gWkSec = gWkSec;
    snprintf(s_live.gWkRst, sizeof(s_live.gWkRst), "%s", gWkRst ? gWkRst : "Full");

    // 4. Claude & GPT Models (3P) Quotas
    float tp5h = 100.0f;
    const char* tp5hRst = "Full";
    int32_t tp5hSec = 0;
    float tpWk = 100.0f;
    const char* tpWkRst = "Full";
    int32_t tpWkSec = 0;
    if (data.containsKey("claude_gpt_models")) {
        tp5h = data["claude_gpt_models"]["five_hour_limit_percent"] | 100.0f;
        tp5hRst = data["claude_gpt_models"]["next_reset_5h"] | "Full";
        tp5hSec = data["claude_gpt_models"]["reset_in_seconds_5h"] | 0;
        tpWk = data["claude_gpt_models"]["weekly_limit_percent"] | 100.0f;
        tpWkRst = data["claude_gpt_models"]["next_reset_weekly"] | "Full";
        tpWkSec = data["claude_gpt_models"]["reset_in_seconds_weekly"] | 0;
    }
    if (tp5hSec == 0 && data.containsKey("quota") && data["quota"].containsKey("3p-5h")) {
        tp5hSec = data["quota"]["3p-5h"]["reset_in_seconds"] | 0;
    }
    if (tpWkSec == 0 && data.containsKey("quota") && data["quota"].containsKey("3p-weekly")) {
        tpWkSec = data["quota"]["3p-weekly"]["reset_in_seconds"] | 0;
    }

    s_live.tp5hPct = tp5h;
    s_live.tp5hSec = tp5hSec;
    snprintf(s_live.tp5hRst, sizeof(s_live.tp5hRst), "%s", tp5hRst ? tp5hRst : "Full");
    s_live.tpWkPct = tpWk;
    s_live.tpWkSec = tpWkSec;
    snprintf(s_live.tpWkRst, sizeof(s_live.tpWkRst), "%s", tpWkRst ? tpWkRst : "Full");

    antigravity_refresh_countdowns();
}

void destroy_page_antigravity() {
    if (s_countdownTimer) {
        lv_timer_del(s_countdownTimer);
        s_countdownTimer = nullptr;
    }
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

    s_footerLabel        = nullptr;
}
