
// Hand-authored watchface (not bin2lvgl-generated): a Citizen Eco-Drive "Red
// Arrows" Skyhawk pilot watch, built from a real product photo (background)
// with fresh procedurally-drawn hands and a date window on top - the photo's
// own baked-in hands were inpainted out first so they don't duplicate/clash
// with the live ones. First iteration: dial + hands + date only, none of the
// bezel/UTC-subdial/left-LCD complications are wired to real data yet.
// Watchface: citizen_410

#ifndef _FACE_CITIZEN_410_H
#define _FACE_CITIZEN_410_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "app_hal.h"

//#define ENABLE_FACE_CITIZEN_410 // (Citizen) uncomment to enable or define it elsewhere

#ifdef ENABLE_FACE_CITIZEN_410
    extern lv_obj_t *face_citizen_410;

    LV_IMG_DECLARE(face_citizen_410_dial_img);
    LV_IMG_DECLARE(face_citizen_410_dial_img_preview);
    LV_IMG_DECLARE(face_citizen_410_body_bg);
    LV_IMG_DECLARE(face_citizen_410_hand_hour);
    LV_IMG_DECLARE(face_citizen_410_hand_minute);
    LV_IMG_DECLARE(face_citizen_410_hand_second);
    LV_IMG_DECLARE(face_citizen_410_lcd_bg);
    LV_IMG_DECLARE(face_citizen_410_lcd_bg_left);
    LV_FONT_DECLARE(dseg14_bold_18);
#endif

    void onFaceEvent(lv_event_t * e);

    void init_face_citizen_410(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **));
    void update_time_citizen_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday);
    void update_weather_citizen_410(int temp, int icon);
    void update_status_citizen_410(int battery, bool connection);
    void update_activity_citizen_410(int steps, int distance, int kcal);
    void update_health_citizen_410(int bpm, int oxygen);
    void update_all_citizen_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
                int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);
    void update_check_citizen_410(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
                int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);


#ifdef __cplusplus
}
#endif

#endif
