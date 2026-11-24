/**
 * @file page_calendar.cpp
 * @brief Source for Screen 4 (Dual-Card Monthly Calendar & Paginated Upcoming Agendas).
 *
 * Theme: High-contrast monochrome dual-card layout (400x274 active viewport):
 *   - Top Bar (y: 4..24): Month/Year ("NOV 2026") + Page/Action hint
 *   - Left Card (x=10, y=28, 188x238): Monthly calendar grid with Sunday divider,
 *     inverted black pill on Today, and bottom dots on Today & upcoming event days.
 *   - Right Card (x=204, y=28, 186x238): Date-grouped upcoming agendas with
 *     4x4 bullet dots, ThaiReshaper support, 2-line wrap, and BOOT pagination.
 */

#include "page_calendar.h"
#include "ui/thai_reshaper.h"
#include "ui/fonts/lv_font_prompt.h"
#include "drivers/rtc_pcf85063.h"
#include <Arduino.h>
#include <time.h>

extern PCF85063RTC g_rtc;

// Widget pointers
static lv_obj_t* s_monthTitleLabel    = nullptr;
static lv_obj_t* s_pageHintLabel      = nullptr;
static lv_obj_t* s_leftCard           = nullptr;
static lv_obj_t* s_calGridContainer   = nullptr;
static lv_obj_t* s_rightCard          = nullptr;
static lv_obj_t* s_agendaContainer    = nullptr;
static lv_obj_t* s_agendaFooterLabel  = nullptr;

// Calendar & pagination state
static DynamicJsonDocument* s_cachedCalDoc = nullptr;
static int      s_year           = 2026;
static int      s_month          = 11;
static int      s_todayDay       = 24;
static uint32_t s_eventDaysMask  = 0;

static int      s_pageStarts[16] = { 0 };
static int      s_pageCount      = 0;
static int      s_activePage     = 0;

static const char* kMonthNames[12] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

static const char* kMonthShortTitle[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static int days_in_month(int year, int month) {
    static const int kDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month < 1 || month > 12) return 30;
    if (month == 2) {
        bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        return leap ? 29 : 28;
    }
    return kDays[month - 1];
}

// Sakamoto's algorithm: returns 0=Sun..6=Sat for given y-m-d
static int weekday_of_date(int y, int m, int d) {
    static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    if (m < 3) y -= 1;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

static void resolve_fallback_date(int& outYear, int& outMonth, int& outDay) {
    DateTime rtcNow = g_rtc.now();
    if (rtcNow.year() >= 2024 && rtcNow.month() >= 1 && rtcNow.month() <= 12 && rtcNow.day() >= 1 && rtcNow.day() <= 31) {
        outYear = rtcNow.year();
        outMonth = rtcNow.month();
        outDay = rtcNow.day();
        return;
    }
    time_t nowEpoch = time(nullptr);
    struct tm tmLocal;
    if (nowEpoch > 1700000000 && localtime_r(&nowEpoch, &tmLocal)) {
        outYear = tmLocal.tm_year + 1900;
        outMonth = tmLocal.tm_mon + 1;
        outDay = tmLocal.tm_mday;
        return;
    }
    outYear = 2026;
    outMonth = 11;
    outDay = 24;
}

static lv_obj_t* create_card(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* box = lv_obj_create(parent);
    lv_obj_set_pos(box, x, y);
    lv_obj_set_size(box, w, h);
    lv_obj_set_style_bg_color(box, lv_color_white(), 0);
    lv_obj_set_style_border_color(box, lv_color_black(), 0);
    lv_obj_set_style_border_width(box, 1, 0);
    lv_obj_set_style_radius(box, 4, 0);
    lv_obj_set_style_pad_all(box, 0, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    return box;
}

static lv_obj_t* create_hline(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w) {
    lv_obj_t* line = lv_obj_create(parent);
    lv_obj_set_pos(line, x, y);
    lv_obj_set_size(line, w, 1);
    lv_obj_set_style_bg_color(line, lv_color_black(), 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_radius(line, 0, 0);
    lv_obj_set_style_pad_all(line, 0, 0);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);
    return line;
}

static lv_obj_t* create_vline(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t h) {
    lv_obj_t* line = lv_obj_create(parent);
    lv_obj_set_pos(line, x, y);
    lv_obj_set_size(line, 1, h);
    lv_obj_set_style_bg_color(line, lv_color_black(), 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_radius(line, 0, 0);
    lv_obj_set_style_pad_all(line, 0, 0);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);
    return line;
}

// Count visible spacing glyphs (skips UTF-8 continuations and Thai above/below marks)
static int count_visible_glyphs(const char* utf8) {
    if (!utf8) return 0;
    int count = 0;
    const uint8_t* p = (const uint8_t*)utf8;
    while (*p) {
        uint32_t cp = 0;
        if ((*p & 0x80) == 0) {
            cp = *p++;
        } else if ((*p & 0xE0) == 0xC0 && (p[1] & 0xC0) == 0x80) {
            cp = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
            p += 2;
        } else if ((*p & 0xF0) == 0xE0 && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80) {
            cp = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
            p += 3;
        } else if ((*p & 0xF8) == 0xF0 && p[1] && p[2] && p[3]) {
            cp = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
            p += 4;
        } else {
            p++;
            continue;
        }
        bool isCombining = (cp == 0x0E31) ||
                           (cp >= 0x0E34 && cp <= 0x0E3A) ||
                           (cp >= 0x0E47 && cp <= 0x0E4E) ||
                           (cp >= 0xF709 && cp <= 0xF70E);
        if (!isCombining) count++;
    }
    return count;
}

static String build_event_display_text(JsonObjectConst ev) {
    const char* title = ev["title"] | "Untitled Event";
    const char* timeStr = ev["time"] | "";
    bool hasIsAllDay = ev.containsKey("is_all_day");
    bool isAllDay = ev["is_all_day"] | false;

    String reshapedTitle = ThaiReshaper::reshape(title);

    if (hasIsAllDay) {
        if (!isAllDay && timeStr[0] != '\0' && strcmp(timeStr, "All Day") != 0) {
            return String(timeStr) + " " + reshapedTitle;
        }
        return reshapedTitle;
    }

    // Backward compatibility with legacy payloads lacking `is_all_day`
    if (timeStr[0] != '\0' &&
        strcmp(timeStr, "All Day") != 0 &&
        strcmp(timeStr, "All-day") != 0 &&
        strcmp(timeStr, "Tomorrow (All Day)") != 0) {
        const char* cleanTime = timeStr;
        if (strncmp(timeStr, "Tomorrow ", 9) == 0) {
            cleanTime = timeStr + 9;
        }
        return String(cleanTime) + " " + reshapedTitle;
    }
    return reshapedTitle;
}

static String build_group_key(JsonObjectConst ev) {
    bool isToday = ev["is_today"] | false;
    const char* dateStr = ev["date"] | "";
    const char* dateLbl = ev["date_label"] | "";
    if (dateStr[0] != '\0') return String(dateStr);
    if (dateLbl[0] != '\0') return String(dateLbl);
    return isToday ? String("__today__") : String("__upcoming__");
}

static String build_group_header(JsonObjectConst ev, int todayMonth, int todayDay) {
    bool isToday = ev["is_today"] | false;
    const char* dateLbl = ev["date_label"] | "";
    if (isToday) {
        if (dateLbl[0] != '\0' && strlen(dateLbl) >= 4) {
            // date_label looks like "Tue 24 Nov" -> strip weekday prefix if possible -> "Today - 24 Nov"
            const char* sp = strchr(dateLbl, ' ');
            if (sp && *(sp + 1) != '\0') {
                return String("Today - ") + String(sp + 1);
            }
        }
        char buf[32];
        const char* mStr = (todayMonth >= 1 && todayMonth <= 12) ? kMonthShortTitle[todayMonth - 1] : "Nov";
        snprintf(buf, sizeof(buf), "Today - %d %s", todayDay, mStr);
        return String(buf);
    }
    if (dateLbl[0] != '\0') {
        return String(dateLbl);
    }
    return String("Upcoming");
}

static void render_calendar_grid() {
    if (s_monthTitleLabel) {
        char buf[24];
        const char* mName = (s_month >= 1 && s_month <= 12) ? kMonthNames[s_month - 1] : "CAL";
        snprintf(buf, sizeof(buf), "%s %04d", mName, s_year);
        lv_label_set_text(s_monthTitleLabel, buf);
    }

    if (!s_calGridContainer) return;
    lv_obj_clean(s_calGridContainer);

    int totalDays = days_in_month(s_year, s_month);
    int firstWday = weekday_of_date(s_year, s_month, 1); // 0=Sun..6=Sat
    int totalCells = firstWday + totalDays;
    int numWeeks = (totalCells + 6) / 7;
    int rowH = (numWeeks <= 5) ? 38 : 34;
    const int colW = 26;
    const int x0 = 2;

    for (int day = 1; day <= totalDays; day++) {
        int cellIdx = firstWday + (day - 1);
        int col = cellIdx % 7;
        int row = cellIdx / 7;
        int cx = x0 + col * colW;
        int cy = row * rowH;

        bool isToday = (day == s_todayDay);
        bool hasDot = (day >= s_todayDay) && (day <= 31) && ((s_eventDaysMask & (1UL << day)) != 0);

        char dayStr[4];
        snprintf(dayStr, sizeof(dayStr), "%d", day);

        if (isToday) {
            lv_obj_t* pill = lv_obj_create(s_calGridContainer);
            lv_obj_set_pos(pill, cx + 2, cy + 2);
            lv_obj_set_size(pill, 22, 20);
            lv_obj_set_style_bg_color(pill, lv_color_black(), 0);
            lv_obj_set_style_border_width(pill, 0, 0);
            lv_obj_set_style_radius(pill, 3, 0);
            lv_obj_set_style_pad_all(pill, 0, 0);
            lv_obj_clear_flag(pill, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t* lbl = lv_label_create(pill);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
            lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
            lv_obj_center(lbl);
            lv_label_set_text(lbl, dayStr);
        } else {
            lv_obj_t* lbl = lv_label_create(s_calGridContainer);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
            lv_obj_set_style_text_color(lbl, lv_color_black(), 0);
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_pos(lbl, cx, cy + 4);
            lv_obj_set_width(lbl, colW);
            lv_label_set_text(lbl, dayStr);
        }

        if (hasDot) {
            lv_obj_t* dot = lv_obj_create(s_calGridContainer);
            int dotY = cy + (numWeeks <= 5 ? 24 : 23);
            lv_obj_set_pos(dot, cx + 11, dotY);
            lv_obj_set_size(dot, 4, 4);
            lv_obj_set_style_bg_color(dot, lv_color_black(), 0);
            lv_obj_set_style_border_width(dot, 0, 0);
            lv_obj_set_style_radius(dot, 2, 0);
            lv_obj_set_style_pad_all(dot, 0, 0);
            lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
        }
    }
}

static void compute_agenda_pages(JsonArrayConst events) {
    s_pageCount = 0;
    s_pageStarts[0] = 0;
    if (!events || events.size() == 0) {
        return;
    }

    const int maxY = 202;
    int idx = 0;
    int total = (int)events.size();

    while (idx < total && s_pageCount < 16) {
        s_pageStarts[s_pageCount++] = idx;
        int usedY = 0;
        String lastKey = "";
        int itemsOnPage = 0;

        while (idx < total) {
            JsonObjectConst ev = events[idx];
            String gKey = build_group_key(ev);
            bool needHeader = (gKey != lastKey);

            int headerNeed = needHeader ? (itemsOnPage > 0 ? 23 : 19) : 0;
            String text = build_event_display_text(ev);
            int glyphs = count_visible_glyphs(text.c_str());
            int lineH = (glyphs > 22) ? 34 : 18;
            int rowNeed = lineH + 3;

            if (itemsOnPage > 0 && (usedY + headerNeed + rowNeed > maxY)) {
                break;
            }

            usedY += headerNeed + rowNeed;
            lastKey = gKey;
            itemsOnPage++;
            idx++;
        }
    }
}

static void render_agenda_page() {
    if (!s_agendaContainer) return;
    lv_obj_clean(s_agendaContainer);

    if (!s_cachedCalDoc) {
        if (s_pageHintLabel) lv_label_set_text(s_pageHintLabel, "Upcoming Agendas");
        if (s_agendaFooterLabel) lv_label_set_text(s_agendaFooterLabel, "Press BOOT to Sync");
        lv_obj_t* emptyLbl = lv_label_create(s_agendaContainer);
        lv_obj_set_style_text_font(emptyLbl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_align(emptyLbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(emptyLbl, 170);
        lv_obj_align(emptyLbl, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(emptyLbl, "Awaiting Calendar Data...");
        return;
    }

    JsonArrayConst events = (*s_cachedCalDoc)["events"].as<JsonArrayConst>();
    int total = events ? (int)events.size() : 0;

    if (total == 0) {
        if (s_pageHintLabel) lv_label_set_text(s_pageHintLabel, "Upcoming Agendas");
        if (s_agendaFooterLabel) lv_label_set_text(s_agendaFooterLabel, "All caught up");
        lv_obj_t* emptyLbl = lv_label_create(s_agendaContainer);
        lv_obj_set_style_text_font(emptyLbl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_align(emptyLbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(emptyLbl, 170);
        lv_obj_align(emptyLbl, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(emptyLbl, "No upcoming agendas");
        return;
    }

    if (s_activePage < 0 || s_activePage >= s_pageCount) {
        s_activePage = 0;
    }

    int startIdx = s_pageStarts[s_activePage];
    int endIdx = (s_activePage + 1 < s_pageCount) ? s_pageStarts[s_activePage + 1] : total;

    int curY = 2;
    String lastKey = "";

    for (int i = startIdx; i < endIdx; i++) {
        JsonObjectConst ev = events[i];
        String gKey = build_group_key(ev);

        if (gKey != lastKey) {
            if (i > startIdx) curY += 4;
            String headerText = build_group_header(ev, s_month, s_todayDay);

            lv_obj_t* hdr = lv_label_create(s_agendaContainer);
            lv_obj_set_style_text_font(hdr, &lv_font_montserrat_12, 0);
            lv_obj_set_style_text_color(hdr, lv_color_black(), 0);
            lv_obj_set_pos(hdr, 6, curY);
            lv_label_set_text(hdr, headerText.c_str());

            create_hline(s_agendaContainer, 6, curY + 15, 172);
            curY += 19;
            lastKey = gKey;
        }

        String text = build_event_display_text(ev);
        int glyphs = count_visible_glyphs(text.c_str());
        int lineH = (glyphs > 22) ? 34 : 18;

        // Crisp 4x4 monochrome bullet dot (avoids missing U+2022 in lv_font_prompt_12)
        lv_obj_t* bullet = lv_obj_create(s_agendaContainer);
        lv_obj_set_pos(bullet, 6, curY + 5);
        lv_obj_set_size(bullet, 4, 4);
        lv_obj_set_style_bg_color(bullet, lv_color_black(), 0);
        lv_obj_set_style_border_width(bullet, 0, 0);
        lv_obj_set_style_radius(bullet, 2, 0);
        lv_obj_set_style_pad_all(bullet, 0, 0);
        lv_obj_clear_flag(bullet, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* rowLbl = lv_label_create(s_agendaContainer);
        lv_obj_set_style_text_font(rowLbl, &lv_font_prompt_12, 0);
        lv_obj_set_style_text_color(rowLbl, lv_color_black(), 0);
        lv_obj_set_pos(rowLbl, 14, curY);
        lv_obj_set_size(rowLbl, 164, lineH);
        lv_label_set_long_mode(rowLbl, LV_LABEL_LONG_DOT);
        lv_label_set_text(rowLbl, text.c_str());

        curY += lineH + 3;
    }

    int remaining = total - endIdx;

    if (s_pageHintLabel) {
        if (s_pageCount > 1) {
            char buf[32];
            snprintf(buf, sizeof(buf), "Page %d/%d | BOOT: Next", s_activePage + 1, s_pageCount);
            lv_label_set_text(s_pageHintLabel, buf);
        } else {
            lv_label_set_text(s_pageHintLabel, "Upcoming Agendas");
        }
    }

    if (s_agendaFooterLabel) {
        char fbuf[40];
        if (remaining > 0) {
            snprintf(fbuf, sizeof(fbuf), "+ %d more | BOOT: Next", remaining);
        } else if (s_pageCount > 1) {
            snprintf(fbuf, sizeof(fbuf), "Page %d/%d | BOOT: Top", s_activePage + 1, s_pageCount);
        } else {
            snprintf(fbuf, sizeof(fbuf), "%d upcoming %s", total, (total == 1) ? "event" : "events");
        }
        lv_label_set_text(s_agendaFooterLabel, fbuf);
    }
}

void create_page_calendar(lv_obj_t* parent) {
    // Invalidate static pointers first without freeing cache? destroy_page_calendar() resets all cleanly.
    // Note: UIManager::showPage() calls destroy_page_calendar() on exit and replayCachedData() right after create.
    s_monthTitleLabel   = nullptr;
    s_pageHintLabel     = nullptr;
    s_leftCard          = nullptr;
    s_calGridContainer  = nullptr;
    s_rightCard         = nullptr;
    s_agendaContainer   = nullptr;
    s_agendaFooterLabel = nullptr;

    resolve_fallback_date(s_year, s_month, s_todayDay);
    s_eventDaysMask = 0;
    s_pageCount = 0;
    s_activePage = 0;

    // -------------------------------------------------------------
    // Top Bar (y=4..24): Month Title (left) & Page/Action Hint (right)
    // -------------------------------------------------------------
    s_monthTitleLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_monthTitleLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_monthTitleLabel, lv_color_black(), 0);
    lv_obj_set_pos(s_monthTitleLabel, 12, 4);
    lv_label_set_text(s_monthTitleLabel, "NOV 2026");

    s_pageHintLabel = lv_label_create(parent);
    lv_obj_set_style_text_font(s_pageHintLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_pageHintLabel, lv_color_black(), 0);
    lv_obj_align(s_pageHintLabel, LV_ALIGN_TOP_RIGHT, -12, 6);
    lv_label_set_text(s_pageHintLabel, "Upcoming Agendas");

    // -------------------------------------------------------------
    // Left Card: Monthly Grid (x=10, y=28, w=188, h=238)
    // -------------------------------------------------------------
    s_leftCard = create_card(parent, 10, 28, 188, 238);

    static const char* kWeekdays[7] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
    for (int c = 0; c < 7; c++) {
        lv_obj_t* wLbl = lv_label_create(s_leftCard);
        lv_obj_set_style_text_font(wLbl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(wLbl, lv_color_black(), 0);
        lv_obj_set_style_text_align(wLbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(wLbl, 2 + c * 26, 5);
        lv_obj_set_width(wLbl, 26);
        lv_label_set_text(wLbl, kWeekdays[c]);
    }

    create_hline(s_leftCard, 4, 21, 178);
    create_vline(s_leftCard, 28, 21, 210);

    s_calGridContainer = lv_obj_create(s_leftCard);
    lv_obj_set_pos(s_calGridContainer, 0, 24);
    lv_obj_set_size(s_calGridContainer, 186, 210);
    lv_obj_set_style_bg_opa(s_calGridContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_calGridContainer, 0, 0);
    lv_obj_set_style_pad_all(s_calGridContainer, 0, 0);
    lv_obj_clear_flag(s_calGridContainer, LV_OBJ_FLAG_SCROLLABLE);

    // -------------------------------------------------------------
    // Right Card: Upcoming Agendas List (x=204, y=28, w=186, h=238)
    // -------------------------------------------------------------
    s_rightCard = create_card(parent, 204, 28, 186, 238);

    s_agendaContainer = lv_obj_create(s_rightCard);
    lv_obj_set_pos(s_agendaContainer, 0, 4);
    lv_obj_set_size(s_agendaContainer, 184, 206);
    lv_obj_set_style_bg_opa(s_agendaContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_agendaContainer, 0, 0);
    lv_obj_set_style_pad_all(s_agendaContainer, 0, 0);
    lv_obj_clear_flag(s_agendaContainer, LV_OBJ_FLAG_SCROLLABLE);

    create_hline(s_rightCard, 6, 212, 172);

    s_agendaFooterLabel = lv_label_create(s_rightCard);
    lv_obj_set_style_text_font(s_agendaFooterLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_agendaFooterLabel, lv_color_black(), 0);
    lv_obj_set_style_text_align(s_agendaFooterLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(s_agendaFooterLabel, 4, 216);
    lv_obj_set_width(s_agendaFooterLabel, 176);
    lv_label_set_text(s_agendaFooterLabel, "Press BOOT to Sync");

    // Render initial offline RTC month grid immediately
    render_calendar_grid();
    render_agenda_page();
}

void update_page_calendar(JsonVariantConst data) {
    if (data.isNull()) return;

    if (s_cachedCalDoc) {
        delete s_cachedCalDoc;
        s_cachedCalDoc = nullptr;
    }
    s_cachedCalDoc = new DynamicJsonDocument(8192);
    s_cachedCalDoc->set(data);

    // Resolve date: prefer backend year/month/today_day, fallback to RTC
    int fbYear, fbMonth, fbDay;
    resolve_fallback_date(fbYear, fbMonth, fbDay);
    int y = data["year"] | fbYear;
    int m = data["month"] | fbMonth;
    int d = data["today_day"] | fbDay;
    if (y >= 2024 && m >= 1 && m <= 12 && d >= 1 && d <= 31) {
        s_year = y;
        s_month = m;
        s_todayDay = d;
    } else {
        s_year = fbYear;
        s_month = fbMonth;
        s_todayDay = fbDay;
    }

    // Build bitmask of current-month event days (>= today)
    s_eventDaysMask = 0;
    JsonArrayConst daysArr = data["month_days_with_events"].as<JsonArrayConst>();
    if (daysArr) {
        for (JsonVariantConst dv : daysArr) {
            int dayNum = dv | 0;
            if (dayNum >= s_todayDay && dayNum <= 31) {
                s_eventDaysMask |= (1UL << dayNum);
            }
        }
    }

    JsonArrayConst events = (*s_cachedCalDoc)["events"].as<JsonArrayConst>();
    compute_agenda_pages(events);
    s_activePage = 0;

    render_calendar_grid();
    render_agenda_page();
}

void cycle_calendar_page() {
    if (s_pageCount > 1) {
        s_activePage = (s_activePage + 1) % s_pageCount;
        render_agenda_page();
    }
}

void destroy_page_calendar() {
    if (s_cachedCalDoc) {
        delete s_cachedCalDoc;
        s_cachedCalDoc = nullptr;
    }
    s_monthTitleLabel   = nullptr;
    s_pageHintLabel     = nullptr;
    s_leftCard          = nullptr;
    s_calGridContainer  = nullptr;
    s_rightCard         = nullptr;
    s_agendaContainer   = nullptr;
    s_agendaFooterLabel = nullptr;
    s_pageCount         = 0;
    s_activePage        = 0;
}
