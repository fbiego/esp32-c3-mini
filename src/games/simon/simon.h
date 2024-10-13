

/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/


#ifndef _GAME_SIMON_H
#define _GAME_SIMON_H

#ifdef __cplusplus
extern "C"
{
#endif


#include "lvgl.h"
#include "app_hal.h"

#include "misc/lv_timer_private.h"


#ifdef ENABLE_GAME_SIMON

extern lv_obj_t *ui_simonScreen;

LV_IMG_DECLARE(ui_img_colors_png);   // assets/colors.png

#endif

    void ui_simonScreen_screen_init(void (*callback)(const char*, const lv_image_dsc_t *, lv_obj_t **));

    void start_game();
    void play_sequence(lv_timer_t *timer);
    void flash_panel(int panel_idx);
    void generate_sequence();
    void panel_event_handler(lv_event_t *e);

    void ui_gameExit(void);
    
    void onGameOpened(void);
    void onGameClosed(void);

    void toneOut(int pitch, int duration);

    void savePrefInt(const char* key, int value);
    int getPrefInt(const char* key, int def_value);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif