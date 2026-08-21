/**
 * @file page_spotify.cpp
 * @brief Source for Screen 5 (Spotify Screen) of the AuraDeck dashboard.
 * Layout:
 * - Top Header: Left-aligned "🎵 Spotify Player" + Right-aligned Status Pill "[▶ PLAYING]" / "[⏸ PAUSED]"
 * - Left Box (80x80): High-Contrast Playback Status Widget (Icon + Subtext)
 * - Right Area (270px): Smooth Marquee Track Title (Prompt 24) and Artist (Prompt 16)
 * - Middle Area: Progress Bar + Elapsed / Duration Timestamps
 * - Center Box (368x72): 2-Line Synced Lyrics with ThaiReshaper or Album Name Fallback
 * - Footer: "🔘 Press BOOT to Play / Pause" Action Hint
 */

#include "page_spotify.h"
#include "ui/thai_reshaper.h"
#include "ui/fonts/lv_font_prompt.h"
#include <Arduino.h>

static lv_obj_t* s_screenTitleLabel    = nullptr;
static lv_obj_t* s_statusBadgeLabel    = nullptr;
static lv_obj_t* s_playbackBox         = nullptr;
static lv_obj_t* s_playbackIconLabel   = nullptr;
static lv_obj_t* s_playbackSubtextLabel= nullptr;
static lv_obj_t* s_trackLabel          = nullptr;
static lv_obj_t* s_artistLabel         = nullptr;
static lv_obj_t* s_bar                 = nullptr;
static lv_obj_t* s_progressTimeLabel   = nullptr;
static lv_obj_t* s_durationTimeLabel   = nullptr;
static lv_obj_t* s_lyricsBox           = nullptr;
static lv_obj_t* s_lyricCurrentLabel   = nullptr;
static lv_obj_t* s_lyricNextLabel      = nullptr;
static lv_obj_t* s_footerLabel         = nullptr;

// Live playback state tracking for 1-second local progress bar animation tick
static bool s_isPlaying = false;
static int s_progressSec = 0;
static int s_durationSec = 0;
static uint32_t s_lastProgressTickMs = 0;

static void formatTime(int totalSeconds, char* outBuf, size_t bufSize) {
    if (totalSeconds < 0) totalSeconds = 0;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    snprintf(outBuf, bufSize, "%d:%02d", minutes, seconds);
}

void create_page_spotify(lv_obj_t* parent) {
    // Reset static pointers first (safety guard for re-entry)
    s_screenTitleLabel     = nullptr;
    s_statusBadgeLabel     = nullptr;
    s_playbackBox          = nullptr;
    s_playbackIconLabel    = nullptr;
    s_playbackSubtextLabel = nullptr;
    s_trackLabel           = nullptr;
    s_artistLabel          = nullptr;
    s_bar                  = nullptr;
    s_progressTimeLabel    = nullptr;
    s_durationTimeLabel    = nullptr;
    s_lyricsBox            = nullptr;
    s_lyricCurrentLabel    = nullptr;
    s_lyricNextLabel       = nullptr;
    s_footerLabel          = nullptr;

    // ==========================================
    // 1. Top Header: Left Title + Right Badge
    // ==========================================
    s_screenTitleLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_screenTitleLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_screenTitleLabel, lv_color_black(), 0);
    lv_obj_align(s_screenTitleLabel, LV_ALIGN_TOP_LEFT, 14, 6);
    lv_label_set_text(s_screenTitleLabel, LV_SYMBOL_AUDIO " Spotify Player");

    s_statusBadgeLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_statusBadgeLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_statusBadgeLabel, LV_ALIGN_TOP_RIGHT, -14, 8);
    lv_label_set_text(s_statusBadgeLabel, "[" LV_SYMBOL_PAUSE " PAUSED]");

    // ==========================================
    // 2. Playback Control Box (80x80 Left Widget)
    // ==========================================
    s_playbackBox = lv_obj_create(parent);
    lv_obj_set_size(s_playbackBox, 80, 80);
    lv_obj_align(s_playbackBox, LV_ALIGN_TOP_LEFT, 14, 28);
    lv_obj_set_style_radius(s_playbackBox, 6, 0);
    lv_obj_set_style_border_color(s_playbackBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_playbackBox, 2, 0);
    lv_obj_set_style_bg_color(s_playbackBox, lv_color_white(), 0);
    lv_obj_set_style_pad_all(s_playbackBox, 0, 0);
    lv_obj_clear_flag(s_playbackBox, LV_OBJ_FLAG_SCROLLABLE);

    s_playbackIconLabel = lv_label_create(s_playbackBox);
    lv_obj_set_style_text_font(s_playbackIconLabel, &lv_font_montserrat_24, 0);
    lv_obj_align(s_playbackIconLabel, LV_ALIGN_TOP_MID, 0, 12);
    lv_label_set_text(s_playbackIconLabel, LV_SYMBOL_PAUSE);

    s_playbackSubtextLabel = lv_label_create(s_playbackBox);
    lv_obj_set_style_text_font(s_playbackSubtextLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_playbackSubtextLabel, LV_ALIGN_TOP_MID, 0, 48);
    lv_label_set_text(s_playbackSubtextLabel, "PAUSED");

    // ==========================================
    // 3. Track Title & Artist (Right of 80x80 box)
    // ==========================================
    s_trackLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_trackLabel, &lv_font_prompt_24, 0);
    lv_obj_set_style_text_color(s_trackLabel, lv_color_black(), 0);
    lv_obj_set_width(s_trackLabel, 274);
    lv_label_set_long_mode(s_trackLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(s_trackLabel, LV_ALIGN_TOP_LEFT, 106, 30);
    lv_label_set_text(s_trackLabel, "Spotify Ready");

    s_artistLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_artistLabel, &lv_font_prompt_16, 0);
    lv_obj_set_style_text_color(s_artistLabel, lv_color_black(), 0);
    lv_obj_set_width(s_artistLabel, 274);
    lv_label_set_long_mode(s_artistLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(s_artistLabel, LV_ALIGN_TOP_LEFT, 106, 68);
    lv_label_set_text(s_artistLabel, "Press BOOT to start music");

    // ==========================================
    // 4. Progress Bar & Elapsed / Duration Times
    // ==========================================
    s_bar = lv_bar_create(parent);
    lv_obj_set_size(s_bar, 368, 8);
    lv_obj_align(s_bar, LV_ALIGN_TOP_MID, 0, 118);
    lv_obj_set_style_bg_color(s_bar, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_color(s_bar, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_bar, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_bar, lv_color_black(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_bar, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(s_bar, 2, LV_PART_INDICATOR);
    lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);

    s_progressTimeLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_progressTimeLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_progressTimeLabel, LV_ALIGN_TOP_LEFT, 16, 130);
    lv_label_set_text(s_progressTimeLabel, "0:00");

    s_durationTimeLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_durationTimeLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_durationTimeLabel, LV_ALIGN_TOP_RIGHT, -16, 130);
    lv_label_set_text(s_durationTimeLabel, "0:00");

    // ==========================================
    // 5. Lyrics / Album Container (368x72)
    // ==========================================
    s_lyricsBox = lv_obj_create(parent);
    lv_obj_set_size(s_lyricsBox, 368, 72);
    lv_obj_align(s_lyricsBox, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_set_style_radius(s_lyricsBox, 4, 0);
    lv_obj_set_style_border_color(s_lyricsBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_lyricsBox, 1, 0);
    lv_obj_set_style_bg_color(s_lyricsBox, lv_color_white(), 0);
    lv_obj_set_style_pad_all(s_lyricsBox, 2, 0);
    lv_obj_clear_flag(s_lyricsBox, LV_OBJ_FLAG_SCROLLABLE);

    s_lyricCurrentLabel = lv_label_create(s_lyricsBox);
    lv_obj_set_style_text_font(s_lyricCurrentLabel, &lv_font_prompt_16, 0);
    lv_obj_set_style_text_color(s_lyricCurrentLabel, lv_color_black(), 0);
    lv_obj_set_style_text_align(s_lyricCurrentLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(s_lyricCurrentLabel, 356);
    lv_label_set_long_mode(s_lyricCurrentLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(s_lyricCurrentLabel, LV_ALIGN_TOP_MID, 0, 4);
    lv_label_set_text(s_lyricCurrentLabel, "... ... ...");

    s_lyricNextLabel = lv_label_create(s_lyricsBox);
    lv_obj_set_style_text_font(s_lyricNextLabel, &lv_font_prompt_16, 0);
    lv_obj_set_style_text_color(s_lyricNextLabel, lv_color_black(), 0);
    lv_obj_set_style_text_align(s_lyricNextLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(s_lyricNextLabel, 356);
    lv_label_set_long_mode(s_lyricNextLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(s_lyricNextLabel, LV_ALIGN_TOP_MID, 0, 36);
    lv_label_set_text(s_lyricNextLabel, "");

    // ==========================================
    // 6. Action Hint Footer Info
    // ==========================================
    s_footerLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_footerLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(s_footerLabel, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_label_set_text(s_footerLabel, "BOOT: Play / Pause");
}

void update_page_spotify(JsonVariantConst data) {
    if (data.isNull()) return;

    bool isPlaying = data["is_playing"] | false;
    const char* track = data["title"] | data["track"] | "";
    const char* artist = data["artist"] | "";
    const char* album = data["album"] | "";
    const char* curLyric = data["current_lyric"] | "";
    const char* nextLyric = data["next_lyric"] | "";

    // Track absence check (Idle / No music at all)
    if (!track || strlen(track) == 0 || strcmp(track, "Not Playing") == 0) {
        s_isPlaying = false;
        if (s_statusBadgeLabel) lv_label_set_text(s_statusBadgeLabel, "[" LV_SYMBOL_PAUSE " IDLE]");
        if (s_playbackIconLabel) lv_label_set_text(s_playbackIconLabel, LV_SYMBOL_AUDIO);
        if (s_playbackSubtextLabel) lv_label_set_text(s_playbackSubtextLabel, "IDLE");
        if (s_trackLabel) lv_label_set_text(s_trackLabel, "Spotify Idle");
        if (s_artistLabel) lv_label_set_text(s_artistLabel, "No music playing");
        if (s_bar) lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);
        if (s_progressTimeLabel) lv_label_set_text(s_progressTimeLabel, "0:00");
        if (s_durationTimeLabel) lv_label_set_text(s_durationTimeLabel, "0:00");
        if (s_lyricCurrentLabel) lv_label_set_text(s_lyricCurrentLabel, "");
        if (s_lyricNextLabel) lv_label_set_text(s_lyricNextLabel, "");
        if (s_footerLabel) lv_label_set_text(s_footerLabel, "BOOT: Resume Playback");
        s_progressSec = 0;
        s_durationSec = 0;
        return;
    }

    // Active or Paused Song session
    s_isPlaying = isPlaying;

    // 1. Update Playback Status Badge & Control Box
    if (isPlaying) {
        if (s_statusBadgeLabel) lv_label_set_text(s_statusBadgeLabel, "[" LV_SYMBOL_PLAY " PLAYING]");
        if (s_playbackIconLabel) lv_label_set_text(s_playbackIconLabel, LV_SYMBOL_PLAY);
        if (s_playbackSubtextLabel) lv_label_set_text(s_playbackSubtextLabel, "PLAYING");
        if (s_footerLabel) lv_label_set_text(s_footerLabel, "BOOT: Pause Playback");
    } else {
        if (s_statusBadgeLabel) lv_label_set_text(s_statusBadgeLabel, "[" LV_SYMBOL_PAUSE " PAUSED]");
        if (s_playbackIconLabel) lv_label_set_text(s_playbackIconLabel, LV_SYMBOL_PAUSE);
        if (s_playbackSubtextLabel) lv_label_set_text(s_playbackSubtextLabel, "PAUSED");
        if (s_footerLabel) lv_label_set_text(s_footerLabel, "BOOT: Resume Playback");
    }

    // 2. Track & Artist Names (with Thai Reshaping)
    if (s_trackLabel) {
        String reshapedTrack = ThaiReshaper::reshape(track);
        lv_label_set_text(s_trackLabel, reshapedTrack.c_str());
    }

    if (s_artistLabel) {
        String reshapedArtist = ThaiReshaper::reshape(artist);
        lv_label_set_text(s_artistLabel, reshapedArtist.c_str());
    }

    // 3. Progress and Duration Calculation
    int pMs = data["progress_ms"] | 0;
    int dMs = data["duration_ms"] | 0;

    s_progressSec = pMs / 1000;
    s_durationSec = dMs / 1000;
    s_lastProgressTickMs = millis();

    if (s_bar && s_durationSec > 0) {
        lv_bar_set_range(s_bar, 0, s_durationSec);
        lv_bar_set_value(s_bar, s_progressSec, LV_ANIM_OFF);
    }

    char timeBuf[16];
    if (s_progressTimeLabel) {
        formatTime(s_progressSec, timeBuf, sizeof(timeBuf));
        lv_label_set_text(s_progressTimeLabel, timeBuf);
    }

    if (s_durationTimeLabel) {
        formatTime(s_durationSec, timeBuf, sizeof(timeBuf));
        lv_label_set_text(s_durationTimeLabel, timeBuf);
    }

    // 4. Synced Lyrics or Album Info
    bool hasLyrics = data["has_lyrics"] | false;
    if (hasLyrics && curLyric && strlen(curLyric) > 0) {
        if (s_lyricCurrentLabel) {
            String reshaped = ThaiReshaper::reshape(curLyric);
            lv_label_set_text(s_lyricCurrentLabel, reshaped.c_str());
        }
        if (s_lyricNextLabel) {
            String reshapedNext = (nextLyric && strlen(nextLyric) > 0) ? ThaiReshaper::reshape(nextLyric) : "";
            lv_label_set_text(s_lyricNextLabel, reshapedNext.c_str());
        }
    } else {
        if (s_lyricCurrentLabel) {
            if (album && strlen(album) > 0) {
                char aBuf[128];
                snprintf(aBuf, sizeof(aBuf), "Album: %s", album);
                String reshaped = ThaiReshaper::reshape(aBuf);
                lv_label_set_text(s_lyricCurrentLabel, reshaped.c_str());
            } else {
                lv_label_set_text(s_lyricCurrentLabel, "♪ ♪ ♪");
            }
        }
        if (s_lyricNextLabel) {
            lv_label_set_text(s_lyricNextLabel, "");
        }
    }
}

void update_page_spotify_tick() {
    if (!s_isPlaying || s_durationSec <= 0) return;

    uint32_t now = millis();
    if (now - s_lastProgressTickMs >= 1000) {
        s_lastProgressTickMs = now;
        s_progressSec++;
        if (s_progressSec > s_durationSec) {
            s_progressSec = s_durationSec;
        }

        if (s_bar) {
            lv_bar_set_value(s_bar, s_progressSec, LV_ANIM_OFF);
        }

        if (s_progressTimeLabel) {
            char timeBuf[16];
            formatTime(s_progressSec, timeBuf, sizeof(timeBuf));
            lv_label_set_text(s_progressTimeLabel, timeBuf);
        }
    }
}

void destroy_page_spotify() {
    s_screenTitleLabel     = nullptr;
    s_statusBadgeLabel     = nullptr;
    s_playbackBox          = nullptr;
    s_playbackIconLabel    = nullptr;
    s_playbackSubtextLabel = nullptr;
    s_trackLabel           = nullptr;
    s_artistLabel          = nullptr;
    s_bar                  = nullptr;
    s_progressTimeLabel    = nullptr;
    s_durationTimeLabel    = nullptr;
    s_lyricsBox            = nullptr;
    s_lyricCurrentLabel    = nullptr;
    s_lyricNextLabel       = nullptr;
    s_footerLabel          = nullptr;
}
