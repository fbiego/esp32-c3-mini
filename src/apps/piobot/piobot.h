

/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.
    For a copy, see <https://opensource.org/licenses/MIT>.
*/

#ifndef _PIOBOT_APP_H
#define _PIOBOT_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "app_hal.h"

#include "indev/lv_indev_private.h"

    extern lv_obj_t *ui_pioScreen;

    LV_IMG_DECLARE(ui_img_pio_logo_png);  // assets/pio_logo.png
    LV_IMG_DECLARE(ui_img_pio_ear_l_png); // assets/pio_ear_l.png
    LV_IMG_DECLARE(ui_img_pio_ear_r_png); // assets/pio_ear_r.png
    LV_IMG_DECLARE(ui_img_pio_face_png);  // assets/pio_face.png
    LV_IMG_DECLARE(ui_img_pio_eye_l_png); // assets/pio_eye_l.png
    LV_IMG_DECLARE(ui_img_pio_eye_r_png); // assets/pio_eye_r.png

    void ui_pioScreen_screen_init(void (*callback)(const char *, const lv_image_dsc_t *, lv_obj_t **));
    void ui_event_pioScreen(lv_event_t *e);

    void openEyes(int level);
    void blink(lv_obj_t *eye, int end);
    void look(int x, int y, int h);
    void move(int x, int y);
    void moveX(lv_obj_t *obj, int32_t x);
    void moveY(lv_obj_t *obj, int32_t y);

    void pio_timer_cb(lv_timer_t *timer);

    void start_pio_animation();
    void stop_pio_animation();

    void showError(const char *title, const char *message);

    void ui_gameExit(void);

    void onGameOpened(void);
    void onGameClosed(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
