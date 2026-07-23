/**
 * @file ui_manager.h
 * @brief Coordinates the LVGL graphics library, page rotations, and top headers.
 */

#pragma once

#include <lvgl.h>
#include <ArduinoJson.h>
#include "drivers/display_st7305.h"

class UIManager {
public:
    UIManager();
    ~UIManager();

    /**
     * @brief Hooks LVGL to the ST7305 hardware display and builds the default screen layout.
     * @param display Pointer to initialized physical display driver.
     * @return true if successful, false otherwise.
     */
    bool begin(ST7305Display* display);
    
    /**
     * @brief Renders the visual boot up sequence/diagnostics page on the screen.
     * @param status Description of current boot phase (e.g. "Connecting to Wi-Fi...")
     * @param progress_percent Value between 0 and 100
     */
    void drawBootStatus(const char* status, int progress_percent);

    /**
     * @brief Completes the boot sequence and transition to the full interactive dashboard page.
     */
    void completeBoot();

    /**
     * @brief Polled in the main loop to handle LVGL core rendering cycles.
     */
    void tick();

    /**
     * @brief Displays the screen corresponding to the given page index.
     * Automatically cleans up the previous screen to maintain free heap space.
     * @param pageIndex 0 (Home) to 6 (Analytics).
     */
    void showPage(int pageIndex);

    /**
     * @brief Dispatches telemetry JSON payloads to the respective page view modules.
     * @param topic Target MQTT topic.
     * @param doc Parsed JSON document.
     */
    void dispatchData(const char* topic, const JsonDocument& doc);

    /**
     * @brief Updates top header labels directly.
     * @param time formatted time string (e.g. "14:30")
     * @param temp live temperature float
     * @param hum live humidity float
     * @param wifiConnected wifi status boolean
     * @param mqttConnected mqtt broker connection status
     */
    void updateHeader(const char* time, float temp, float hum, bool wifiConnected, bool mqttConnected);

private:
    ST7305Display* m_hardwareDisplay = nullptr;
    lv_disp_draw_buf_t m_drawBuf;
    lv_disp_drv_t m_dispDrv;
    lv_color_t* m_lvBuffer1 = nullptr;
    
    // Core layout parent pointers
    lv_obj_t* m_topHeaderContainer = nullptr;
    lv_obj_t* m_activePageContainer = nullptr;

    // Header labels
    lv_obj_t* m_headerTitleLabel    = nullptr;
    lv_obj_t* m_headerTimeLabel     = nullptr;
    lv_obj_t* m_headerSensorLabel   = nullptr;
    lv_obj_t* m_headerWifiIconLabel = nullptr;  ///< WiFi + MQTT combined status icon

    int m_currentPageIndex = 0;

    // Custom LVGL flush callback bridge
    static void dispFlushCallback(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);
    
    void createPersistentHeader();
    const char* getPageName(int pageIndex);
};
