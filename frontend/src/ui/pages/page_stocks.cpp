/**
 * @file page_stocks.cpp
 * @brief Source for Screen 2 (Stocks watches Screen) of the AuraDeck dashboard.
 */

#include "page_stocks.h"

// Setup 6 static row labels for stock quotes
static lv_obj_t* s_stockRows[6] = { nullptr };

void create_page_stocks(lv_obj_t* parent) {
    // Reset static row pointers first (safety guard for re-entry)
    for (int i = 0; i < 6; i++) s_stockRows[i] = nullptr;

    // 1. Title Label
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_label_set_text(title, "Market Watchlist");

    // 2. Define vertical positions for stock slots (support up to 6 stocks)
    const int startY = 48;
    const int rowSpacing = 35;

    for (int i = 0; i < 6; i++) {
        s_stockRows[i] = lv_label_create(parent);
        lv_obj_set_style_text_font(s_stockRows[i], &lv_font_montserrat_16, 0);
        lv_obj_align(s_stockRows[i], LV_ALIGN_TOP_LEFT, 20, startY + (i * rowSpacing));
        lv_label_set_text(s_stockRows[i], ""); // blank until payload arrives
    }
}

void update_page_stocks(const JsonDocument& doc) {
    JsonArrayConst stocks;
    bool useChangePct = false;

    if (doc.is<JsonArray>()) {
        stocks = doc.as<JsonArrayConst>();
        useChangePct = true;
    } else if (doc.containsKey("stocks")) {
        stocks = doc["stocks"].as<JsonArrayConst>();
        useChangePct = false;
    } else {
        return;
    }

    int idx = 0;

    for (JsonObjectConst stock : stocks) {
        if (idx >= 6) break;

        const char* symbol = stock["symbol"] | "";
        float price = stock["price"] | 0.0;
        float changePct = useChangePct
            ? (stock["change_pct"] | 0.0)
            : (stock["change_percent"] | stock["change_pct"] | 0.0);

        char buf[64];
        const char* trendSign = (changePct >= 0) ? "+ " : "- ";
        float absPct = changePct < 0 ? -changePct : changePct;

        const char* assetType = stock["type"] | "";
        if (strcmp(assetType, "GOLD") == 0) {
            snprintf(buf, sizeof(buf), "%-10s  \xE0\xB8\xBF%d  %s%.2f%%",
                     symbol, (int)price, trendSign, absPct);
        } else if (strcmp(assetType, "TH_STOCK") == 0) {
            snprintf(buf, sizeof(buf), "%-10s  %.2f  %s%.2f%%",
                     symbol, price, trendSign, absPct);
        } else {
            snprintf(buf, sizeof(buf), "%-10s  $%.2f  %s%.2f%%",
                     symbol, price, trendSign, absPct);
        }

        if (s_stockRows[idx]) {
            lv_label_set_text(s_stockRows[idx], buf);
        }
        idx++;
    }

    // Clear unused slots if watchlist has fewer than 6 items
    for (int i = idx; i < 6; i++) {
        if (s_stockRows[i]) {
            lv_label_set_text(s_stockRows[i], "");
        }
    }
}

void destroy_page_stocks() {
    for (int i = 0; i < 6; i++) s_stockRows[i] = nullptr;
}
