
/**
 * @file sample.c
 * @brief Sample app for C3 Watch
 */

#include "sample.h"

#ifdef ENABLE_APP_SAMPLE

REGISTER_APP("Sample App", &ui_img_sample_png, sample_screen_main, sample_screen_init);

void sample_screen_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        /* Do something before the screen is loaded */
    }
    if (event_code == LV_EVENT_SCREEN_LOADED)
    {
        /* Do something after the screen is loaded */
        /* This is a good place to start animations or timers */
    }
    if (event_code == LV_EVENT_SCREEN_UNLOAD_START)
    {
        /* Do something before the screen is unloaded */
        /* This is a good place to save data or stop timers */
    }
    if (event_code == LV_EVENT_SCREEN_UNLOADED)
    {
        /* Do something after the screen is unloaded */
        /* This is a good place to clean up resources */

        /* Clean and delete screen if needed */
        lv_obj_delete(sample_screen_main);
        sample_screen_main = NULL;
    }

    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_RIGHT)
    {
        ui_app_exit(); /* exit to app list */
        /* Call this function to close the app, you can even use button instead of gesture */
    }
}

void sample_screen_init(void)
{
    /* create the screen */
    sample_screen_main = lv_obj_create(NULL);
    lv_obj_remove_flag(sample_screen_main, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(sample_screen_main, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sample_screen_main, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *label = lv_label_create(sample_screen_main);
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_label_set_text(label, "Hello world!\nSample App");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(sample_screen_main, sample_screen_event_cb, LV_EVENT_ALL, NULL);
}


#endif
