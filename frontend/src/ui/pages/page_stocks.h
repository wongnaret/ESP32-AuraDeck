/**
 * @file page_stocks.h
 * @brief Header for Screen 2 (Stocks watches Screen) of the AuraDeck dashboard.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

void create_page_stocks(lv_obj_t* parent);
void update_page_stocks(const JsonDocument& doc);

/// @brief Resets static widget pointers to nullptr. Call before lv_obj_clean() on page switch.
void destroy_page_stocks();
