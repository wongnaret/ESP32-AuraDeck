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
    // Support two payload shapes from backend:
    //   Shape A — root array (from device-specific topics, get_multi_asset_prices() return type):
    //     [{"symbol":"GOLD/TH", "price":41200, "change_pct":0.12, "type":"GOLD"}, ...]
    //   Shape B — wrapped object (from generic topic):
    //     {"stocks": [{"symbol":"...", "price":..., "change_percent":...}, ...]}
    JsonArrayConst stocks;
    bool useChangePct = false; // true = read "change_pct" field (backend native), false = read "change_percent"

    if (doc.is<JsonArray>()) {
        stocks = doc.as<JsonArrayConst>(); // Shape A: root array
        useChangePct = true; // backend native field name
    } else if (doc.containsKey("stocks")) {
        stocks = doc["stocks"].as<JsonArrayConst>(); // Shape B: wrapped object
        useChangePct = false; // legacy field name
    } else {
        return; // Unrecognized payload — skip silently
    }

    int idx = 0;

    for (JsonObjectConst stock : stocks) {
        if (idx >= 4) break;

        const char* symbol = stock["symbol"] | "";
        float price = stock["price"] | 0.0;
        // Support both field names for cross-format compatibility
        float changePct = useChangePct
            ? (stock["change_pct"] | 0.0)
            : (stock["change_percent"] | stock["change_pct"] | 0.0);

        char buf[64];
        const char* trendSign = (changePct >= 0) ? "+ " : "- ";
        // Normalize negative change for display (sign is already shown via trendSign)
        float absPct = changePct < 0 ? -changePct : changePct;

        // Format based on asset type
        const char* assetType = stock["type"] | "";
        if (strcmp(assetType, "GOLD") == 0) {
            snprintf(buf, sizeof(buf), "%-10s  \xE0\xB8\xBF%d  %s%.2f%%",
                     symbol, (int)price, trendSign, absPct);
        } else if (strcmp(assetType, "TH_STOCK") == 0) {
            snprintf(buf, sizeof(buf), "%-10s  %.2f  %s%.2f%%",
                     symbol, price, trendSign, absPct);
        } else {
            // CRYPTO, GLOBAL, etc.
            snprintf(buf, sizeof(buf), "%-10s  $%.2f  %s%.2f%%",
                     symbol, price, trendSign, absPct);
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
