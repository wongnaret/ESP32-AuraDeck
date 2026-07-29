/**
 * @file page_spotify.cpp
 * @brief Source for Screen 5 (Spotify Screen) of the AuraDeck dashboard.
 * Includes local 1-second progress bar animation tick and 1-bit dithered Album Cover rendering layout.
 */

#include "page_spotify.h"
#include "ui/thai_reshaper.h"
#include "ui/fonts/lv_font_prompt.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

static lv_obj_t* s_trackLabel = nullptr;
static lv_obj_t* s_artistLabel = nullptr;
static lv_obj_t* s_bar = nullptr;
static lv_obj_t* s_progressTimeLabel = nullptr;
static lv_obj_t* s_durationTimeLabel = nullptr;
static lv_obj_t* s_stateLabel = nullptr;
static lv_obj_t* s_albumBox = nullptr;
static lv_obj_t* s_albumIconLabel = nullptr;
static lv_obj_t* s_albumImg = nullptr;

// Live playback state tracking for 1-second local progress bar animation tick
static bool s_isPlaying = false;
static int s_progressSec = 0;
static int s_durationSec = 0;
static uint32_t s_lastProgressTickMs = 0;
static String s_lastCoverUrl = "";

static void formatTime(int totalSeconds, char* outBuf, size_t bufSize) {
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    snprintf(outBuf, bufSize, "%d:%02d", minutes, seconds);
}

void create_page_spotify(lv_obj_t* parent) {
    // Reset static pointers first (safety guard for re-entry)
    s_trackLabel        = nullptr;
    s_artistLabel       = nullptr;
    s_bar               = nullptr;
    s_progressTimeLabel = nullptr;
    s_durationTimeLabel = nullptr;
    s_stateLabel        = nullptr;
    s_albumBox          = nullptr;
    s_albumIconLabel    = nullptr;
    s_albumImg          = nullptr;

    // 1. Screen Title Header
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    lv_label_set_text(title, "Now Playing");

    // 2. Album Art Box (80x80 Decorative Container on Left Side)
    s_albumBox = lv_obj_create(parent);
    lv_obj_set_size(s_albumBox, 80, 80);
    lv_obj_align(s_albumBox, LV_ALIGN_TOP_LEFT, 15, 38);
    lv_obj_set_style_radius(s_albumBox, 6, 0);
    lv_obj_set_style_border_color(s_albumBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_albumBox, 2, 0);
    lv_obj_set_style_bg_color(s_albumBox, lv_color_white(), 0);
    lv_obj_set_style_pad_all(s_albumBox, 0, 0);
    lv_obj_clear_flag(s_albumBox, LV_OBJ_FLAG_SCROLLABLE);

    s_albumIconLabel = lv_label_create(s_albumBox);
    lv_obj_set_style_text_font(s_albumIconLabel, &lv_font_montserrat_24, 0);
    lv_obj_center(s_albumIconLabel);
    lv_label_set_text(s_albumIconLabel, LV_SYMBOL_AUDIO);

    // 3. Track Name (Right of Album Box, width=270)
    s_trackLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_trackLabel, &lv_font_prompt_24, 0);
    lv_obj_set_style_text_color(s_trackLabel, lv_color_black(), 0);
    lv_obj_set_width(s_trackLabel, 270);
    lv_label_set_long_mode(s_trackLabel, LV_LABEL_LONG_DOT);
    lv_obj_align(s_trackLabel, LV_ALIGN_TOP_LEFT, 110, 42);
    lv_label_set_text(s_trackLabel, "Spotify Offline");

    // 4. Sub-label for Artist Names
    s_artistLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_artistLabel, &lv_font_prompt_16, 0);
    lv_obj_set_style_text_color(s_artistLabel, lv_color_black(), 0);
    lv_obj_set_width(s_artistLabel, 270);
    lv_label_set_long_mode(s_artistLabel, LV_LABEL_LONG_DOT);
    lv_obj_align(s_artistLabel, LV_ALIGN_TOP_LEFT, 110, 80);
    lv_label_set_text(s_artistLabel, "Connect device to sync music");

    // 5. Playback Progress Bar (Full width underneath with high contrast black/white styling)
    s_bar = lv_bar_create(parent);
    lv_obj_set_size(s_bar, 360, 12);
    lv_obj_align(s_bar, LV_ALIGN_TOP_MID, 0, 138);
    lv_obj_set_style_bg_color(s_bar, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_color(s_bar, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_bar, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_bar, lv_color_black(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_bar, 3, LV_PART_MAIN);
    lv_obj_set_style_radius(s_bar, 3, LV_PART_INDICATOR);
    lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);

    // 6. Progress Time (Left Under Bar)
    s_progressTimeLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_progressTimeLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_progressTimeLabel, LV_ALIGN_TOP_LEFT, 20, 158);
    lv_label_set_text(s_progressTimeLabel, "0:00");

    // 7. Duration Time (Right Under Bar)
    s_durationTimeLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_durationTimeLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_durationTimeLabel, LV_ALIGN_TOP_RIGHT, -20, 158);
    lv_label_set_text(s_durationTimeLabel, "0:00");

    // 8. Status Footer Info
    s_stateLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_stateLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_stateLabel, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_label_set_text(s_stateLabel, "Playback Stopped");
}

