/**
 * @file weather_icons.h
 * @brief Crisp 24x24 1-bit monochrome weather icon descriptors for LVGL.
 */

#pragma once

#include <lvgl.h>

/**
 * @brief Returns the appropriate 24x24 weather image descriptor given an icon code or condition name.
 * Supported codes: SUN, CLEAR, P_CLOUD, PARTLY_CLOUDY, CLOUD, CLOUDY, RAIN, SHOWERS, DRIZZLE, STORM, THUNDER, FOG, MIST, SNOW.
 */
const lv_img_dsc_t* get_weather_icon_dsc(const char* icon_code);
