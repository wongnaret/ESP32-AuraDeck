/**
 * @file page_spotify.cpp
 * @brief Source for Screen 5 (Spotify Screen) of the AuraDeck dashboard.
 */

#include "page_spotify.h"
#include "ui/thai_reshaper.h"

static lv_obj_t* s_trackLabel = nullptr;
static lv_obj_t* s_artistLabel = nullptr;
static lv_obj_t* s_bar = nullptr;
static lv_obj_t* s_progressTimeLabel = nullptr;
static lv_obj_t* s_durationTimeLabel = nullptr;
static lv_obj_t* s_stateLabel = nullptr;

void create_page_spotify(lv_obj_t* parent) {
    // 1. Screen Title (Decorative Spotify Logo Tag)
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);
    lv_label_set_text(title, "Now Playing");

    // 2. Bold Track Name
    s_trackLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_trackLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_width(s_trackLabel, 340);
    lv_label_set_long_mode(s_trackLabel, LV_LABEL_LONG_DOT);
    lv_obj_align(s_trackLabel, LV_ALIGN_CENTER, 0, -35);
    lv_label_set_text(s_trackLabel, "Spotify Offline");

    // 3. Sub-label for Artist Names
    s_artistLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_artistLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_width(s_artistLabel, 340);
    lv_label_set_long_mode(s_artistLabel, LV_LABEL_LONG_DOT);
    lv_obj_align(s_artistLabel, LV_ALIGN_CENTER, 0, -5);
    lv_label_set_text(s_artistLabel, "Connect device to sync music");

    // 4. Playback Progress Slider Bar
    s_bar = lv_bar_create(parent);
    lv_obj_set_size(s_bar, 280, 10);
    lv_obj_align(s_bar, LV_ALIGN_CENTER, 0, 30);
    lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);

    // 5. Progress Time (Left Corner)
    s_progressTimeLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_progressTimeLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_progressTimeLabel, LV_ALIGN_CENTER, -125, 50);
    lv_label_set_text(s_progressTimeLabel, "0:00");

    // 6. Duration Time (Right Corner)
    s_durationTimeLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_durationTimeLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_durationTimeLabel, LV_ALIGN_CENTER, 125, 50);
    lv_label_set_text(s_durationTimeLabel, "0:00");

    // 7. Status footer info
    s_stateLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_stateLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_stateLabel, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_label_set_text(s_stateLabel, "Playback Stopped");
}

static void formatTime(int totalSeconds, char* outBuf, size_t bufSize) {
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    snprintf(outBuf, bufSize, "%d:%02d", minutes, seconds);
}

void update_page_spotify(const JsonDocument& doc) {
    bool isPlaying = doc["is_playing"] | false;

    if (!isPlaying) {
        if (s_trackLabel) lv_label_set_text(s_trackLabel, "Spotify Idle");
        if (s_artistLabel) lv_label_set_text(s_artistLabel, "No music playing");
        if (s_bar) lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);
        if (s_progressTimeLabel) lv_label_set_text(s_progressTimeLabel, "0:00");
        if (s_durationTimeLabel) lv_label_set_text(s_durationTimeLabel, "0:00");
        if (s_stateLabel) lv_label_set_text(s_stateLabel, "Playback Stopped");
        return;
    }

    const char* track = doc["track"] | "Unknown Track";
    const char* artist = doc["artist"] | "Unknown Artist";
    int progress = doc["progress"] | 0;
    int duration = doc["duration"] | 0;

    // Apply Thai Unicode Reshaper to track and artist (for Thai music)
    String reshapedTrack = ThaiReshaper::reshape(track);
    String reshapedArtist = ThaiReshaper::reshape(artist);

    if (s_trackLabel) lv_label_set_text(s_trackLabel, reshapedTrack.c_str());
    if (s_artistLabel) lv_label_set_text(s_artistLabel, reshapedArtist.c_str());

    // Calculate percentage progress
    int percentage = 0;
    if (duration > 0) {
        percentage = (progress * 100) / duration;
    }
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;

    if (s_bar) lv_bar_set_value(s_bar, percentage, LV_ANIM_OFF);

    // Format progress and duration timestamps
    char timeBuf[16];
    formatTime(progress, timeBuf, sizeof(timeBuf));
    if (s_progressTimeLabel) lv_label_set_text(s_progressTimeLabel, timeBuf);

    formatTime(duration, timeBuf, sizeof(timeBuf));
    if (s_durationTimeLabel) lv_label_set_text(s_durationTimeLabel, timeBuf);

    if (s_stateLabel) lv_label_set_text(s_stateLabel, "Active Streaming...");
}
