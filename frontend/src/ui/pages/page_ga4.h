/**
 * @file page_ga4.h
 * @brief Header for Screen 6 (Dedicated Google Analytics 4) of the AuraDeck dashboard.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

/**
 * @brief Creates the GA4 Analytics UI elements under the specified parent container.
 */
void create_page_ga4(lv_obj_t* parent);

/**
 * @brief Dynamically updates GA4 metrics (30m active users, 28D users, new users, engagement, events, top cities).
 */
void update_page_ga4(JsonVariantConst data);

/**
 * @brief Cycles through multiple configured GA4 Properties (1/N -> 2/N ...).
 */
void cycle_ga4_property();

/**
 * @brief Resets all static widget pointers to nullptr before parent container is cleaned.
 */
void destroy_page_ga4();

