
/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/

#ifndef _GAME_RACING_H
#define _GAME_RACING_H

#ifdef __cplusplus
extern "C"
{
#endif


#include "lvgl.h"
#include "app_hal.h"

//#define ENABLE_GAME_RACING // (Racing) uncomment to enable or define it elsewhere

#ifdef ENABLE_GAME_RACING

    extern lv_obj_t *ui_raceScreen;

    LV_IMG_DECLARE(ui_img_road_png);       // assets\road.png
    LV_IMG_DECLARE(ui_img_car_png);        // assets\car.png
    LV_IMG_DECLARE(ui_img_car_green_png);  // assets\car_green.png
    LV_IMG_DECLARE(ui_img_car_red_png);    // assets\car_red.png
    LV_IMG_DECLARE(ui_img_car_yellow_png); // assets\car_yellow.png

#endif
    void ui_raceScreen_screen_init(void (*callback)(const char*, const lv_image_dsc_t *, lv_obj_t **));
    void ui_raceScreen_screen_loop(void);

    void ui_gameExit(void);
    
    void onGameOpened(void);
    void onGameClosed(void);

    void savePrefInt(const char* key, int value);
    int getPrefInt(const char* key, int def_value);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif