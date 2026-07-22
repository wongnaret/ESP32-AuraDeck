/**
 * @file ui_manager.cpp
 * @brief Coordinates the LVGL graphics library, page rotations, and top headers.
 */

#include "ui_manager.h"
#include "ui/pages/page_home.h"
#include "ui/pages/page_antigravity.h"
#include "ui/pages/page_stocks.h"
#include "ui/pages/page_todos.h"
#include "ui/pages/page_calendar.h"
#include "ui/pages/page_spotify.h"
#include "ui/pages/page_analytics.h"

UIManager::UIManager() {}

UIManager::~UIManager() {
    if (m_lvBuffer1) free(m_lvBuffer1);
}

bool UIManager::begin(ST7305Display* display) {
    m_hardwareDisplay = display;

    // 1. Initialize LVGL Library Core
    lv_init();

    // 2. Allocate Display Draw Buffer in PSRAM (Rule 5: Memory Awareness)
#if defined(BOARD_HAS_PSRAM)
    m_lvBuffer1 = (lv_color_t*)ps_malloc(400 * 300 * sizeof(lv_color_t));
#else
    m_lvBuffer1 = (lv_color_t*)malloc(400 * 300 * sizeof(lv_color_t));
#endif

    if (!m_lvBuffer1) {
        Serial.println("❌ Error: Failed to allocate LVGL draw buffer in PSRAM!");
        return false;
    }

    lv_disp_draw_buf_init(&m_drawBuf, m_lvBuffer1, nullptr, 400 * 300);

    // 3. Register Display Driver with callback bridge
    lv_disp_drv_init(&m_dispDrv);
    m_dispDrv.draw_buf = &m_drawBuf;
    m_dispDrv.flush_cb = dispFlushCallback;
    m_dispDrv.hor_res = 400;
    m_dispDrv.ver_res = 300;
    m_dispDrv.user_data = m_hardwareDisplay; // Pass display pointer to static callback
    lv_disp_drv_register(&m_dispDrv);

    // 4. Create Main Layout Containers
    // Create persistent top header panel (y=0 to y=25)
    m_topHeaderContainer = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(m_topHeaderContainer, 0, 0);
    lv_obj_set_size(m_topHeaderContainer, 400, 26);
    lv_obj_set_style_border_width(m_topHeaderContainer, 0, 0);
    lv_obj_set_style_radius(m_topHeaderContainer, 0, 0);
    lv_obj_set_style_pad_all(m_topHeaderContainer, 0, 0);

    // Create main viewport frame for active pages (y=26 to y=300)
    m_activePageContainer = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(m_activePageContainer, 0, 26);
    lv_obj_set_size(m_activePageContainer, 400, 274);
    lv_obj_set_style_border_width(m_activePageContainer, 0, 0);
    lv_obj_set_style_radius(m_activePageContainer, 0, 0);
    lv_obj_set_style_pad_all(m_activePageContainer, 0, 0);

    // 5. Setup persistent top status bar
    createPersistentHeader();

    // 6. Draw default Home Screen Page 0
    showPage(0);

    Serial.println("✅ LVGL v8 Core and UI Manager initialized.");
    return true;
}

