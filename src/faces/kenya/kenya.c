
// File generated by bin2lvgl
// developed by fbiego. 
// https://github.com/fbiego
// Watchface: KENYA

#include "kenya.h"

#ifdef ENABLE_FACE_KENYA

lv_obj_t *face_kenya;
lv_obj_t *face_kenya_0_264;
lv_obj_t *face_kenya_1_58372;
lv_obj_t *face_kenya_2_58391;
lv_obj_t *face_kenya_3_58391;
lv_obj_t *face_kenya_4_58391;
lv_obj_t *face_kenya_5_58391;
lv_obj_t *face_kenya_7_60782;
lv_obj_t *face_kenya_8_60782;
lv_obj_t *face_kenya_9_60782;
lv_obj_t *face_kenya_10_60782;
lv_obj_t *face_kenya_12_85153;


#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit for watchfaces"
#endif

const lv_img_dsc_t *face_kenya_dial_img_2_58391_group[] = {
	&face_kenya_dial_img_2_58391_0,
	&face_kenya_dial_img_2_58391_1,
	&face_kenya_dial_img_2_58391_2,
	&face_kenya_dial_img_2_58391_3,
	&face_kenya_dial_img_2_58391_4,
	&face_kenya_dial_img_2_58391_5,
	&face_kenya_dial_img_2_58391_6,
	&face_kenya_dial_img_2_58391_7,
	&face_kenya_dial_img_2_58391_8,
	&face_kenya_dial_img_2_58391_9,
};
const lv_img_dsc_t *face_kenya_dial_img_7_60782_group[] = {
	&face_kenya_dial_img_7_60782_0,
	&face_kenya_dial_img_7_60782_1,
	&face_kenya_dial_img_7_60782_2,
	&face_kenya_dial_img_7_60782_3,
	&face_kenya_dial_img_7_60782_4,
	&face_kenya_dial_img_7_60782_5,
	&face_kenya_dial_img_7_60782_6,
	&face_kenya_dial_img_7_60782_7,
	&face_kenya_dial_img_7_60782_8,
	&face_kenya_dial_img_7_60782_9,
};
const lv_img_dsc_t *face_kenya_dial_img_11_80954_group[] = {
	&face_kenya_dial_img_11_80954_0,
	&face_kenya_dial_img_11_80954_1,
	&face_kenya_dial_img_11_80954_2,
	&face_kenya_dial_img_11_80954_3,
	&face_kenya_dial_img_11_80954_4,
	&face_kenya_dial_img_11_80954_5,
	&face_kenya_dial_img_11_80954_6,
};
const lv_img_dsc_t *face_kenya_dial_img_12_85153_group[] = {
	&face_kenya_dial_img_12_85153_0,
	&face_kenya_dial_img_12_85153_1,
	&face_kenya_dial_img_12_85153_2,
	&face_kenya_dial_img_12_85153_3,
	&face_kenya_dial_img_12_85153_4,
	&face_kenya_dial_img_12_85153_5,
	&face_kenya_dial_img_12_85153_6,
};




#endif

