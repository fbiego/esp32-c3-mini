
/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/


#ifndef _ATTITUDE_APP_H
#define _ATTITUDE_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "app_hal.h"
#include "../../common/api.h"
#include "math.h"
#include "../../common/app_manager.h"

#ifdef ENABLE_APP_ATTITUDE
    LV_IMG_DECLARE(ui_img_airport_png); // assets/airport.png

    LV_IMG_DECLARE(ui_img_dial_bg_png);    // assets/Dial bg.png
    LV_IMG_DECLARE(ui_img_pitch_scale_png);    // assets/Pitch Scale.png
    LV_IMG_DECLARE(ui_img_roll_scale2_png);    // assets/Roll Scale2.png
    LV_IMG_DECLARE(ui_img_roll_scale_png);    // assets/Roll Scale.png
    LV_IMG_DECLARE(ui_img_pointer_png);    // assets/Pointer.png

    void ui_attiudeScreen_screen_init();

    void ui_app_load(lv_obj_t **screen, void (*screen_init)(void));
    void ui_app_exit(void);

    void imu_init();
    imu_data_t get_imu_data();
    void imu_close();

    void onGameOpened(void);
    void onGameClosed(void);

    void onRTWState(bool state);

#endif

    

    void update_roll(int roll_value);
    void update_pitch(int pitch_value, int pitch_rotation);

    

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif