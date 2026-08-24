/**
 * @file page_stocks.cpp
 * @brief Screen 2 – Market Watchlist with 4-column layout.
 *
 * Layout (400px wide, content area starts at y=0 relative to page container):
 *   Col A (Symbol)    : x=20,  auto-width, LEFT-aligned
 *   Col B (Price)     : x=110, width=95,   RIGHT-aligned, ฿ for TH_STOCK, $ for others
 *   Col C (Prev Close): x=205, width=95,   RIGHT-aligned, same currency prefix as price
 *   Col D (Change%)   : x=300, width=88,   RIGHT-aligned, always signed (+/-)
 *
 * change_pct is calculated vs. chartPreviousClose (yesterday's close from Yahoo Finance).
 * Prev Close column shows that same reference price.
 * GoldTraders spot price has no historical data → prev_close=0.0 → displayed as "--".
 */

#include "page_stocks.h"
#include "ui/fonts/lv_font_prompt.h"

// ---------------------------------------------------------------------------
// Static widget storage – 4 labels per row × 6 rows
// ---------------------------------------------------------------------------
static lv_obj_t* s_symbolCols[6]   = { nullptr };
static lv_obj_t* s_prevCols[6]     = { nullptr };
static lv_obj_t* s_priceCols[6]    = { nullptr };
static lv_obj_t* s_changeCols[6]   = { nullptr };

// Thai Baht UTF-8:  ฿ = U+0E3F = 0xE0 0xB8 0xBF
static const char BAHT_UTF8[] = "\xE0\xB8\xBF";

// ---------------------------------------------------------------------------
void create_page_stocks(lv_obj_t* parent) {
    // Reset all static pointers (safety guard for re-entry)
    for (int i = 0; i < 6; i++) {
        s_symbolCols[i] = nullptr;
        s_prevCols[i]   = nullptr;
        s_priceCols[i]  = nullptr;
        s_changeCols[i] = nullptr;
    }

    // Title
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_label_set_text(title, "Market Watchlist");

    // Column headers (small font, right-aligned to match data columns)
    const struct { int x; int w; const char* txt; } headers[] = {
        { 110, 95,  "Price" },
        { 205, 95,  "Prev"  },
        { 300, 88,  "Chg%"  },
    };
    for (auto& h : headers) {
        lv_obj_t* hdr = lv_label_create(parent);
        lv_obj_set_style_text_font(hdr, &lv_font_montserrat_12, 0);
        lv_obj_set_width(hdr, h.w);
        lv_obj_align(hdr, LV_ALIGN_TOP_LEFT, h.x, 42);
        lv_obj_set_style_text_align(hdr, LV_TEXT_ALIGN_RIGHT, 0);
        lv_label_set_text(hdr, h.txt);
    }

    // Row data labels
    const int startY     = 58;
    const int rowSpacing = 35;

    for (int i = 0; i < 6; i++) {
        int y = startY + i * rowSpacing;

        // Col A – Symbol (left-aligned, auto-width)
        s_symbolCols[i] = lv_label_create(parent);
        lv_obj_set_style_text_font(s_symbolCols[i], &lv_font_prompt_16, 0);
        lv_obj_align(s_symbolCols[i], LV_ALIGN_TOP_LEFT, 20, y);
        lv_label_set_text(s_symbolCols[i], "");

        // Col B – Current Price (right-aligned, 95px at x=110)
        s_priceCols[i] = lv_label_create(parent);
        lv_obj_set_style_text_font(s_priceCols[i], &lv_font_prompt_16, 0);
        lv_obj_set_width(s_priceCols[i], 95);
        lv_obj_align(s_priceCols[i], LV_ALIGN_TOP_LEFT, 110, y);
        lv_obj_set_style_text_align(s_priceCols[i], LV_TEXT_ALIGN_RIGHT, 0);
        lv_label_set_text(s_priceCols[i], "");

        // Col C – Prev Close (right-aligned, 95px at x=205)
        s_prevCols[i] = lv_label_create(parent);
        lv_obj_set_style_text_font(s_prevCols[i], &lv_font_prompt_16, 0);
        lv_obj_set_width(s_prevCols[i], 95);
        lv_obj_align(s_prevCols[i], LV_ALIGN_TOP_LEFT, 205, y);
        lv_obj_set_style_text_align(s_prevCols[i], LV_TEXT_ALIGN_RIGHT, 0);
        lv_label_set_text(s_prevCols[i], "");

        // Col D – Change% (right-aligned, 88px at x=300)
        s_changeCols[i] = lv_label_create(parent);
        lv_obj_set_style_text_font(s_changeCols[i], &lv_font_prompt_16, 0);
        lv_obj_set_width(s_changeCols[i], 88);
        lv_obj_align(s_changeCols[i], LV_ALIGN_TOP_LEFT, 300, y);
        lv_obj_set_style_text_align(s_changeCols[i], LV_TEXT_ALIGN_RIGHT, 0);
        lv_label_set_text(s_changeCols[i], "");
    }
}

