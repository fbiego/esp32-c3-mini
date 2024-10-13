

/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.
    For a copy, see <https://opensource.org/licenses/MIT>.
*/

#include "piobot.h"

lv_obj_t *ui_pioScreen;
lv_obj_t *ui_pioPanel;
lv_obj_t *ui_pioEarL;
lv_obj_t *ui_pioEarR;
lv_obj_t *ui_pioFace;
lv_obj_t *ui_pioEyeL;
lv_obj_t *ui_pioEyeR;
lv_obj_t *ui_pioIrisL;
lv_obj_t *ui_pioIrisR;

lv_point_t face_offset;
lv_point_t gaze;
lv_point_t touch;

lv_timer_t *pio_bot_timer = NULL;

void pio_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *img;
    lv_coord_t x_target = lv_rand(-20, 20);
    lv_coord_t y_target = lv_rand(-15, 25);
    int h = lv_rand(5, 70);

    openEyes(h);

    look(lv_rand(-50, 50), lv_rand(-50, 50), h);

    if (y_target % 5 == 0 && h > 50)
    {
        blink(ui_pioEyeR, h);
    }
    if (x_target % 5 == 0 && h > 50)
    {
        blink(ui_pioEyeL, h);
    }

    //  move(x_target, y_target);

    // lv_anim_t anim_x;
    // lv_anim_init(&anim_x);
    // lv_anim_set_var(&anim_x, img);
    // lv_anim_set_values(&anim_x, face_offset.x, x_target);
    // lv_anim_set_time(&anim_x, 1000);
    // lv_anim_set_exec_cb(&anim_x, (lv_anim_exec_xcb_t)moveX);
    // lv_anim_start(&anim_x);

    // lv_anim_t anim_y;
    // lv_anim_init(&anim_y);
    // lv_anim_set_var(&anim_y, img);
    // lv_anim_set_values(&anim_y, face_offset.y, y_target);
    // lv_anim_set_time(&anim_y, 1000);
    // lv_anim_set_exec_cb(&anim_y, (lv_anim_exec_xcb_t)moveY);
    // lv_anim_start(&anim_y);
}

void start_pio_animation()
{
    pio_bot_timer = lv_timer_create(pio_timer_cb, 1500, NULL);
    lv_timer_set_repeat_count(pio_bot_timer, -1);
    pio_timer_cb(pio_bot_timer); // Trigger first run
}

void stop_pio_animation()
{
    if (pio_bot_timer != NULL)
    {
        lv_timer_delete(pio_bot_timer);
        pio_bot_timer = NULL;
    }
}

void ui_event_pioScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_indev_t *indev = lv_indev_active();
    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
    }
    if (event_code == LV_EVENT_SCREEN_LOADED)
    {
        onGameOpened();

        blink(ui_pioEyeL, 70);
        blink(ui_pioEyeR, 70);

        // showError("PIO Bot", "Long press to exit");
    }
    if (event_code == LV_EVENT_SCREEN_UNLOAD_START)
    {
    }
    if (event_code == LV_EVENT_SCREEN_UNLOADED)
    {
        onGameClosed();

        openEyes(0);
        look(0, 0, 5);
        move(0, 0);
        stop_pio_animation();
    }

    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        ui_gameExit();
    }


    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);

    if (actScr != ui_pioScreen){
        return;
    }

    if (event_code == LV_EVENT_PRESSING)
    {
        int vectX = indev->pointer.act_point.x; // - 120; // - touch.x;
        int vectY = indev->pointer.act_point.y; // - 120; // - touch.y;

        // lv_obj_set_x(ui_lvglLogo, vectX);
        // lv_obj_set_y(ui_lvglLogo, vectY);
        openEyes(70);
        look(vectX - 120, vectY - 120, 70);
        // move(vectX - touch.x, vectY - touch.y);
    }

    if (event_code == LV_EVENT_PRESSED)
    {
        touch.x = indev->pointer.act_point.x;
        touch.y = indev->pointer.act_point.y;

        stop_pio_animation();
    }
    if (event_code == LV_EVENT_RELEASED)
    {
        // look(0, 0);
        start_pio_animation();
    }
}

void ui_event_eye_touched(lv_event_t *e)
{

     lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
    }
    if (event_code == LV_EVENT_RELEASED)
    {
        lv_obj_set_height(target, 70);
    }
    if (event_code == LV_EVENT_CLICKED)
    {
        lv_obj_set_height(target, 5);
    }

}

