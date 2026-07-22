/**
 * @file lv_conf.h
 * @brief Custom configuration for LVGL v8.3.11 on Waveshare ESP32-S3-RLCD-4.2.
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
uint32_t custom_tick_get(void);
#ifdef __cplusplus
}
#endif

/* Color depth: 1 (1 bit per pixel, monochrome) */
#define LV_COLOR_DEPTH 1

/* Swap bits in monochrome format (0 is standard) */
#define LV_COLOR_16_SWAP 0

/* Memory management: Delegate allocations to the system heap (Rule 5: leverages PSRAM) */
#define LV_MEM_CUSTOM 1
#define LV_MEM_CUSTOM_ALLOC malloc
#define LV_MEM_CUSTOM_FREE free
#define LV_MEM_CUSTOM_REALLOC realloc

/* Custom tick source: Query Arduino millis() directly (non-blocking, thread-safe, no interrupts) */
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (custom_tick_get())

/* Graphics Theme: Enable the built-in high-contrast monochrome theme */
#define LV_USE_THEME_MONO 1

/* Typography: Enable standard default Montserrat vector fonts */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_48 1

/* Set default fallback typography */
#define LV_FONT_DEFAULT &lv_font_montserrat_16

/* Performance & Draw cycle timing */
#define LV_DISP_DEF_REFR_PERIOD 30      /* Refresh rate in milliseconds */
#define LV_DPI_DEF 130                  /* Calculated DPI for the 4.2inch RLCD screen */

/* Use standard styling layouts */
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

#endif /*LV_CONF_H*/
