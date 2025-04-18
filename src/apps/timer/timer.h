#ifndef _TIMER_APP_H
#define _TIMER_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "app_hal.h"
#include "../../common/app_manager.h"

#ifdef ENABLE_APP_TIMER

    LV_IMG_DECLARE(ui_img_timer_png);

    void timer_screen_init();

    void onGameOpened(void);
    void onGameClosed(void);

    void ui_app_load(lv_obj_t **screen, void (*screen_init)(void));
    void ui_app_exit(void);

    void showError(const char *title, const char *message);

    void timerEnded(int x);

#endif


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif