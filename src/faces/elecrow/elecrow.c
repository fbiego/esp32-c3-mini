

#include "elecrow.h"

lv_obj_t *face_elecrow;

lv_obj_t *face_elecrow_bg;
lv_obj_t *face_elecrow_logo;
lv_obj_t *face_elecrow_am_pm;
lv_obj_t *face_elecrow_hour;
lv_obj_t *face_elecrow_minute;
lv_obj_t *face_elecrow_second;

bool type;

void onClick(lv_event_t *e)
{
    #ifdef ENABLE_FACE_ELECROW

    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != face_elecrow){
        return;
    }

    if (type){
        lv_img_set_src(face_elecrow_bg, &ui_img_204151226);

        lv_img_set_src(face_elecrow_second, &ui_img_1726273370);
        lv_obj_set_y(face_elecrow_second, -50);
        lv_img_set_pivot(face_elecrow_second, 5, 120);

        lv_img_set_src(face_elecrow_minute, &ui_img_1304719198);
        lv_obj_set_y(face_elecrow_minute, -48);
        lv_img_set_pivot(face_elecrow_minute, 8, 101);

        lv_img_set_src(face_elecrow_hour, &ui_img_1608590414);
        lv_obj_set_y(face_elecrow_hour, -30);
        lv_img_set_pivot(face_elecrow_hour, 7, 65);
    } else {

        lv_img_set_src(face_elecrow_bg, &ui_img_1530809127);

        lv_img_set_src(face_elecrow_second, &ui_img_39728541);
        lv_obj_set_y(face_elecrow_second, -33);
        lv_img_set_pivot(face_elecrow_second, 4, 119);

        lv_img_set_src(face_elecrow_minute, &ui_img_327242551);
        lv_obj_set_y(face_elecrow_minute, -33);
        lv_img_set_pivot(face_elecrow_minute, 3, 95);

        lv_img_set_src(face_elecrow_hour, &ui_img_146052295);
        lv_obj_set_y(face_elecrow_hour, -36);
        lv_img_set_pivot(face_elecrow_hour, 7, 76);
    }

    type = !type;

    #endif
}