void ui_pioScreen_screen_init(void (*callback)(const char *, const lv_image_dsc_t *, lv_obj_t **))
{

    ui_pioScreen = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_pioScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_pioScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_pioScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_pioPanel = lv_obj_create(ui_pioScreen);
    lv_obj_set_width(ui_pioPanel, 240);
    lv_obj_set_height(ui_pioPanel, 240);
    lv_obj_set_align(ui_pioPanel, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_pioPanel, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE); /// Flags
    lv_obj_set_style_radius(ui_pioPanel, 120, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_pioPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_pioPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_pioPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_pioPanel, lv_color_hex(0xFF7F00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_pioPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_pioPanel, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_pioPanel, -6, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_pioEarL = lv_image_create(ui_pioScreen);
    lv_image_set_src(ui_pioEarL, &ui_img_pio_ear_l_png);
    lv_obj_set_width(ui_pioEarL, 30);
    lv_obj_set_height(ui_pioEarL, 40);
    lv_obj_set_x(ui_pioEarL, -35);
    lv_obj_set_y(ui_pioEarL, -70);
    lv_obj_set_align(ui_pioEarL, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_pioEarL, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_image_set_pivot(ui_pioEarL, 25, 50);

    ui_pioEarR = lv_image_create(ui_pioScreen);
    lv_image_set_src(ui_pioEarR, &ui_img_pio_ear_r_png);
    lv_obj_set_width(ui_pioEarR, 30);
    lv_obj_set_height(ui_pioEarR, 40);
    lv_obj_set_x(ui_pioEarR, 35);
    lv_obj_set_y(ui_pioEarR, -70);
    lv_obj_set_align(ui_pioEarR, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_pioEarR, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_image_set_pivot(ui_pioEarR, 5, 50);

    ui_pioFace = lv_image_create(ui_pioScreen);
    lv_image_set_src(ui_pioFace, &ui_img_pio_face_png);
    lv_obj_set_width(ui_pioFace, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_pioFace, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_pioFace, 0);
    lv_obj_set_y(ui_pioFace, 10);
    lv_obj_set_align(ui_pioFace, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_pioFace, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_pioEyeL = lv_image_create(ui_pioScreen);
    lv_image_set_src(ui_pioEyeL, &ui_img_pio_eye_l_png);
    lv_obj_set_width(ui_pioEyeL, 60);
    lv_obj_set_height(ui_pioEyeL, 70);
    lv_obj_set_x(ui_pioEyeL, -32);
    lv_obj_set_y(ui_pioEyeL, 3);
    lv_obj_set_align(ui_pioEyeL, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_pioEyeL, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_image_set_inner_align(ui_pioEyeL, LV_IMAGE_ALIGN_CENTER);

    ui_pioEyeR = lv_image_create(ui_pioScreen);
    lv_image_set_src(ui_pioEyeR, &ui_img_pio_eye_r_png);
    lv_obj_set_width(ui_pioEyeR, 60);
    lv_obj_set_height(ui_pioEyeR, 70);
    lv_obj_set_x(ui_pioEyeR, 32);
    lv_obj_set_y(ui_pioEyeR, 3);
    lv_obj_set_align(ui_pioEyeR, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_pioEyeR, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_image_set_inner_align(ui_pioEyeR, LV_IMAGE_ALIGN_CENTER);

    ui_pioIrisL = lv_obj_create(ui_pioScreen);
    lv_obj_set_width(ui_pioIrisL, 20);
    lv_obj_set_height(ui_pioIrisL, 20);
    lv_obj_set_x(ui_pioIrisL, -32);
    lv_obj_set_y(ui_pioIrisL, 6);
    lv_obj_set_align(ui_pioIrisL, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_pioIrisL, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_pioIrisL, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_pioIrisL, lv_color_hex(0xFF7D00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_pioIrisL, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_pioIrisL, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_pioIrisR = lv_obj_create(ui_pioScreen);
    lv_obj_set_width(ui_pioIrisR, 20);
    lv_obj_set_height(ui_pioIrisR, 20);
    lv_obj_set_x(ui_pioIrisR, 32);
    lv_obj_set_y(ui_pioIrisR, 6);
    lv_obj_set_align(ui_pioIrisR, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_pioIrisR, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_pioIrisR, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_pioIrisR, lv_color_hex(0xE67100), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_pioIrisR, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_pioIrisR, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_pioScreen, ui_event_pioScreen, LV_EVENT_ALL, NULL);
    // lv_obj_add_event_cb(ui_pioEyeL, ui_event_eye_touched, LV_EVENT_CLICKED | LV_EVENT_PRESSED | LV_EVENT_RELEASED, NULL);
    // lv_obj_add_event_cb(ui_pioEyeR, ui_event_eye_touched, LV_EVENT_CLICKED | LV_EVENT_PRESSED | LV_EVENT_RELEASED, NULL);

    callback("PIO Bot", &ui_img_pio_logo_png, &ui_pioScreen);
}

void blink(lv_obj_t *eye, int end)
{
    lv_anim_t anim_x;
    lv_anim_init(&anim_x);
    lv_anim_set_var(&anim_x, eye);
    lv_anim_set_values(&anim_x, end, 0);
    lv_anim_set_time(&anim_x, 200);
    lv_anim_set_playback_delay(&anim_x, 0);
    lv_anim_set_playback_time(&anim_x, 200);
    lv_anim_set_exec_cb(&anim_x, (lv_anim_exec_xcb_t)lv_obj_set_height);
    lv_anim_start(&anim_x);
}

void openEyes(int h)
{
    lv_obj_set_height(ui_pioEyeL, h);
    lv_obj_set_height(ui_pioEyeR, h);
}

void look(int x, int y, int h)
{
    int mY = lv_map(h, 0, 70, 0, 20);
    gaze.x = LV_CLAMP(-15, x, 15);
    gaze.y = LV_CLAMP(-1 * mY, y, mY);
    lv_obj_set_x(ui_pioIrisL, -32 + gaze.x + face_offset.x);
    lv_obj_set_y(ui_pioIrisL, 6 + gaze.y + face_offset.y);

    lv_obj_set_x(ui_pioIrisR, 32 + gaze.x + face_offset.x);
    lv_obj_set_y(ui_pioIrisR, 6 + gaze.y + face_offset.y);

    lv_image_set_rotation(ui_pioEarL, lv_map(gaze.x, -15, 15, 130, -130));
    lv_image_set_rotation(ui_pioEarR, lv_map(gaze.x, -15, 15, 130, -130));
}

void move(int x, int y)
{
    face_offset.x = LV_CLAMP(-20, x, 20);
    face_offset.y = LV_CLAMP(-15, y, 25);

    lv_obj_set_x(ui_pioEarL, -35 + face_offset.x);
    lv_obj_set_y(ui_pioEarL, -70 + face_offset.y);
    lv_obj_set_x(ui_pioEarR, 35 + face_offset.x);
    lv_obj_set_y(ui_pioEarR, -70 + face_offset.y);

    lv_obj_set_x(ui_pioFace, 0 + face_offset.x);
    lv_obj_set_y(ui_pioFace, 10 + face_offset.y);

    lv_obj_set_x(ui_pioEyeL, -32 + face_offset.x);
    lv_obj_set_y(ui_pioEyeL, 3 + face_offset.y);

    lv_obj_set_x(ui_pioEyeR, 32 + face_offset.x);
    lv_obj_set_y(ui_pioEyeR, 3 + face_offset.y);

    lv_obj_set_x(ui_pioIrisL, -32 + face_offset.x + gaze.x);
    lv_obj_set_y(ui_pioIrisL, 6 + face_offset.y + gaze.y);

    lv_obj_set_x(ui_pioIrisR, 32 + face_offset.x + gaze.x);
    lv_obj_set_y(ui_pioIrisR, 6 + face_offset.y + gaze.y);
}

void moveX(lv_obj_t *obj, int32_t x)
{
    face_offset.x = LV_CLAMP(-20, x, 20);

    lv_obj_set_x(ui_pioEarL, -35 + face_offset.x);
    lv_obj_set_x(ui_pioEarR, 35 + face_offset.x);

    lv_obj_set_x(ui_pioFace, 0 + face_offset.x);

    lv_obj_set_x(ui_pioEyeL, -32 + face_offset.x);

    lv_obj_set_x(ui_pioEyeR, 32 + face_offset.x);

    lv_obj_set_x(ui_pioIrisL, -32 + face_offset.x + gaze.x);

    lv_obj_set_x(ui_pioIrisR, 32 + face_offset.x + gaze.x);
}

void moveY(lv_obj_t *obj, int32_t y)
{
    face_offset.y = LV_CLAMP(-15, y, 25);

    lv_obj_set_y(ui_pioEarL, -70 + face_offset.y);
    lv_obj_set_y(ui_pioEarR, -70 + face_offset.y);

    lv_obj_set_y(ui_pioFace, 10 + face_offset.y);

    lv_obj_set_y(ui_pioEyeL, 3 + face_offset.y);

    lv_obj_set_y(ui_pioEyeR, 3 + face_offset.y);

    lv_obj_set_y(ui_pioIrisL, 6 + face_offset.y + gaze.y);

    lv_obj_set_y(ui_pioIrisR, 6 + face_offset.y + gaze.y);
}