void init_face_kenya(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **)){
#ifdef ENABLE_FACE_KENYA
    face_kenya = lv_obj_create(NULL);
    lv_obj_remove_flag(face_kenya, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(face_kenya, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_kenya, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_kenya, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(face_kenya, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(face_kenya, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(face_kenya, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(face_kenya, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_radius(face_kenya, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(face_kenya, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(face_kenya, onFaceEvent, LV_EVENT_ALL, NULL);

    
    face_kenya_0_264 = lv_image_create(face_kenya);
    lv_image_set_src(face_kenya_0_264, &face_kenya_dial_img_0_264_0);
    lv_obj_set_width(face_kenya_0_264, LV_SIZE_CONTENT);
    lv_obj_set_height(face_kenya_0_264, LV_SIZE_CONTENT);
    lv_obj_set_x(face_kenya_0_264, 0);
    lv_obj_set_y(face_kenya_0_264, 0);
    lv_obj_add_flag(face_kenya_0_264, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_kenya_0_264, LV_OBJ_FLAG_SCROLLABLE );

    face_kenya_1_58372 = lv_image_create(face_kenya);
    lv_image_set_src(face_kenya_1_58372, &face_kenya_dial_img_1_58372_0);
    lv_obj_set_width(face_kenya_1_58372, LV_SIZE_CONTENT);
    lv_obj_set_height(face_kenya_1_58372, LV_SIZE_CONTENT);
    lv_obj_set_x(face_kenya_1_58372, 118);
    lv_obj_set_y(face_kenya_1_58372, 118);
    lv_obj_add_flag(face_kenya_1_58372, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_kenya_1_58372, LV_OBJ_FLAG_SCROLLABLE );

    face_kenya_2_58391 = lv_image_create(face_kenya);
    lv_image_set_src(face_kenya_2_58391, &face_kenya_dial_img_2_58391_0);
    lv_obj_set_width(face_kenya_2_58391, LV_SIZE_CONTENT);
    lv_obj_set_height(face_kenya_2_58391, LV_SIZE_CONTENT);
    lv_obj_set_x(face_kenya_2_58391, 143);
    lv_obj_set_y(face_kenya_2_58391, 200);
    lv_obj_add_flag(face_kenya_2_58391, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_kenya_2_58391, LV_OBJ_FLAG_SCROLLABLE );

    face_kenya_3_58391 = lv_image_create(face_kenya);
    lv_image_set_src(face_kenya_3_58391, &face_kenya_dial_img_2_58391_0);
    lv_obj_set_width(face_kenya_3_58391, LV_SIZE_CONTENT);
    lv_obj_set_height(face_kenya_3_58391, LV_SIZE_CONTENT);
    lv_obj_set_x(face_kenya_3_58391, 131);
    lv_obj_set_y(face_kenya_3_58391, 200);
    lv_obj_add_flag(face_kenya_3_58391, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_kenya_3_58391, LV_OBJ_FLAG_SCROLLABLE );

    face_kenya_4_58391 = lv_image_create(face_kenya);
    lv_image_set_src(face_kenya_4_58391, &face_kenya_dial_img_2_58391_0);
    lv_obj_set_width(face_kenya_4_58391, LV_SIZE_CONTENT);
    lv_obj_set_height(face_kenya_4_58391, LV_SIZE_CONTENT);
    lv_obj_set_x(face_kenya_4_58391, 98);
    lv_obj_set_y(face_kenya_4_58391, 200);
    lv_obj_add_flag(face_kenya_4_58391, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_kenya_4_58391, LV_OBJ_FLAG_SCROLLABLE );

    face_kenya_5_58391 = lv_image_create(face_kenya);
    lv_image_set_src(face_kenya_5_58391, &face_kenya_dial_img_2_58391_0);
    lv_obj_set_width(face_kenya_5_58391, LV_SIZE_CONTENT);
    lv_obj_set_height(face_kenya_5_58391, LV_SIZE_CONTENT);
    lv_obj_set_x(face_kenya_5_58391, 86);
    lv_obj_set_y(face_kenya_5_58391, 200);
    lv_obj_add_flag(face_kenya_5_58391, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_kenya_5_58391, LV_OBJ_FLAG_SCROLLABLE );

    face_kenya_7_60782 = lv_image_create(face_kenya);
    lv_image_set_src(face_kenya_7_60782, &face_kenya_dial_img_7_60782_0);
    lv_obj_set_width(face_kenya_7_60782, LV_SIZE_CONTENT);
    lv_obj_set_height(face_kenya_7_60782, LV_SIZE_CONTENT);
    lv_obj_set_x(face_kenya_7_60782, 47);
    lv_obj_set_y(face_kenya_7_60782, 92);
    lv_obj_add_flag(face_kenya_7_60782, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_kenya_7_60782, LV_OBJ_FLAG_SCROLLABLE );

    face_kenya_8_60782 = lv_image_create(face_kenya);
    lv_image_set_src(face_kenya_8_60782, &face_kenya_dial_img_7_60782_0);
    lv_obj_set_width(face_kenya_8_60782, LV_SIZE_CONTENT);
    lv_obj_set_height(face_kenya_8_60782, LV_SIZE_CONTENT);
    lv_obj_set_x(face_kenya_8_60782, 2);
    lv_obj_set_y(face_kenya_8_60782, 92);
    lv_obj_add_flag(face_kenya_8_60782, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_kenya_8_60782, LV_OBJ_FLAG_SCROLLABLE );

    face_kenya_9_60782 = lv_image_create(face_kenya);
    lv_image_set_src(face_kenya_9_60782, &face_kenya_dial_img_7_60782_0);
    lv_obj_set_width(face_kenya_9_60782, LV_SIZE_CONTENT);
    lv_obj_set_height(face_kenya_9_60782, LV_SIZE_CONTENT);
    lv_obj_set_x(face_kenya_9_60782, 199);
    lv_obj_set_y(face_kenya_9_60782, 92);
    lv_obj_add_flag(face_kenya_9_60782, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_kenya_9_60782, LV_OBJ_FLAG_SCROLLABLE );

    face_kenya_10_60782 = lv_image_create(face_kenya);
    lv_image_set_src(face_kenya_10_60782, &face_kenya_dial_img_7_60782_0);
    lv_obj_set_width(face_kenya_10_60782, LV_SIZE_CONTENT);
    lv_obj_set_height(face_kenya_10_60782, LV_SIZE_CONTENT);
    lv_obj_set_x(face_kenya_10_60782, 154);
    lv_obj_set_y(face_kenya_10_60782, 92);
    lv_obj_add_flag(face_kenya_10_60782, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_kenya_10_60782, LV_OBJ_FLAG_SCROLLABLE );

    face_kenya_12_85153 = lv_image_create(face_kenya);
    lv_image_set_src(face_kenya_12_85153, &face_kenya_dial_img_12_85153_0);
    lv_obj_set_width(face_kenya_12_85153, LV_SIZE_CONTENT);
    lv_obj_set_height(face_kenya_12_85153, LV_SIZE_CONTENT);
    lv_obj_set_x(face_kenya_12_85153, 98);
    lv_obj_set_y(face_kenya_12_85153, 25);
    lv_obj_add_flag(face_kenya_12_85153, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_kenya_12_85153, LV_OBJ_FLAG_SCROLLABLE );


    callback("Kenya", &face_kenya_dial_img_preview_0, &face_kenya, NULL);

#endif
}

void update_time_kenya(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday)
{
#ifdef ENABLE_FACE_KENYA
    if (!face_kenya)
    {
        return;
    }
	lv_image_set_src(face_kenya_2_58391, face_kenya_dial_img_2_58391_group[(month / 1) % 10]);
	lv_image_set_src(face_kenya_3_58391, face_kenya_dial_img_2_58391_group[(month / 10) % 10]);
	lv_image_set_src(face_kenya_4_58391, face_kenya_dial_img_2_58391_group[(day / 1) % 10]);
	lv_image_set_src(face_kenya_5_58391, face_kenya_dial_img_2_58391_group[(day / 10) % 10]);
	lv_image_set_src(face_kenya_7_60782, face_kenya_dial_img_7_60782_group[(hour / 1) % 10]);
	lv_image_set_src(face_kenya_8_60782, face_kenya_dial_img_7_60782_group[(hour / 10) % 10]);
	lv_image_set_src(face_kenya_9_60782, face_kenya_dial_img_7_60782_group[(minute / 1) % 10]);
	lv_image_set_src(face_kenya_10_60782, face_kenya_dial_img_7_60782_group[(minute / 10) % 10]);
	lv_image_set_src(face_kenya_12_85153, face_kenya_dial_img_12_85153_group[((weekday + 6) / 1) % 7]);

#endif
}

void update_weather_kenya(int temp, int icon)
{
#ifdef ENABLE_FACE_KENYA
    if (!face_kenya)
    {
        return;
    }

#endif
}

void update_status_kenya(int battery, bool connection){
#ifdef ENABLE_FACE_KENYA
    if (!face_kenya)
    {
        return;
    }

#endif
}

void update_activity_kenya(int steps, int distance, int kcal)
{
#ifdef ENABLE_FACE_KENYA
    if (!face_kenya)
    {
        return;
    }

#endif
}

void update_health_kenya(int bpm, int oxygen)
{
#ifdef ENABLE_FACE_KENYA
    if (!face_kenya)
    {
        return;
    }

#endif
}

void update_all_kenya(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
    int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_KENYA
    update_time_kenya(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_kenya(temp, icon);
    update_status_kenya(battery, connection);
    update_activity_kenya(steps, distance, kcal);
    update_health_kenya(bpm, oxygen);
#endif
}

void update_check_kenya(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
    int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_KENYA
    if (root != face_kenya)
    {
        return;
    }
    update_time_kenya(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_kenya(temp, icon);
    update_status_kenya(battery, connection);
    update_activity_kenya(steps, distance, kcal);
    update_health_kenya(bpm, oxygen);
#endif
}


