/**
 * @file page_gcp.h
 * @brief Header for Screen 7 (GCP Multi-Project Cloud Billing & Daily Forecast) of the AuraDeck dashboard.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

/**
 * @brief Creates the GCP Billing UI elements under the specified parent container.
 */
void create_page_gcp(lv_obj_t* parent);

/**
 * @brief Dynamically updates GCP billing metrics, service breakdowns, and daily bar chart.
 */
void update_page_gcp(JsonVariantConst data);

/**
 * @brief Cycles to the next configured GCP project and updates the screen.
 */
void cycle_gcp_project();

/**
 * @brief Resets all static widget pointers to nullptr before parent container is cleaned.
 */
void destroy_page_gcp();
