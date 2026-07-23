/**
 * @file page_analytics.h
 * @brief Header for Screen 6 (GCP Billing & GA4 Analytics) of the AuraDeck dashboard.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

void create_page_analytics(lv_obj_t* parent);
void update_page_analytics(const JsonDocument& doc);

/// @brief Resets static widget pointers to nullptr. Call before lv_obj_clean() on page switch.
void destroy_page_analytics();
