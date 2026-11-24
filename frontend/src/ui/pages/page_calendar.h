/**
 * @file page_calendar.h
 * @brief Header for Screen 4 (Calendar & Upcoming Agendas) of the AuraDeck dashboard.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

void create_page_calendar(lv_obj_t* parent);
void update_page_calendar(JsonVariantConst data);
void cycle_calendar_page();

/// @brief Resets static widget pointers and frees cached JSON. Call before lv_obj_clean() on page switch.
void destroy_page_calendar();
