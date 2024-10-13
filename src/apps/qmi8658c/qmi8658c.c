
/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/

#include "qmi8658c.h"

#ifdef ENABLE_APP_QMI8658C
lv_obj_t *ui_imuScreen;
lv_obj_t *ui_imuPanel;
lv_obj_t *ui_imuTitle;
lv_obj_t *ui_imuInfo;
lv_obj_t *ui_accTitle;
lv_obj_t *ui_accXText;
lv_obj_t *ui_accXBar;
lv_obj_t *ui_accYText;
lv_obj_t *ui_accYBar;
lv_obj_t *ui_accZText;
lv_obj_t *ui_accZBar;
lv_obj_t *ui_gyroTitle;
lv_obj_t *ui_gyroXText;
lv_obj_t *ui_gyroXBar;
lv_obj_t *ui_gyroYText;
lv_obj_t *ui_gyroYBar;
lv_obj_t *ui_gyroZText;
lv_obj_t *ui_gyroZBar;
lv_obj_t *ui_tempLabel;
lv_obj_t *ui_rtwPanel;
lv_obj_t *ui_rtwText;
lv_obj_t *ui_rtwSwitch;

bool qmi8658c_active;

void ui_event_imuScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
    }
    if (event_code == LV_EVENT_SCREEN_LOADED)
    {
        qmi8658c_active = true;
    }
    if (event_code == LV_EVENT_SCREEN_UNLOAD_START)
    {
        qmi8658c_active = false;
    }
    if (event_code == LV_EVENT_SCREEN_UNLOADED)
    {
    }

    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        ui_gameExit();
    }
}

// void ui_event_rtwSwitch(lv_event_t * e)
// {
//     lv_event_code_t event_code = lv_event_get_code(e);
//     lv_obj_t * target = lv_event_get_target(e);
//     if(event_code == LV_EVENT_VALUE_CHANGED) {
//         onRTWState(lv_obj_has_state(target, LV_STATE_CHECKED));
//     }
// }

#endif