void UIManager::createPersistentHeader() {
    // Left-aligned wifi and active page name label
    m_headerTitleLabel = lv_label_create(m_topHeaderContainer);
    lv_obj_set_style_text_font(m_headerTitleLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(m_headerTitleLabel, LV_ALIGN_LEFT_MID, 15, 0);
    lv_label_set_text(m_headerTitleLabel, "AuraDeck");

    // Right-aligned clock
    m_headerTimeLabel = lv_label_create(m_topHeaderContainer);
    lv_obj_set_style_text_font(m_headerTimeLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(m_headerTimeLabel, LV_ALIGN_RIGHT_MID, -15, 0);
    lv_label_set_text(m_headerTimeLabel, "12:00");

    // Environmental readings (temp/humidity) on the left of clock
    m_headerSensorLabel = lv_label_create(m_topHeaderContainer);
    lv_obj_set_style_text_font(m_headerSensorLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(m_headerSensorLabel, LV_ALIGN_RIGHT_MID, -65, 0);
    lv_label_set_text(m_headerSensorLabel, "25.0°C");

    // Wifi Icon status symbol
    m_headerWifiIconLabel = lv_label_create(m_topHeaderContainer);
    lv_obj_set_style_text_font(m_headerWifiIconLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(m_headerWifiIconLabel, LV_ALIGN_LEFT_MID, 5, 0);
    lv_label_set_text(m_headerWifiIconLabel, "•"); // simple dot status

    // Draw horizontal dividing rule at the bottom boundary of header container
    lv_obj_t* line = lv_line_create(m_topHeaderContainer);
    static lv_point_t line_points[] = { {0, 25}, {400, 25} };
    lv_line_set_points(line, line_points, 2);
}

void UIManager::updateHeader(const char* time, float temp, float hum, bool wifiConnected) {
    if (m_headerTimeLabel && time) {
        lv_label_set_text(m_headerTimeLabel, time);
    }

    if (m_headerSensorLabel) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f°C", temp);
        lv_label_set_text(m_headerSensorLabel, buf);
    }

    if (m_headerWifiIconLabel) {
        lv_label_set_text(m_headerWifiIconLabel, wifiConnected ? "📶" : "⚠️");
    }
}

void UIManager::showPage(int pageIndex) {
    m_currentPageIndex = pageIndex;

    // Clean up previous page widgets in active container (Rule 5: Memory Leak protection)
    lv_obj_clean(m_activePageContainer);

    // Update active page name in top header left corner
    if (m_headerTitleLabel) {
        lv_label_set_text(m_headerTitleLabel, getPageName(pageIndex));
    }

    // Build the newly activated screen using specific constructors
    switch (pageIndex) {
        case 0: create_page_home(m_activePageContainer); break;
        case 1: create_page_antigravity(m_activePageContainer); break;
        case 2: create_page_stocks(m_activePageContainer); break;
        case 3: create_page_todos(m_activePageContainer); break;
        case 4: create_page_calendar(m_activePageContainer); break;
        case 5: create_page_spotify(m_activePageContainer); break;
        case 6: create_page_analytics(m_activePageContainer); break;
        default: break;
    }

    // Force LVGL display refresh cycle
    lv_refr_now(nullptr);
}

void UIManager::dispatchData(const char* topic, const JsonDocument& doc) {
    // Forward MQTT payload to individual pages to keep widgets updated in memory
    // Note: To preserve CPU, we only update widgets if data corresponds to that view
    if (strcmp(topic, "auradeck/spotify") == 0) {
        update_page_spotify(doc);
    } else if (strcmp(topic, "auradeck/home_telemetry") == 0) {
        update_page_home(doc);
    } else if (strcmp(topic, "auradeck/calendar") == 0) {
        update_page_calendar(doc);
    } else if (strcmp(topic, "auradeck/todos") == 0) {
        update_page_todos(doc);
    } else if (strcmp(topic, "auradeck/stocks") == 0) {
        update_page_stocks(doc);
    } else if (strcmp(topic, "auradeck/analytics") == 0) {
        update_page_analytics(doc);
    } else if (strcmp(topic, "auradeck/antigravity") == 0) {
        update_page_antigravity(doc);
    }
}

void UIManager::tick() {
    // Non-blocking poll for LVGL timers
    lv_timer_handler();
}

// Static bridge routing callback
void UIManager::dispFlushCallback(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p) {
    ST7305Display* hwDisp = (ST7305Display*)disp_drv->user_data;
    if (!hwDisp) return;

    int32_t x, y;
    for (y = area->y1; y <= area->y2; y++) {
        for (x = area->x1; x <= area->x2; x++) {
            // In monochrome format, active state is black (color_p->full == 0)
            bool is_black = (color_p->full == 0);
            hwDisp->drawPixel(x, y, is_black);
            color_p++;
        }
    }

    // If this is the final draw segment, commit full buffer flush to ST7305
    if (lv_disp_flush_is_last(disp_drv)) {
        hwDisp->flush();
    }

    lv_disp_flush_ready(disp_drv);
}

const char* UIManager::getPageName(int pageIndex) {
    switch (pageIndex) {
        case 0: return "AuraDeck Home";
        case 1: return "Antigravity AI";
        case 2: return "Market Quotes";
        case 3: return "Task Checklist";
        case 4: return "Google Calendar";
        case 5: return "Spotify Music";
        case 6: return "Cloud Analytics";
        default: return "Dashboard";
    }
}
