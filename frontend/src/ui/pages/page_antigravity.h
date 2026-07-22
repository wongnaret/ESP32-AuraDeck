/**
 * @file page_antigravity.h
 * @brief Header for Screen 1 (Antigravity Credits Screen) of the AuraDeck dashboard.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

void create_page_antigravity(lv_obj_t* parent);
void update_page_antigravity(const JsonDocument& doc);
