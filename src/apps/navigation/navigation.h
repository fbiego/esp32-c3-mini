
/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/

#ifndef _NAVIGATION_APP_H
#define _NAVIGATION_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "app_hal.h"

#ifdef ENABLE_APP_NAVIGATION

extern lv_obj_t *ui_navScreen;

LV_IMG_DECLARE( ui_img_arrow_png);   // assets/arrow.png
LV_IMG_DECLARE( ui_img_chronos_logo_png);   // assets/chronos_logo.png

#endif

    void ui_navScreen_screen_init(void (*callback)(const char *, const lv_image_dsc_t *, lv_obj_t **));

    void navigateInfo(const char* text, const char* title, const char *directions);
    void navIconState(bool show);
    void setNavIconPx(uint16_t x, uint16_t y, bool on);
    
    void ui_gameExit(void);

    void onGameOpened(void);
    void onGameClosed(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif