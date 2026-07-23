/**
 * @file page_todos.cpp
 * @brief Source for Screen 3 (Checklist Screen) of the AuraDeck dashboard.
 */

#include "page_todos.h"
#include "ui/thai_reshaper.h"

static lv_obj_t* s_todoRows[4] = { nullptr };

void create_page_todos(lv_obj_t* parent) {
    // Reset static row pointers first (safety guard for re-entry)
    for (int i = 0; i < 4; i++) s_todoRows[i] = nullptr;

    // 1. Screen Title
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);
    lv_label_set_text(title, "Checklist Tasks");

    // 2. Setup 4 fixed task items
    const int startY = 70;
    const int rowSpacing = 45;

    for (int i = 0; i < 4; i++) {
        s_todoRows[i] = lv_label_create(parent);
        lv_obj_set_style_text_font(s_todoRows[i], &lv_font_montserrat_16, 0);
        lv_obj_set_width(s_todoRows[i], 360); // Constrain width for wrapping
        lv_label_set_long_mode(s_todoRows[i], LV_LABEL_LONG_DOT); // Ellipsis truncation (...)
        lv_obj_align(s_todoRows[i], LV_ALIGN_TOP_LEFT, 20, startY + (i * rowSpacing));
        
        // Default placeholder
        lv_label_set_text(s_todoRows[i], "[ ] No active tasks");
    }
}

void update_page_todos(const JsonDocument& doc) {
    if (!doc.containsKey("todos")) return;

    JsonArrayConst todos = doc["todos"].as<JsonArrayConst>();
    int idx = 0;

    for (const char* todo : todos) {
        if (idx >= 4) break;

        // Apply Thai Unicode Reshaper to prevent overlapping floating vowels
        String reshapedTodo = ThaiReshaper::reshape(todo);
        
        char buf[128];
        snprintf(buf, sizeof(buf), "[ ] %s", reshapedTodo.c_str());

        if (s_todoRows[idx]) {
            lv_label_set_text(s_todoRows[idx], buf);
        }
        idx++;
    }

    // Clear remaining rows if checklist contains fewer than 4 items
    for (int i = idx; i < 4; i++) {
        if (s_todoRows[i]) {
            lv_label_set_text(s_todoRows[i], "");
        }
    }
}

void destroy_page_todos() {
    // Invalidate all static widget pointers before LVGL frees the parent container.
    for (int i = 0; i < 4; i++) s_todoRows[i] = nullptr;
}
