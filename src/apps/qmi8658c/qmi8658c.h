
/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/


#ifndef _QMI8658C_APP_H
#define _QMI8658C_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "app_hal.h"

#ifdef ENABLE_APP_QMI8658C
    // extern lv_obj_t *ui_imuScreen;
    // extern lv_obj_t *ui_imuPanel;
    // extern lv_obj_t *ui_imuTitle;
    // extern lv_obj_t *ui_accTitle;
    // extern lv_obj_t *ui_accXText;
    // extern lv_obj_t *ui_accXBar;
    // extern lv_obj_t *ui_accYText;
    // extern lv_obj_t *ui_accYBar;
    // extern lv_obj_t *ui_accZText;
    // extern lv_obj_t *ui_accZBar;
    // extern lv_obj_t *ui_gyroTitle;
    // extern lv_obj_t *ui_gyroXText;
    // extern lv_obj_t *ui_gyroXBar;
    // extern lv_obj_t *ui_gyroYText;
    // extern lv_obj_t *ui_gyroYBar;
    // extern lv_obj_t *ui_gyroZText;
    // extern lv_obj_t *ui_gyroZBar;
    // extern lv_obj_t *ui_tempLabel;
    // extern lv_obj_t *ui_rtwPanel;
    // extern lv_obj_t *ui_rtwText;
    // extern lv_obj_t *ui_rtwSwitch;

    extern bool qmi8658c_active;

    LV_IMG_DECLARE(ui_img_gyro_l_png); // assets/gyro_l.png

#endif

    void ui_imuScreen_screen_init(void (*callback)(const char *, const lv_image_dsc_t *, lv_obj_t **));
    void ui_imu_set_info(bool status, uint8_t id, uint8_t rev);
    void ui_imu_update_acc(float x, float y, float z);
    void ui_imu_update_gyro(float x, float y, float z);
    void ui_imu_update_temp(float t);

    void ui_gameExit(void);
    void onRTWState(bool state);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif