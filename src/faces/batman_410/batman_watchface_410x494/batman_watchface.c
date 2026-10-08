/**
 * Batman Chronograph Watchface – 410 x 494
 * Designed for ChronosESP32 + LVGL on Waveshare ESP32-S3-Touch-AMOLED-2.06
 *
 * Place the PNG assets in your filesystem (SPIFFS / LittleFS / FFAT)
 * or convert them to lv_img_dsc_t and change the src paths.
 */

#include "batman_watchface.h"
#include <time.h>
#include <stdio.h>

/* Optional: include ChronosESP32 if you want live weather / phone battery */
/* #include "ChronosESP32.h" */
/* extern ChronosESP32 watch; */

#define WF_W 410
#define WF_H 494

static lv_obj_t *bg_img;
static lv_obj_t *hour_hand;
static lv_obj_t *min_hand;
static lv_obj_t *sec_hand;
static lv_obj_t *sub_hands[3];          /* 0 = left (sec), 1 = right (min), 2 = bottom (hour) */

static lv_obj_t *label_day;
static lv_obj_t *label_batt;
static lv_obj_t *label_date;
static lv_obj_t *label_time;
static lv_obj_t *label_weather;

static lv_style_t style_yellow;

static void update_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    batman_watchface_update();
}

void batman_watchface_update(void)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (!t) return;

    /* Main hands – LVGL angle unit is 0.1 degree */
    int16_t hour_a = (t->tm_hour % 12) * 300 + t->tm_min * 5;
    int16_t min_a  = t->tm_min * 60 + t->tm_sec;
    int16_t sec_a  = t->tm_sec * 60;

    if (hour_hand) lv_img_set_angle(hour_hand, hour_a);
    if (min_hand)  lv_img_set_angle(min_hand,  min_a);
    if (sec_hand)  lv_img_set_angle(sec_hand,  sec_a);

    /* Animated subdial hands (decorative chronograph) */
    if (sub_hands[0]) lv_img_set_angle(sub_hands[0], sec_a);                 /* running seconds */
    if (sub_hands[1]) lv_img_set_angle(sub_hands[1], (t->tm_min % 30) * 120); /* 30-min style */
    if (sub_hands[2]) lv_img_set_angle(sub_hands[2], (t->tm_hour % 12) * 300);

    /* Labels */
    static const char *days[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
    char buf[16];

    if (label_day) {
        lv_label_set_text(label_day, days[t->tm_wday]);
    }

    if (label_date) {
        lv_snprintf(buf, sizeof(buf), "%02d", t->tm_mday);
        lv_label_set_text(label_date, buf);
    }

    if (label_time) {
        lv_snprintf(buf, sizeof(buf), "%02d:%02d", t->tm_hour, t->tm_min);
        lv_label_set_text(label_time, buf);
    }

    if (label_batt) {
        /* Replace with real value from Chronos or AXP2101 */
        /* Example: lv_snprintf(buf, sizeof(buf), "%d%%", watch.getPhoneBattery()); */
        lv_label_set_text(label_batt, "78%");
    }

    if (label_weather) {
        /* Example: if (watch.getWeather().available) ... */
        lv_label_set_text(label_weather, "22°C");
    }
}

void batman_watchface_create(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_size(scr, WF_W, WF_H);

    /* ---------- Background ---------- */
    bg_img = lv_img_create(scr);
    /* Change path according to your FS driver (S: SPIFFS, L: LittleFS, etc.) */
    lv_img_set_src(bg_img, "S:batman_bg_410x494.png");
    lv_obj_center(bg_img);

    /* ---------- Yellow style ---------- */
    lv_style_init(&style_yellow);
    lv_style_set_text_color(&style_yellow, lv_color_hex(0xFFD700));
    lv_style_set_text_font(&style_yellow, &lv_font_montserrat_18);

    /* Day – top left integrated area */
    label_day = lv_label_create(scr);
    lv_obj_add_style(label_day, &style_yellow, 0);
    lv_label_set_text(label_day, "FRI");
    lv_obj_align(label_day, LV_ALIGN_TOP_LEFT, 28, 22);

    /* Battery – top right */
    label_batt = lv_label_create(scr);
    lv_obj_add_style(label_batt, &style_yellow, 0);
    lv_label_set_text(label_batt, "78%");
    lv_obj_align(label_batt, LV_ALIGN_TOP_RIGHT, -28, 22);

    /* Digital time */
    label_time = lv_label_create(scr);
    lv_obj_add_style(label_time, &style_yellow, 0);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_28, 0);
    lv_label_set_text(label_time, "14:37");
    lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 105);

    /* Date */
    label_date = lv_label_create(scr);
    lv_obj_add_style(label_date, &style_yellow, 0);
    lv_obj_set_style_text_font(label_date, &lv_font_montserrat_20, 0);
    lv_label_set_text(label_date, "31");
    lv_obj_align(label_date, LV_ALIGN_CENTER, 0, 38);

    /* Weather */
    label_weather = lv_label_create(scr);
    lv_obj_add_style(label_weather, &style_yellow, 0);
    lv_label_set_text(label_weather, "22°C");
    lv_obj_align(label_weather, LV_ALIGN_BOTTOM_RIGHT, -28, -28);

    /* ---------- Main hands ---------- */
    /* Adjust pivot to the rotation center of each hand image */
    hour_hand = lv_img_create(scr);
    lv_img_set_src(hour_hand, "S:hand_hour.png");
    lv_img_set_pivot(hour_hand, 10, 100);   /* center-x, near bottom */
    lv_obj_align(hour_hand, LV_ALIGN_CENTER, 0, 0);

    min_hand = lv_img_create(scr);
    lv_img_set_src(min_hand, "S:hand_min.png");
    lv_img_set_pivot(min_hand, 8, 120);
    lv_obj_align(min_hand, LV_ALIGN_CENTER, 0, 0);

    sec_hand = lv_img_create(scr);
    lv_img_set_src(sec_hand, "S:hand_sec.png");
    lv_img_set_pivot(sec_hand, 4, 145);
    lv_obj_align(sec_hand, LV_ALIGN_CENTER, 0, 0);

    /* ---------- Subdial hands (positions approximate – fine-tune to match bg) ---------- */
    /* Left subdial (~9 o'clock) */
    sub_hands[0] = lv_img_create(scr);
    lv_img_set_src(sub_hands[0], "S:hand_sub.png");
    lv_img_set_pivot(sub_hands[0], 5, 40);
    lv_obj_set_pos(sub_hands[0], 70, 200);   /* tweak X/Y to center on left subdial */

    /* Right subdial (~3 o'clock) */
    sub_hands[1] = lv_img_create(scr);
    lv_img_set_src(sub_hands[1], "S:hand_sub.png");
    lv_img_set_pivot(sub_hands[1], 5, 40);
    lv_obj_set_pos(sub_hands[1], 310, 200);

    /* Bottom subdial (~6 o'clock) */
    sub_hands[2] = lv_img_create(scr);
    lv_img_set_src(sub_hands[2], "S:hand_sub.png");
    lv_img_set_pivot(sub_hands[2], 5, 40);
    lv_obj_set_pos(sub_hands[2], 190, 340);

    /* Update timer – 200 ms for smoother second hand */
    lv_timer_create(update_cb, 200, NULL);

    /* Initial draw */
    batman_watchface_update();
}
