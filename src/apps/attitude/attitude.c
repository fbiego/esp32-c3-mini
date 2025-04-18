
/*
    Original code from
    https://www.dofbot.com/post/1-28-inch-gc9a01-round-lcd-with-esp32-and-lvgl-ui-part3
    https://www.youtube.com/watch?v=lKMbVWMAgMs    
    All rights reserved.
*/

#include "attitude.h"


#ifdef ENABLE_APP_ATTITUDE

REGISTER_APP("Attitude", &ui_img_airport_png, ui_attiudeScreen, ui_attiudeScreen_screen_init);

lv_obj_t *ui_img_dial_bg;
lv_obj_t *ui_img_pitch_scale;
lv_obj_t *ui_img_dum_scale;
lv_obj_t *ui_img_roll_scale;
lv_obj_t *ui_img_pointer;
lv_obj_t *ui_Label_roll;
lv_obj_t *ui_Label_pitch;

#define RAD2DEG 57.2958

imu_data_t d;

lv_timer_t *attiude_timer = NULL;

float pitch, roll;

void attiude_timer_cb(lv_timer_t *timer)
{
    d = get_imu_data();

#ifdef ENABLE_APP_ATTITUDE

    // d.ax += 1.0;

    pitch = atan2(-d.az, sqrt((d.ax *d.ax) + (d.ay * d.ay)));
    roll = atan2(d.ay, sqrt((d.ax * d.ax) + (d.az * d.az)));

    pitch *= RAD2DEG;
    roll *= RAD2DEG;

    update_pitch((int)pitch, (int)roll);
    update_roll((int)roll);

#endif
}

void ui_event_attiudeScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
    }
    if (event_code == LV_EVENT_SCREEN_LOADED)
    {
        onGameOpened();

        attiude_timer = lv_timer_create(attiude_timer_cb, 100, NULL);
        lv_timer_set_repeat_count(attiude_timer, -1);
        attiude_timer_cb(attiude_timer); // Trigger first run
    }
    if (event_code == LV_EVENT_SCREEN_UNLOAD_START)
    {

        if (attiude_timer != NULL)
        {
            lv_timer_delete(attiude_timer);
            attiude_timer = NULL;
        }
    }
    if (event_code == LV_EVENT_SCREEN_UNLOADED)
    {
        onGameClosed();
    }

    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_RIGHT)
    {
        ui_app_exit();
    }
}


void ui_attiudeScreen_screen_init()
{

    ui_attiudeScreen = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_attiudeScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_img_dial_bg = lv_image_create(ui_attiudeScreen);
    lv_image_set_src(ui_img_dial_bg, &ui_img_dial_bg_png);
    lv_obj_set_width(ui_img_dial_bg, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_img_dial_bg, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_img_dial_bg, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_img_dial_bg, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_remove_flag(ui_img_dial_bg, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_img_pitch_scale = lv_image_create(ui_attiudeScreen);
    lv_image_set_src(ui_img_pitch_scale, &ui_img_pitch_scale_png);
    lv_obj_set_width(ui_img_pitch_scale, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_img_pitch_scale, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_img_pitch_scale, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_img_pitch_scale, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_remove_flag(ui_img_pitch_scale, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_img_dum_scale = lv_image_create(ui_attiudeScreen);
    lv_image_set_src(ui_img_dum_scale, &ui_img_roll_scale2_png);
    lv_obj_set_width(ui_img_dum_scale, LV_SIZE_CONTENT);  /// 300
    lv_obj_set_height(ui_img_dum_scale, LV_SIZE_CONTENT); /// 300
    lv_obj_set_align(ui_img_dum_scale, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_img_dum_scale, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_remove_flag(ui_img_dum_scale, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_img_roll_scale = lv_image_create(ui_attiudeScreen);
    lv_image_set_src(ui_img_roll_scale, &ui_img_roll_scale_png);
    lv_obj_set_width(ui_img_roll_scale, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_img_roll_scale, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_img_roll_scale, 0);
    lv_obj_set_y(ui_img_roll_scale, 1);
    lv_obj_set_align(ui_img_roll_scale, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_img_roll_scale, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_remove_flag(ui_img_roll_scale, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_img_pointer = lv_image_create(ui_attiudeScreen);
    lv_image_set_src(ui_img_pointer, &ui_img_pointer_png);
    lv_obj_set_width(ui_img_pointer, LV_SIZE_CONTENT);  /// 8
    lv_obj_set_height(ui_img_pointer, LV_SIZE_CONTENT); /// 7
    lv_obj_set_x(ui_img_pointer, 1);
    lv_obj_set_y(ui_img_pointer, -27);
    lv_obj_set_align(ui_img_pointer, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_img_pointer, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_remove_flag(ui_img_pointer, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_Label_roll = lv_label_create(ui_attiudeScreen);
    lv_obj_set_width(ui_Label_roll, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label_roll, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label_roll, -59);
    lv_obj_set_y(ui_Label_roll, -1);
    lv_obj_set_align(ui_Label_roll, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label_roll, "-90");
    lv_obj_set_style_text_color(ui_Label_roll, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_roll, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_decor(ui_Label_roll, LV_TEXT_DECOR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui_Label_roll, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Label_roll, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Label_roll, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_Label_roll, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_Label_roll, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_Label_roll, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_Label_roll, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Label_pitch = lv_label_create(ui_attiudeScreen);
    lv_obj_set_width(ui_Label_pitch, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label_pitch, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label_pitch, 60);
    lv_obj_set_y(ui_Label_pitch, -1);
    lv_obj_set_align(ui_Label_pitch, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label_pitch, "-20");
    lv_obj_set_style_text_color(ui_Label_pitch, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label_pitch, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_decor(ui_Label_pitch, LV_TEXT_DECOR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui_Label_pitch, &ui_font_Small_Font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Label_pitch, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Label_pitch, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_Label_pitch, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_Label_pitch, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_Label_pitch, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_Label_pitch, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_attiudeScreen, ui_event_attiudeScreen, LV_EVENT_ALL, NULL);


}
#endif


void update_pitch(int pitch_value, int pitch_rotation)
{
#ifdef ENABLE_APP_ATTITUDE
    if (ui_attiudeScreen == NULL)
    {
        return;
    }
    // Calculate the rotation angle in degrees for pitch
    int rotation_angle = pitch_rotation * -1; // Each 100 units corresponds to 10 degrees
    lv_image_set_rotation(ui_img_pitch_scale, rotation_angle * 10); // LVGL uses 0.1 degree units

    // Calculate the Y position
    int y_position = pitch_value * 2 * -1; // Each 10 pixels corresponds to 5 units
    lv_obj_set_y(ui_img_pitch_scale, y_position);

    // Update the pitch label with the current pitch value
    lv_label_set_text_fmt(ui_Label_pitch, "%d", pitch_value);
#endif
}

void update_roll(int roll_value)
{
#ifdef ENABLE_APP_ATTITUDE
    if (ui_attiudeScreen == NULL)
    {
        return;
    }
    // Calculate the rotation angle in degrees
    int rotation_angle = roll_value; // Each unit corresponds to 1 degree
    lv_image_set_rotation(ui_img_roll_scale, rotation_angle * 10); // LVGL uses 0.1 degree units

    // Display positive degree on the right and negative degree on the left
    int display_angle = roll_value * -1; // Invert the roll value for display

    // Update the roll label with the current rotation angle in degrees
    lv_label_set_text_fmt(ui_Label_roll, "%d", display_angle);
#endif
}
