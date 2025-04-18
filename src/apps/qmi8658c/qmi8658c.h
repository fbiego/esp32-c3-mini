
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
#include "../../common/api.h"
#include "../../common/app_manager.h"

#ifdef ENABLE_APP_QMI8658C


    extern bool qmi8658c_active;

    LV_IMG_DECLARE(ui_img_gyro_l_png); // assets/gyro_l.png

    void ui_imuScreen_screen_init();

    void ui_app_load(lv_obj_t **screen, void (*screen_init)(void));
    void ui_app_exit(void);

    void imu_init();
    imu_data_t get_imu_data();
    void imu_close();

    void onGameOpened(void);
    void onGameClosed(void);

    void onRTWState(bool state);

#endif

    
    void ui_imu_set_info(bool status, uint8_t id, uint8_t rev);

    void ui_imu_update_acc(float x, float y, float z);

    // bool get_imu_data(float *ax, float *ay, float *az, float *gx, float *gy, float *gz, float *temp);
    

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif