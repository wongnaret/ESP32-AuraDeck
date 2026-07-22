/**
 * @file page_home.h
 * @brief Header for Screen 0 (Home Screen) of the AuraDeck dashboard.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

/**
 * @brief Creates the Home Screen UI elements under the specified parent object.
 */
void create_page_home(lv_obj_t* parent);

/**
 * @brief Dynamically updates Home Screen contents with new telemetry/sensor data.
 */
void update_page_home(const JsonDocument& doc);