void init_face_elecrow(void (*callback)(const char *, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **))
{
#ifdef ENABLE_FACE_ELECROW

    face_elecrow = lv_obj_create(NULL);
    lv_obj_clear_flag(face_elecrow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(face_elecrow, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_elecrow, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_elecrow, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(face_elecrow, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(face_elecrow, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(face_elecrow, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(face_elecrow, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_obj_add_event_cb(face_elecrow, onFaceEvent, LV_EVENT_ALL, NULL);
    

    face_elecrow_bg = lv_img_create(face_elecrow);
    lv_img_set_src(face_elecrow_bg, &ui_img_204151226);
    lv_obj_set_width(face_elecrow_bg, lv_pct(100));
    lv_obj_set_height(face_elecrow_bg, lv_pct(100));
    lv_obj_set_align(face_elecrow_bg, LV_ALIGN_CENTER);
    lv_obj_add_flag(face_elecrow_bg, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(face_elecrow_bg, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    face_elecrow_logo = lv_img_create(face_elecrow);
    lv_img_set_src(face_elecrow_logo, &ui_img_81598846);
    lv_obj_set_width( face_elecrow_logo, 100);
    lv_obj_set_height( face_elecrow_logo, 20);
    lv_obj_set_x( face_elecrow_logo, 0 );
    lv_obj_set_y( face_elecrow_logo, 42 );
    lv_obj_set_align( face_elecrow_logo, LV_ALIGN_CENTER );
    lv_obj_add_flag( face_elecrow_logo, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
    lv_obj_add_flag( face_elecrow_logo, LV_OBJ_FLAG_CLICKABLE );   /// Flags
    lv_obj_clear_flag( face_elecrow_logo, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_obj_add_event_cb(face_elecrow_logo, onClick, LV_EVENT_CLICKED, NULL);



    face_elecrow_am_pm = lv_label_create(face_elecrow);
    lv_obj_set_width( face_elecrow_am_pm, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( face_elecrow_am_pm, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_x( face_elecrow_am_pm, 40 );
    lv_obj_set_y( face_elecrow_am_pm, -40 );
    lv_obj_set_align( face_elecrow_am_pm, LV_ALIGN_CENTER );
    lv_label_set_text(face_elecrow_am_pm,"AM");
    lv_obj_set_style_text_color(face_elecrow_am_pm, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(face_elecrow_am_pm, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    face_elecrow_hour = lv_img_create(face_elecrow);
    lv_img_set_src(face_elecrow_hour, &ui_img_1608590414);
    lv_obj_set_width(face_elecrow_hour, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(face_elecrow_hour, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(face_elecrow_hour, 0);
    lv_obj_set_y(face_elecrow_hour, -30);
    lv_obj_set_align(face_elecrow_hour, LV_ALIGN_CENTER);
    lv_obj_add_flag(face_elecrow_hour, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(face_elecrow_hour, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_img_set_pivot(face_elecrow_hour,7,65);//8, 64

    face_elecrow_minute = lv_img_create(face_elecrow);
    lv_img_set_src(face_elecrow_minute, &ui_img_1304719198);
    lv_obj_set_width(face_elecrow_minute, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(face_elecrow_minute, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(face_elecrow_minute, 0);
    lv_obj_set_y(face_elecrow_minute, -48);//
    lv_obj_set_align(face_elecrow_minute, LV_ALIGN_CENTER);
    lv_obj_add_flag(face_elecrow_minute, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(face_elecrow_minute, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_img_set_pivot(face_elecrow_minute,8,101);//8, 100


    face_elecrow_second = lv_img_create(face_elecrow);
    lv_img_set_src(face_elecrow_second, &ui_img_1726273370);
    lv_obj_set_width(face_elecrow_second, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(face_elecrow_second, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(face_elecrow_second, 0);
    lv_obj_set_y(face_elecrow_second, -50);
    lv_obj_set_align(face_elecrow_second, LV_ALIGN_CENTER);
    lv_obj_add_flag(face_elecrow_second, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(face_elecrow_second, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_img_set_pivot(face_elecrow_second, 5,120 );//119

    callback("Elecrow", &ui_img_elecrow_png, &face_elecrow, &face_elecrow_second);


#endif
}
void update_time_elecrow(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday)
{
#ifdef ENABLE_FACE_ELECROW
    if (!face_elecrow)
    {
        return;
    }

    lv_img_set_angle(face_elecrow_hour, hour * 300 + (minute * 5) + (second * (5 / 60)));
	lv_img_set_angle(face_elecrow_minute, (minute * 60) + second);
    lv_label_set_text(face_elecrow_am_pm, am ? "AM" : "PM");

#endif
}
void update_weather_elecrow(int temp, int icon)
{
#ifdef ENABLE_FACE_ELECROW
    if (!face_elecrow)
    {
        return;
    }

#endif
}
void update_status_elecrow(int battery, bool connection)
{
#ifdef ENABLE_FACE_ELECROW
    if (!face_elecrow)
    {
        return;
    }

#endif
}
void update_activity_elecrow(int steps, int distance, int kcal)
{
#ifdef ENABLE_FACE_ELECROW
    if (!face_elecrow)
    {
        return;
    }

#endif
}
void update_health_elecrow(int bpm, int oxygen)
{
#ifdef ENABLE_FACE_ELECROW
    if (!face_elecrow)
    {
        return;
    }

#endif
}
void update_all_elecrow(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
                        int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_ELECROW
    update_time_elecrow(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_elecrow(temp, icon);
    update_status_elecrow(battery, connection);
    update_activity_elecrow(steps, distance, kcal);
    update_health_elecrow(bpm, oxygen);
#endif
}
void update_check_elecrow(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
                          int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_ELECROW
    if (root != face_elecrow)
    {
        return;
    }
    update_time_elecrow(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_elecrow(temp, icon);
    update_status_elecrow(battery, connection);
    update_activity_elecrow(steps, distance, kcal);
    update_health_elecrow(bpm, oxygen);


#endif
}