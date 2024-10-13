

#ifndef _FACE_ELECROW_H
#define _FACE_ELECROW_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "app_hal.h"

#ifdef ENABLE_FACE_ELECROW

extern lv_obj_t *face_elecrow;


// LV_IMG_DECLARE(ui_img_1671030567);    // assets\Rainbow ring 240px.png
LV_IMG_DECLARE(ui_img_logo_60px_png);    // assets\logo 60px.png
LV_IMG_DECLARE( ui_img_81598846);   // assets\elecrow_logo_100-20px.png
LV_IMG_DECLARE(ui_img_312354278);    // assets\Rainbow ring 135px.png
LV_IMG_DECLARE(ui_img_1530809127);    // assets\01 dial plate240px.png (blue analog background)
LV_IMG_DECLARE(ui_img_39728541);    // assets\01 Second hand update.png
LV_IMG_DECLARE(ui_img_327242551);    // assets\01 Minute hand updating.png
LV_IMG_DECLARE(ui_img_146052295);    // assets\01 Clockwise update.png
LV_IMG_DECLARE(ui_img_204151226);    // assets\02 dial plate 240px.png
LV_IMG_DECLARE(ui_img_1726273370);    // assets\second hand.png
LV_IMG_DECLARE(ui_img_1304719198);    // assets\minute hand.png
LV_IMG_DECLARE(ui_img_1608590414);    // assets\hour hand.png
// LV_IMG_DECLARE(ui_img_1760865983);    // assets\Round bottom of alarm clock 240px.png
LV_IMG_DECLARE(ui_img_01_png); 
LV_IMG_DECLARE(ui_img_elecrow_png); 

#endif


    void onFaceEvent(lv_event_t * e);

    void onClick(lv_event_t *e);

    void init_face_elecrow(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **));
    void update_time_elecrow(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday);
    void update_weather_elecrow(int temp, int icon);
    void update_status_elecrow(int battery, bool connection);
    void update_activity_elecrow(int steps, int distance, int kcal);
    void update_health_elecrow(int bpm, int oxygen);
    void update_all_elecrow(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
                int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);
    void update_check_elecrow(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
                int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);


#ifdef __cplusplus
}
#endif

#endif