void ui_imuScreen_screen_init(void (*callback)(const char *, const lv_image_dsc_t *, lv_obj_t **))
{
#ifdef ENABLE_APP_QMI8658C
    ui_imuScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_imuScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_imuScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_imuScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_imuPanel = lv_obj_create(ui_imuScreen);
    lv_obj_set_width(ui_imuPanel, lv_pct(100));
    lv_obj_set_height(ui_imuPanel, lv_pct(100));
    lv_obj_set_align(ui_imuPanel, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_imuPanel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_imuPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(ui_imuPanel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(ui_imuPanel, LV_DIR_VER);
    lv_obj_add_flag(ui_imuPanel, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_radius(ui_imuPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_imuPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_imuPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_imuPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_imuPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_imuPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_imuPanel, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_imuPanel, 50, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_imuTitle = lv_label_create(ui_imuPanel);
    lv_obj_set_width(ui_imuTitle, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_imuTitle, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_imuTitle, LV_ALIGN_CENTER);
    lv_label_set_text(ui_imuTitle, "QMI8658C");
    lv_obj_set_style_text_font(ui_imuTitle, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_imuTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_imuTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_imuTitle, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_imuTitle, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_imuInfo = lv_label_create(ui_imuPanel);
    lv_obj_set_width(ui_imuInfo, 200);
    lv_obj_set_height(ui_imuInfo, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_imuInfo, LV_ALIGN_CENTER);

    ui_accTitle = lv_label_create(ui_imuPanel);
    lv_obj_set_width(ui_accTitle, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_accTitle, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_accTitle, LV_ALIGN_CENTER);
    lv_label_set_text(ui_accTitle, "Raw Accelerometer");

    ui_accXText = lv_label_create(ui_imuPanel);
    lv_obj_set_width(ui_accXText, 200);
    lv_obj_set_height(ui_accXText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_accXText, LV_ALIGN_CENTER);

    ui_accXBar = lv_bar_create(ui_imuPanel);
    lv_bar_set_value(ui_accXBar, 25, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_accXBar, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_accXBar, 200);
    lv_obj_set_height(ui_accXBar, 10);
    lv_obj_set_align(ui_accXBar, LV_ALIGN_CENTER);

    ui_accYText = lv_label_create(ui_imuPanel);
    lv_obj_set_width(ui_accYText, 200);
    lv_obj_set_height(ui_accYText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_accYText, LV_ALIGN_CENTER);

    ui_accYBar = lv_bar_create(ui_imuPanel);
    lv_bar_set_value(ui_accYBar, 25, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_accYBar, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_accYBar, 200);
    lv_obj_set_height(ui_accYBar, 10);
    lv_obj_set_align(ui_accYBar, LV_ALIGN_CENTER);

    ui_accZText = lv_label_create(ui_imuPanel);
    lv_obj_set_width(ui_accZText, 200);
    lv_obj_set_height(ui_accZText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_accZText, LV_ALIGN_CENTER);

    ui_accZBar = lv_bar_create(ui_imuPanel);
    lv_bar_set_value(ui_accZBar, 25, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_accZBar, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_accZBar, 200);
    lv_obj_set_height(ui_accZBar, 10);
    lv_obj_set_align(ui_accZBar, LV_ALIGN_CENTER);

    ui_gyroTitle = lv_label_create(ui_imuPanel);
    lv_obj_set_width(ui_gyroTitle, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_gyroTitle, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_gyroTitle, LV_ALIGN_CENTER);
    lv_label_set_text(ui_gyroTitle, "Raw Gyroscope");

    ui_gyroXText = lv_label_create(ui_imuPanel);
    lv_obj_set_width(ui_gyroXText, 200);
    lv_obj_set_height(ui_gyroXText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_gyroXText, LV_ALIGN_CENTER);

    ui_gyroXBar = lv_bar_create(ui_imuPanel);
    lv_bar_set_value(ui_gyroXBar, 50, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_gyroXBar, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_gyroXBar, 200);
    lv_obj_set_height(ui_gyroXBar, 10);
    lv_obj_set_align(ui_gyroXBar, LV_ALIGN_CENTER);

    ui_gyroYText = lv_label_create(ui_imuPanel);
    lv_obj_set_width(ui_gyroYText, 200);
    lv_obj_set_height(ui_gyroYText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_gyroYText, LV_ALIGN_CENTER);

    ui_gyroYBar = lv_bar_create(ui_imuPanel);
    lv_bar_set_value(ui_gyroYBar, 50, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_gyroYBar, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_gyroYBar, 200);
    lv_obj_set_height(ui_gyroYBar, 10);
    lv_obj_set_align(ui_gyroYBar, LV_ALIGN_CENTER);

    ui_gyroZText = lv_label_create(ui_imuPanel);
    lv_obj_set_width(ui_gyroZText, 200);
    lv_obj_set_height(ui_gyroZText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_gyroZText, LV_ALIGN_CENTER);

    ui_gyroZBar = lv_bar_create(ui_imuPanel);
    lv_bar_set_value(ui_gyroZBar, 50, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_gyroZBar, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_gyroZBar, 200);
    lv_obj_set_height(ui_gyroZBar, 10);
    lv_obj_set_align(ui_gyroZBar, LV_ALIGN_CENTER);

    ui_tempLabel = lv_label_create(ui_imuPanel);
    lv_obj_set_width(ui_tempLabel, 200);
    lv_obj_set_height(ui_tempLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_tempLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_tempLabel, "Temp: 35C");

    // ui_rtwPanel = lv_obj_create(ui_imuPanel);
    // lv_obj_set_width(ui_rtwPanel, 220);
    // lv_obj_set_height(ui_rtwPanel, LV_SIZE_CONTENT); /// 50
    // lv_obj_set_align(ui_rtwPanel, LV_ALIGN_CENTER);
    // lv_obj_set_flex_flow(ui_rtwPanel, LV_FLEX_FLOW_ROW);
    // lv_obj_set_flex_align(ui_rtwPanel, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // lv_obj_clear_flag(ui_rtwPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    // lv_obj_set_style_radius(ui_rtwPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_color(ui_rtwPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_rtwPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_color(ui_rtwPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_opa(ui_rtwPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_width(ui_rtwPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_side(ui_rtwPanel, LV_BORDER_SIDE_TOP, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_rtwText = lv_label_create(ui_rtwPanel);
    // lv_obj_set_width(ui_rtwText, LV_SIZE_CONTENT);  /// 1
    // lv_obj_set_height(ui_rtwText, LV_SIZE_CONTENT); /// 1
    // lv_obj_set_align(ui_rtwText, LV_ALIGN_CENTER);
    // lv_label_set_text(ui_rtwText, "Raise to Wake");
    // lv_obj_set_style_text_font(ui_rtwText, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_rtwSwitch = lv_switch_create(ui_rtwPanel);
    // lv_obj_set_width(ui_rtwSwitch, 50);
    // lv_obj_set_height(ui_rtwSwitch, 25);
    // lv_obj_set_align(ui_rtwSwitch, LV_ALIGN_CENTER);

    lv_obj_add_event_cb(ui_imuScreen, ui_event_imuScreen, LV_EVENT_ALL, NULL);
    // lv_obj_add_event_cb(ui_rtwSwitch, ui_event_rtwSwitch, LV_EVENT_ALL, NULL);

    callback("QMI8658C", &ui_img_gyro_l_png, &ui_imuScreen);

#endif
}

void ui_imu_set_info(bool status, uint8_t id, uint8_t rev)
{
#ifdef ENABLE_APP_QMI8658C
    if (status)
    {
        lv_label_set_text_fmt(ui_imuInfo, "ID: 0x%02X\nRevision: 0x%02X", id, rev);
    }
    else
    {
        lv_label_set_text(ui_imuInfo, "Failed to init IMU");
        lv_obj_add_flag(ui_imuPanel, LV_OBJ_FLAG_HIDDEN);
    }

#endif
}

void ui_imu_update_acc(float x, float y, float z)
{
#ifdef ENABLE_APP_QMI8658C
    lv_label_set_text_fmt(ui_accXText, "X: %.3f", x);
    lv_label_set_text_fmt(ui_accYText, "Y: %.3f", y);
    lv_label_set_text_fmt(ui_accZText, "Z: %.3f", z);

    lv_bar_set_value(ui_accXBar, lv_map((x * 100), -100, 100, 0, 100), LV_ANIM_OFF);
    lv_bar_set_value(ui_accYBar, lv_map((y * 100), -100, 100, 0, 100), LV_ANIM_OFF);
    lv_bar_set_value(ui_accZBar, lv_map((z * 100), -100, 100, 0, 100), LV_ANIM_OFF);
#endif
}

void ui_imu_update_gyro(float x, float y, float z)
{
#ifdef ENABLE_APP_QMI8658C
    lv_label_set_text_fmt(ui_gyroXText, "X: %.3f", x);
    lv_label_set_text_fmt(ui_gyroYText, "Y: %.3f", y);
    lv_label_set_text_fmt(ui_gyroZText, "Z: %.3f", z);

    // lv_bar_set_value(ui_gyroXBar, lv_map((x * 100), -100, 100, 0, 100), LV_ANIM_OFF);
    // lv_bar_set_value(ui_gyroYBar, lv_map((y * 100), -100, 100, 0, 100), LV_ANIM_OFF);
    // lv_bar_set_value(ui_gyroZBar, lv_map((z * 100), -100, 100, 0, 100), LV_ANIM_OFF);
#endif
}

void ui_imu_update_temp(float t)
{
#ifdef ENABLE_APP_QMI8658C
    lv_label_set_text_fmt(ui_tempLabel, "Temp: %.2f°C", t);
#endif
}