/**
 * @file page_calendar.h
 * @brief Header for Screen 4 (Calendar Screen) of the AuraDeck dashboard.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

void create_page_calendar(lv_obj_t* parent);
void update_page_calendar(const JsonDocument& doc);
