/**
 * @file page_stocks.cpp
 * @brief Source for Screen 2 (Stocks watches Screen) of the AuraDeck dashboard.
 */

#include "page_stocks.h"

// Setup 4 hardcoded row labels to prevent dynamic heap fragmentation (Rule 5)
static lv_obj_t* s_stockRows[4] = { nullptr };

void create_page_stocks(lv_obj_t* parent) {
    // Reset static row pointers first (safety guard for re-entry)
    for (int i = 0; i < 4; i++) s_stockRows[i] = nullptr;

    // 1. Title Label
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);
    lv_label_set_text(title, "Market Watchlist");

    // 2. Define vertical positions for stock slots
    const int startY = 70;
    const int rowSpacing = 45;

    for (int i = 0; i < 4; i++) {
        s_stockRows[i] = lv_label_create(parent);
        lv_obj_set_style_text_font(s_stockRows[i], &lv_font_montserrat_16, 0);
        lv_obj_align(s_stockRows[i], LV_ALIGN_TOP_LEFT, 20, startY + (i * rowSpacing));
        
        // Default placeholder text
        if (i == 0) lv_label_set_text(s_stockRows[i], "SET50       --.--   (--.-%)");
        if (i == 1) lv_label_set_text(s_stockRows[i], "CPALL.BK    --.--   (--.-%)");
        if (i == 2) lv_label_set_text(s_stockRows[i], "GOLD        --.--   (--.-%)");
        if (i == 3) lv_label_set_text(s_stockRows[i], "BTC-THB     --.--   (--.-%)");
    }
}

void update_page_stocks(const JsonDocument& doc) {
    if (!doc.containsKey("stocks")) return;

    JsonArrayConst stocks = doc["stocks"].as<JsonArrayConst>();
    int idx = 0;

    for (JsonObjectConst stock : stocks) {
        if (idx >= 4) break;

        const char* symbol = stock["symbol"] | "";
        float price = stock["price"] | 0.0;
        float changePct = stock["change_percent"] | 0.0;

        char buf[64];
        const char* trendSign = (changePct >= 0) ? "▲ +" : "▼ ";
        const char* currency = (strcmp(symbol, "GOLD") == 0) ? "฿" : (strcmp(symbol, "SET50") == 0) ? "pts" : "$";

        // Align SET50 and GOLD prices appropriately
        if (strcmp(symbol, "SET50") == 0) {
            snprintf(buf, sizeof(buf), "%-10s  %.1f pts  %s%.2f%%", symbol, price, trendSign, changePct);
        } else if (strcmp(symbol, "GOLD") == 0) {
            snprintf(buf, sizeof(buf), "%-10s  ฿%d  %s%.2f%%", symbol, (int)price, trendSign, changePct);
        } else {
            snprintf(buf, sizeof(buf), "%-10s  $%.2f  %s%.2f%%", symbol, price, trendSign, changePct);
        }

        if (s_stockRows[idx]) {
            lv_label_set_text(s_stockRows[idx], buf);
        }
        idx++;
    }
}

void destroy_page_stocks() {
    // Invalidate all static widget pointers before LVGL frees the parent container.
    for (int i = 0; i < 4; i++) s_stockRows[i] = nullptr;
}
