/**
 * @file page_todos.h
 * @brief Header for Screen 3 (Checklist Screen) of the AuraDeck dashboard.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

void create_page_todos(lv_obj_t* parent);
void update_page_todos(const JsonDocument& doc);
