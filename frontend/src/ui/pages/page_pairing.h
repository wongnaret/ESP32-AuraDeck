/**
 * @file page_pairing.h
 * @brief TV-style pairing screen that displays the 6-digit PIN for Web UI entry.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>

/// @brief Renders the pairing PIN screen. Call after obtaining PIN from backend.
/// @param parent   LVGL parent container (active page container from UIManager).
/// @param pin      6-digit PIN string (e.g. "645902").
/// @param gatewayIp Backend server IP string (e.g. "10.42.0.1").
void create_page_pairing(lv_obj_t* parent, const char* pin, const char* gatewayIp);

/// @brief Resets all static widget pointers. Must be called before lv_obj_clean().
void destroy_page_pairing();

/// @brief Animates the 'Waiting...' status text dot cycle. Call from main loop while pairing.
void update_page_pairing_tick();