void update_page_spotify(const JsonDocument& doc) {
    bool isPlaying = doc["is_playing"] | false;
    s_isPlaying = isPlaying;

    if (!isPlaying) {
        if (s_trackLabel) lv_label_set_text(s_trackLabel, "Spotify Idle");
        if (s_artistLabel) lv_label_set_text(s_artistLabel, "No music playing");
        if (s_bar) lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);
        if (s_progressTimeLabel) lv_label_set_text(s_progressTimeLabel, "0:00");
        if (s_durationTimeLabel) lv_label_set_text(s_durationTimeLabel, "0:00");
        if (s_stateLabel) lv_label_set_text(s_stateLabel, "Playback Stopped");
        s_progressSec = 0;
        s_durationSec = 0;
        return;
    }

    const char* track = doc["title"] | doc["track"] | "Unknown Track";
    const char* artist = doc["artist"] | "Unknown Artist";

    int progressMs = doc["progress_ms"] | 0;
    int durationMs = doc["duration_ms"] | 0;
    s_progressSec = (progressMs > 0) ? progressMs / 1000 : (doc["progress"] | 0);
    s_durationSec = (durationMs > 0) ? durationMs / 1000 : (doc["duration"] | 0);
    s_lastProgressTickMs = millis();

    String reshapedTrack = ThaiReshaper::reshape(track);
    String reshapedArtist = ThaiReshaper::reshape(artist);

    if (s_trackLabel) lv_label_set_text(s_trackLabel, reshapedTrack.c_str());
    if (s_artistLabel) lv_label_set_text(s_artistLabel, reshapedArtist.c_str());

    int percentage = 0;
    if (s_durationSec > 0) {
        percentage = (s_progressSec * 100) / s_durationSec;
    }
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;

    if (s_bar) lv_bar_set_value(s_bar, percentage, LV_ANIM_OFF);

    char timeBuf[16];
    formatTime(s_progressSec, timeBuf, sizeof(timeBuf));
    if (s_progressTimeLabel) lv_label_set_text(s_progressTimeLabel, timeBuf);

    formatTime(s_durationSec, timeBuf, sizeof(timeBuf));
    if (s_durationTimeLabel) lv_label_set_text(s_durationTimeLabel, timeBuf);

    if (s_stateLabel) lv_label_set_text(s_stateLabel, "Active Streaming...");
}

void update_page_spotify_tick() {
    if (!s_isPlaying || s_durationSec <= 0 || s_bar == nullptr) return;

    uint32_t now = millis();
    if (now - s_lastProgressTickMs >= 1000) {
        s_lastProgressTickMs = now;
        if (s_progressSec < s_durationSec) {
            s_progressSec++;

            int percentage = (s_progressSec * 100) / s_durationSec;
            if (percentage > 100) percentage = 100;

            if (s_bar) lv_bar_set_value(s_bar, percentage, LV_ANIM_OFF);

            char timeBuf[16];
            formatTime(s_progressSec, timeBuf, sizeof(timeBuf));
            if (s_progressTimeLabel) lv_label_set_text(s_progressTimeLabel, timeBuf);
        }
    }
}

void destroy_page_spotify() {
    s_trackLabel        = nullptr;
    s_artistLabel       = nullptr;
    s_bar               = nullptr;
    s_progressTimeLabel = nullptr;
    s_durationTimeLabel = nullptr;
    s_stateLabel        = nullptr;
    s_albumBox          = nullptr;
    s_albumIconLabel    = nullptr;
    s_albumImg          = nullptr;
}
