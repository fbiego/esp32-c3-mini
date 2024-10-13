
/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/

#ifndef CUSTOM_FACE_H
#define CUSTOM_FACE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

// Struct definitions

typedef struct {
    lv_obj_t *element;
    char* path[20];
} lvs_elem;

typedef struct {
    lvs_elem obj0;
} lvs_obj1;

typedef struct {
    lvs_elem obj0;
    lvs_elem obj1;
} lvs_obj2;

typedef struct {
    lvs_elem obj0;
    lvs_elem obj1;
    lvs_elem obj2;
} lvs_obj3;

typedef struct {
    lvs_elem obj0;
    lvs_elem obj1;
    lvs_elem obj2;
    lvs_elem obj3;
} lvs_obj4;

typedef struct {
    lvs_elem obj0;
    lvs_elem obj1;
    lvs_elem obj2;
    lvs_elem obj3;
    lvs_elem obj4;
} lvs_obj5;

#ifdef ENABLE_CUSTOM_FACE

// Global instances
extern lvs_obj2 c_hour;    // 2 [10] // digital
extern lvs_obj2 c_minute;  // 2 [10]
extern lvs_obj2 c_second;  // 2 [10]

extern lvs_obj1 c_hourA;   // 1 [1] // analog
extern lvs_obj1 c_minuteA; // 1 [1]
extern lvs_obj1 c_secondA; // 1 [1]

extern lvs_obj1 c_am_pm;   // 1 [2]
extern lvs_obj1 c_weekday; // 1 [7]

extern lvs_obj2 c_date;    // 2 [10]
extern lvs_obj2 c_month_digit; // 2 [10]
extern lvs_obj1 c_month;   // 1 [12]
extern lvs_obj4 c_year;    // 4 [10]

extern lvs_obj3 c_battery; // 3 [10]
extern lvs_obj1 c_batteryIC; // 1 [15]

extern lvs_obj5 c_weather; // 5 [10]
extern lvs_obj1 c_weatherIC; // 1 [8]

extern lvs_obj5 c_steps;   // 5 [10]
extern lvs_obj5 c_distance; // 5 [10]
extern lvs_obj3 c_hrm;     // 3 [10]
extern lvs_obj4 c_kcal;    // 4 [10]
extern lvs_obj3 c_sp02;    // 3 [10]


LV_IMG_DECLARE(ui_img_custom_preview_png);    // assets/custom_preview.png

#endif

// Function declarations
void delete_lvc(lv_obj_t **obj_ptr);
void invalidate_all(void);

void add_item(lv_obj_t *root, int id, int x, int y, int pvX, int pvY, const char *image, const char *group[], int group_size);



void init_face_custom(void (*callback)(const char*, const lv_image_dsc_t *, lv_obj_t **));
void update_time_custom(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday);
void update_weather_custom(int temp, int icon);
void update_status_custom(int battery, bool connection);
void update_activity_custom(int steps, int distance, int kcal);
void update_health_custom(int bpm, int oxygen);
void update_all_custom(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
            int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);
void update_check_custom(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
            int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);


#ifdef __cplusplus
}
#endif

#endif // CUSTOM_FACE_H
