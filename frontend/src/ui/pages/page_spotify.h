/**
 * @file page_spotify.h
 * @brief Header for Screen 5 (Spotify Screen) of the AuraDeck dashboard.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

void create_page_spotify(lv_obj_t* parent);
void update_page_spotify(const JsonDocument& doc);

/// @brief Resets static widget pointers to nullptr. Call before lv_obj_clean() on page switch.
void destroy_page_spotify();
