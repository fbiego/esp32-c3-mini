/**
 * @file sample.h
 * @brief Sample app for C3 Watch
 */

#ifndef _SAMPLE_APP_H
#define _SAMPLE_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "app_hal.h"
#include "../../common/app_manager.h"

// #define ENABLE_APP_SAMPLE // Uncomment or define this to enable the sample app

#ifdef ENABLE_APP_SAMPLE

    LV_IMAGE_DECLARE(ui_img_sample_png);

    void sample_screen_init(void);

    void ui_app_load(lv_obj_t **screen, void (*screen_init)(void));
    void ui_app_exit(void);

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif