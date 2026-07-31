/**
 * @file page_todos.h
 * @brief Header for Screen 3 (Checklist Screen) of the AuraDeck dashboard.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

void create_page_todos(lv_obj_t* parent);
void update_page_todos(JsonVariantConst data);

/// @brief Resets static widget pointers to nullptr. Call before lv_obj_clean() on page switch.
void destroy_page_todos();
