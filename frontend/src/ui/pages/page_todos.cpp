/**
 * @file page_todos.cpp
 * @brief Source for Screen 3 (Checklist Screen) of the AuraDeck dashboard.
 */

#include "page_todos.h"
#include "ui/thai_reshaper.h"
#include "ui/fonts/lv_font_prompt.h"

// Support up to 8 tasks (matches backend google_api.py limit of 8 aggregated tasks)
static lv_obj_t* s_todoRows[8] = { nullptr };

void create_page_todos(lv_obj_t* parent) {
    // Reset all row pointers (safety guard for re-entry)
    for (int i = 0; i < 8; i++) s_todoRows[i] = nullptr;

    // Screen Title
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);
    lv_label_set_text(title, "Checklist Tasks");

    // 8 task rows — rowSpacing=27 keeps all 8 within the 274px content area
    const int startY     = 55;
    const int rowSpacing = 27;

    for (int i = 0; i < 8; i++) {
        s_todoRows[i] = lv_label_create(parent);
        lv_obj_set_style_text_font(s_todoRows[i], &lv_font_prompt_16, 0);
        lv_obj_set_width(s_todoRows[i], 370);              // Constrain width for ellipsis
        lv_label_set_long_mode(s_todoRows[i], LV_LABEL_LONG_DOT);
        lv_obj_align(s_todoRows[i], LV_ALIGN_TOP_LEFT, 20, startY + (i * rowSpacing));
        lv_label_set_text(s_todoRows[i], "");              // Blank until payload arrives
    }
}

void update_page_todos(JsonVariantConst data) {
    JsonArrayConst todos;
    if (data.is<JsonArrayConst>()) {
        todos = data.as<JsonArrayConst>(); // Shape A: root array
    } else if (data["todos"].is<JsonArrayConst>()) {
        todos = data["todos"].as<JsonArrayConst>(); // Shape B: wrapped object
    } else {
        return; // Unrecognized payload format — skip silently
    }

    int idx = 0;

    for (JsonVariantConst item : todos) {
        if (idx >= 8) break;

        const char* todoText = nullptr;

        if (item.is<JsonObjectConst>()) {
            // Object: {"id":"...", "title":"[List] Task", "completed":false}
            todoText = item["title"] | "Untitled Task";
        } else {
            // Plain string element (legacy / fallback format)
            todoText = item.as<const char*>();
        }

        if (!todoText || todoText[0] == '\0') {
            idx++;
            continue;
        }

        // Apply Thai Unicode Reshaper to handle Thai floating vowel rendering
        String reshapedTodo = ThaiReshaper::reshape(todoText);

        char buf[128];
        snprintf(buf, sizeof(buf), "[ ] %s", reshapedTodo.c_str());

        if (s_todoRows[idx]) {
            lv_label_set_text(s_todoRows[idx], buf);
        }
        idx++;
    }

    // Clear remaining slots
    for (int i = idx; i < 8; i++) {
        if (s_todoRows[i]) {
            lv_label_set_text(s_todoRows[i], "");
        }
    }
}

void destroy_page_todos() {
    for (int i = 0; i < 8; i++) s_todoRows[i] = nullptr;
}