// ---------------------------------------------------------------------------
void update_page_stocks(JsonVariantConst data) {
    JsonArrayConst stocks;
    bool useChangePct = false;

    if (data.is<JsonArrayConst>()) {
        stocks       = data.as<JsonArrayConst>();
        useChangePct = true;
    } else if (data["stocks"].is<JsonArrayConst>()) {
        stocks       = data["stocks"].as<JsonArrayConst>();
        useChangePct = false;
    } else {
        return;
    }

    int idx = 0;
    for (JsonObjectConst stock : stocks) {
        if (idx >= 6) break;

        const char* symbol    = stock["symbol"]    | "";
        float price           = stock["price"]     | 0.0f;
        float prevClose       = stock["prev_close"] | 0.0f;
        float changePct       = useChangePct
            ? (stock["change_pct"]     | 0.0f)
            : (stock["change_percent"] | stock["change_pct"] | 0.0f);
        const char* assetType = stock["type"]      | "";

        // Currency: Thai Baht for SET stocks, USD ($) for all others
        const char* curr = (strcmp(assetType, "TH_STOCK") == 0) ? BAHT_UTF8 : "$";

        // Prev close: "--" when unavailable (e.g. GoldTraders spot price)
        char prevBuf[24];
        if (prevClose > 0.0f) {
            snprintf(prevBuf, sizeof(prevBuf), "%s%.2f", curr, prevClose);
        } else {
            snprintf(prevBuf, sizeof(prevBuf), "--");
        }

        // Current price: "฿48.00" or "$4141.30"
        char priceBuf[24];
        snprintf(priceBuf, sizeof(priceBuf), "%s%.2f", curr, price);

        // Change%: "+2.13%" or "-0.46%"
        char changeBuf[12];
        snprintf(changeBuf, sizeof(changeBuf), "%+.2f%%", changePct);

        if (s_symbolCols[idx]) lv_label_set_text(s_symbolCols[idx], symbol);
        if (s_prevCols[idx])   lv_label_set_text(s_prevCols[idx],   prevBuf);
        if (s_priceCols[idx])  lv_label_set_text(s_priceCols[idx],  priceBuf);
        if (s_changeCols[idx]) lv_label_set_text(s_changeCols[idx], changeBuf);

        idx++;
    }

    // Clear unused slots
    for (int i = idx; i < 6; i++) {
        if (s_symbolCols[i]) lv_label_set_text(s_symbolCols[i], "");
        if (s_prevCols[i])   lv_label_set_text(s_prevCols[i],   "");
        if (s_priceCols[i])  lv_label_set_text(s_priceCols[i],  "");
        if (s_changeCols[i]) lv_label_set_text(s_changeCols[i], "");
    }
}

// ---------------------------------------------------------------------------
void destroy_page_stocks() {
    for (int i = 0; i < 6; i++) {
        s_symbolCols[i] = nullptr;
        s_prevCols[i]   = nullptr;
        s_priceCols[i]  = nullptr;
        s_changeCols[i] = nullptr;
    }
}
