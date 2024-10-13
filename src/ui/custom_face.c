
/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/

#include "custom_face.h"

#ifdef ENABLE_CUSTOM_FACE
// Global instances
lvs_obj2 c_hour;   // 2 [10] // digital
lvs_obj2 c_minute; // 2 [10]
lvs_obj2 c_second; // 2 [10]

lvs_obj1 c_hourA;   // 1 [1] // analog
lvs_obj1 c_minuteA; // 1 [1]
lvs_obj1 c_secondA; // 1 [1]

lvs_obj1 c_am_pm;   // 1 [2]
lvs_obj1 c_weekday; // 1 [7]

lvs_obj2 c_date;        // 2 [10]
lvs_obj2 c_month_digit; // 2 [10]
lvs_obj1 c_month;       // 1 [12]
lvs_obj4 c_year;        // 4 [10]

lvs_obj3 c_battery;   // 3 [10]
lvs_obj1 c_batteryIC; // 1 [15]

lvs_obj5 c_weather;   // 5 [10]
lvs_obj1 c_weatherIC; // 1 [8]

lvs_obj5 c_steps;    // 5 [10]
lvs_obj5 c_distance; // 5 [10]
lvs_obj3 c_hrm;      // 3 [10]
lvs_obj4 c_kcal;     // 4 [10]
lvs_obj3 c_sp02;     // 3 [10]

int b_second;
int b_minute;
int b_hour;
bool b_mode;
bool b_am;
int b_day;
int b_month;
int b_year;
int b_weekday;

#endif

// Function to delete an object and set its pointer to NULL
void delete_lvc(lv_obj_t **obj_ptr)
{
    if (*obj_ptr)
    {
        lv_obj_del(*obj_ptr);
        *obj_ptr = NULL;
    }
}

// Function to invalidate and delete all objects
void invalidate_all(void)
{
#ifdef ENABLE_CUSTOM_FACE
    // Digital
    delete_lvc(&c_hour.obj0.element);
    delete_lvc(&c_hour.obj1.element);
    delete_lvc(&c_minute.obj0.element);
    delete_lvc(&c_minute.obj1.element);
    delete_lvc(&c_second.obj0.element);
    delete_lvc(&c_second.obj1.element);

    // Analog
    delete_lvc(&c_hourA.obj0.element);
    delete_lvc(&c_minuteA.obj0.element);
    delete_lvc(&c_secondA.obj0.element);

    // AM/PM
    delete_lvc(&c_am_pm.obj0.element);

    // Weekday
    delete_lvc(&c_weekday.obj0.element);

    // Date and Month
    delete_lvc(&c_date.obj0.element);
    delete_lvc(&c_date.obj1.element);
    delete_lvc(&c_month_digit.obj0.element);
    delete_lvc(&c_month_digit.obj1.element);
    delete_lvc(&c_month.obj0.element);

    // Year
    delete_lvc(&c_year.obj0.element);
    delete_lvc(&c_year.obj1.element);
    delete_lvc(&c_year.obj2.element);
    delete_lvc(&c_year.obj3.element);

    // Battery
    delete_lvc(&c_battery.obj0.element);
    delete_lvc(&c_battery.obj1.element);
    delete_lvc(&c_battery.obj2.element);
    delete_lvc(&c_batteryIC.obj0.element);

    // Weather
    delete_lvc(&c_weather.obj0.element);
    delete_lvc(&c_weather.obj1.element);
    delete_lvc(&c_weather.obj2.element);
    delete_lvc(&c_weather.obj3.element);
    delete_lvc(&c_weather.obj4.element);
    delete_lvc(&c_weatherIC.obj0.element);

    // Activity
    delete_lvc(&c_steps.obj0.element);
    delete_lvc(&c_steps.obj1.element);
    delete_lvc(&c_steps.obj2.element);
    delete_lvc(&c_steps.obj3.element);
    delete_lvc(&c_steps.obj4.element);
    delete_lvc(&c_distance.obj0.element);
    delete_lvc(&c_distance.obj1.element);
    delete_lvc(&c_distance.obj2.element);
    delete_lvc(&c_distance.obj3.element);
    delete_lvc(&c_distance.obj4.element);
    delete_lvc(&c_hrm.obj0.element);
    delete_lvc(&c_hrm.obj1.element);
    delete_lvc(&c_hrm.obj2.element);
    delete_lvc(&c_kcal.obj0.element);
    delete_lvc(&c_kcal.obj1.element);
    delete_lvc(&c_kcal.obj2.element);
    delete_lvc(&c_kcal.obj3.element);
    delete_lvc(&c_sp02.obj0.element);
    delete_lvc(&c_sp02.obj1.element);
    delete_lvc(&c_sp02.obj2.element);

    b_second = -1;
    b_minute = -1;
    b_hour = -1;
    b_mode = !b_mode;
    b_am = !b_am;
    b_day = -1;
    b_month = -1;
    b_year = -1;
    b_weekday = -1;
#endif
}

bool is_obj_valid(lv_obj_t *obj)
{
    return obj != NULL;
}

void add_item(lv_obj_t *root, int id, int x, int y, int pvX, int pvY, const char *image, const char *group[], int group_size)
{
#ifdef ENABLE_CUSTOM_FACE
    if (id == 0)
    {
        if (!is_obj_valid(c_hour.obj0.element))
        {
            c_hour.obj0.element = lv_img_create(root);
            lv_img_set_src(c_hour.obj0.element, image);
            lv_obj_set_width(c_hour.obj0.element, LV_SIZE_CONTENT);
            lv_obj_set_height(c_hour.obj0.element, LV_SIZE_CONTENT);
            lv_obj_set_x(c_hour.obj0.element, x);
            lv_obj_set_y(c_hour.obj0.element, y);
            lv_obj_add_flag(c_hour.obj0.element, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(c_hour.obj0.element, LV_OBJ_FLAG_SCROLLABLE);

            for (int i = 0; i < group_size && i < 20; i++)
            {
                c_hour.obj0.path[i] = strdup(group[i]);
            }
        }
        else if (!is_obj_valid(c_hour.obj1.element))
        {
            c_hour.obj1.element = lv_img_create(root);
            lv_img_set_src(c_hour.obj1.element, image);
            lv_obj_set_width(c_hour.obj1.element, LV_SIZE_CONTENT);
            lv_obj_set_height(c_hour.obj1.element, LV_SIZE_CONTENT);
            lv_obj_set_x(c_hour.obj1.element, x);
            lv_obj_set_y(c_hour.obj1.element, y);
            lv_obj_add_flag(c_hour.obj1.element, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(c_hour.obj1.element, LV_OBJ_FLAG_SCROLLABLE);

            for (int i = 0; i < group_size && i < 20; i++)
            {
                c_hour.obj1.path[i] = strdup(group[i]);
            }
        }
    }
    else if (id == 1)
    {
        if (!is_obj_valid(c_minute.obj0.element))
        {
            c_minute.obj0.element = lv_img_create(root);
            lv_img_set_src(c_minute.obj0.element, image);
            lv_obj_set_width(c_minute.obj0.element, LV_SIZE_CONTENT);
            lv_obj_set_height(c_minute.obj0.element, LV_SIZE_CONTENT);
            lv_obj_set_x(c_minute.obj0.element, x);
            lv_obj_set_y(c_minute.obj0.element, y);
            lv_obj_add_flag(c_minute.obj0.element, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(c_minute.obj0.element, LV_OBJ_FLAG_SCROLLABLE);

            for (int i = 0; i < group_size && i < 20; i++)
            {
                c_minute.obj0.path[i] = strdup(group[i]);
            }
        }
        else if (!is_obj_valid(c_minute.obj1.element))
        {
            c_minute.obj1.element = lv_img_create(root);
            lv_img_set_src(c_minute.obj1.element, image);
            lv_obj_set_width(c_minute.obj1.element, LV_SIZE_CONTENT);
            lv_obj_set_height(c_minute.obj1.element, LV_SIZE_CONTENT);
            lv_obj_set_x(c_minute.obj1.element, x);
            lv_obj_set_y(c_minute.obj1.element, y);
            lv_obj_add_flag(c_minute.obj1.element, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(c_minute.obj1.element, LV_OBJ_FLAG_SCROLLABLE);

            for (int i = 0; i < group_size && i < 20; i++)
            {
                c_minute.obj1.path[i] = strdup(group[i]);
            }
        }
    }
    else if (id == 2)
    {
        if (!is_obj_valid(c_date.obj0.element))
        {
            c_date.obj0.element = lv_img_create(root);
            lv_img_set_src(c_date.obj0.element, image);
            lv_obj_set_width(c_date.obj0.element, LV_SIZE_CONTENT);
            lv_obj_set_height(c_date.obj0.element, LV_SIZE_CONTENT);
            lv_obj_set_x(c_date.obj0.element, x);
            lv_obj_set_y(c_date.obj0.element, y);
            lv_obj_add_flag(c_date.obj0.element, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(c_date.obj0.element, LV_OBJ_FLAG_SCROLLABLE);

            for (int i = 0; i < group_size && i < 20; i++)
            {
                c_date.obj0.path[i] = strdup(group[i]);
            }
        }
        else if (!is_obj_valid(c_date.obj1.element))
        {
            c_date.obj1.element = lv_img_create(root);
            lv_img_set_src(c_date.obj1.element, image);
            lv_obj_set_width(c_date.obj1.element, LV_SIZE_CONTENT);
            lv_obj_set_height(c_date.obj1.element, LV_SIZE_CONTENT);
            lv_obj_set_x(c_date.obj1.element, x);
            lv_obj_set_y(c_date.obj1.element, y);
            lv_obj_add_flag(c_date.obj1.element, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(c_date.obj1.element, LV_OBJ_FLAG_SCROLLABLE);

            for (int i = 0; i < group_size && i < 20; i++)
            {
                c_date.obj1.path[i] = strdup(group[i]);
            }
        }
    }
    else if (id == 3)
    {
        if (group_size == 12)
        {
            if (!is_obj_valid(c_month.obj0.element))
            {
                c_month.obj0.element = lv_img_create(root);
                lv_img_set_src(c_month.obj0.element, image);
                lv_obj_set_width(c_month.obj0.element, LV_SIZE_CONTENT);
                lv_obj_set_height(c_month.obj0.element, LV_SIZE_CONTENT);
                lv_obj_set_x(c_month.obj0.element, x);
                lv_obj_set_y(c_month.obj0.element, y);
                lv_obj_add_flag(c_month.obj0.element, LV_OBJ_FLAG_ADV_HITTEST);
                lv_obj_clear_flag(c_month.obj0.element, LV_OBJ_FLAG_SCROLLABLE);

                for (int i = 0; i < group_size && i < 20; i++)
                {
                    c_month.obj0.path[i] = strdup(group[i]);
                }
            }
        }
        else
        {
            if (!is_obj_valid(c_month_digit.obj0.element))
            {
                c_month_digit.obj0.element = lv_img_create(root);
                lv_img_set_src(c_month_digit.obj0.element, image);
                lv_obj_set_width(c_month_digit.obj0.element, LV_SIZE_CONTENT);
                lv_obj_set_height(c_month_digit.obj0.element, LV_SIZE_CONTENT);
                lv_obj_set_x(c_month_digit.obj0.element, x);
                lv_obj_set_y(c_month_digit.obj0.element, y);
                lv_obj_add_flag(c_month_digit.obj0.element, LV_OBJ_FLAG_ADV_HITTEST);
                lv_obj_clear_flag(c_month_digit.obj0.element, LV_OBJ_FLAG_SCROLLABLE);

                for (int i = 0; i < group_size && i < 20; i++)
                {
                    c_month_digit.obj0.path[i] = strdup(group[i]);
                }
            }
            else if (!is_obj_valid(c_month_digit.obj1.element))
            {
                c_month_digit.obj1.element = lv_img_create(root);
                lv_img_set_src(c_month_digit.obj1.element, image);
                lv_obj_set_width(c_month_digit.obj1.element, LV_SIZE_CONTENT);
                lv_obj_set_height(c_month_digit.obj1.element, LV_SIZE_CONTENT);
                lv_obj_set_x(c_month_digit.obj1.element, x);
                lv_obj_set_y(c_month_digit.obj1.element, y);
                lv_obj_add_flag(c_month_digit.obj1.element, LV_OBJ_FLAG_ADV_HITTEST);
                lv_obj_clear_flag(c_month_digit.obj1.element, LV_OBJ_FLAG_SCROLLABLE);

                for (int i = 0; i < group_size && i < 20; i++)
                {
                    c_month_digit.obj1.path[i] = strdup(group[i]);
                }
            }
        }
    }
    else if (id == 6)
    {
        if (!is_obj_valid(c_weekday.obj0.element))
        {
            c_weekday.obj0.element = lv_img_create(root);
            lv_img_set_src(c_weekday.obj0.element, image);
            lv_obj_set_width(c_weekday.obj0.element, LV_SIZE_CONTENT);
            lv_obj_set_height(c_weekday.obj0.element, LV_SIZE_CONTENT);
            lv_obj_set_x(c_weekday.obj0.element, x);
            lv_obj_set_y(c_weekday.obj0.element, y);
            lv_obj_add_flag(c_weekday.obj0.element, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(c_weekday.obj0.element, LV_OBJ_FLAG_SCROLLABLE);

            for (int i = 0; i < group_size && i < 20; i++)
            {
                c_weekday.obj0.path[i] = strdup(group[i]);
            }
        }
    }
    else if (id == 7)
    {
        if (!is_obj_valid(c_year.obj0.element))
        {
            c_year.obj0.element = lv_img_create(root);
            lv_img_set_src(c_year.obj0.element, image);
            lv_obj_set_width(c_year.obj0.element, LV_SIZE_CONTENT);
            lv_obj_set_height(c_year.obj0.element, LV_SIZE_CONTENT);
            lv_obj_set_x(c_year.obj0.element, x);
            lv_obj_set_y(c_year.obj0.element, y);
            lv_obj_add_flag(c_year.obj0.element, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(c_year.obj0.element, LV_OBJ_FLAG_SCROLLABLE);

            for (int i = 0; i < group_size && i < 20; i++)
            {
                c_year.obj0.path[i] = strdup(group[i]);
            }
        }
        else if (!is_obj_valid(c_year.obj1.element))
        {
            c_year.obj1.element = lv_img_create(root);
            lv_img_set_src(c_year.obj1.element, image);
            lv_obj_set_width(c_year.obj1.element, LV_SIZE_CONTENT);
            lv_obj_set_height(c_year.obj1.element, LV_SIZE_CONTENT);
            lv_obj_set_x(c_year.obj1.element, x);
            lv_obj_set_y(c_year.obj1.element, y);
            lv_obj_add_flag(c_year.obj1.element, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(c_year.obj1.element, LV_OBJ_FLAG_SCROLLABLE);

            for (int i = 0; i < group_size && i < 20; i++)
            {
                c_year.obj1.path[i] = strdup(group[i]);
            }
        }
        else if (!is_obj_valid(c_year.obj2.element))
        {
            c_year.obj2.element = lv_img_create(root);
            lv_img_set_src(c_year.obj2.element, image);
            lv_obj_set_width(c_year.obj2.element, LV_SIZE_CONTENT);
            lv_obj_set_height(c_year.obj2.element, LV_SIZE_CONTENT);
            lv_obj_set_x(c_year.obj2.element, x);
            lv_obj_set_y(c_year.obj2.element, y);
            lv_obj_add_flag(c_year.obj2.element, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(c_year.obj2.element, LV_OBJ_FLAG_SCROLLABLE);

            for (int i = 0; i < group_size && i < 20; i++)
            {
                c_year.obj2.path[i] = strdup(group[i]);
            }
        }
        else if (!is_obj_valid(c_year.obj3.element))
        {
            c_year.obj3.element = lv_img_create(root);
            lv_img_set_src(c_year.obj3.element, image);
            lv_obj_set_width(c_year.obj3.element, LV_SIZE_CONTENT);
            lv_obj_set_height(c_year.obj3.element, LV_SIZE_CONTENT);
            lv_obj_set_x(c_year.obj3.element, x);
            lv_obj_set_y(c_year.obj3.element, y);
            lv_obj_add_flag(c_year.obj3.element, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(c_year.obj3.element, LV_OBJ_FLAG_SCROLLABLE);

            for (int i = 0; i < group_size && i < 20; i++)
            {
                c_year.obj3.path[i] = strdup(group[i]);
            }
        }
    }
    else if (id == 13)
    {
        // analog hands
        // if (!is_obj_valid(c_hourA.obj0.element)){
        //     c_hourA.obj0.element = lv_img_create(root);
        //     lv_img_set_src(c_hourA.obj0.element, image);
        //     lv_obj_set_width(c_hourA.obj0.element, LV_SIZE_CONTENT);
        //     lv_obj_set_height(c_hourA.obj0.element, LV_SIZE_CONTENT);
        //     lv_obj_set_x(c_hourA.obj0.element, x);
        //     lv_obj_set_y(c_hourA.obj0.element, y);
        //     lv_obj_add_flag(c_hourA.obj0.element, LV_OBJ_FLAG_ADV_HITTEST );
        //     lv_obj_clear_flag(c_hourA.obj0.element, LV_OBJ_FLAG_SCROLLABLE );
        //     lv_img_set_pivot(c_hourA.obj0.element, pvX, pvY);

        //     for (int i = 0; i < group_size && i < 20; i++) {
        //         c_hourA.obj0.path[i] = strdup(group[i]);
        //     }

        // } else if (!is_obj_valid(c_minuteA.obj0.element)){
        //     c_minuteA.obj0.element = lv_img_create(root);
        //     lv_img_set_src(c_minuteA.obj0.element, image);
        //     lv_obj_set_width(c_minuteA.obj0.element, LV_SIZE_CONTENT);
        //     lv_obj_set_height(c_minuteA.obj0.element, LV_SIZE_CONTENT);
        //     lv_obj_set_x(c_minuteA.obj0.element, x);
        //     lv_obj_set_y(c_minuteA.obj0.element, y);
        //     lv_obj_add_flag(c_minuteA.obj0.element, LV_OBJ_FLAG_ADV_HITTEST );
        //     lv_obj_clear_flag(c_minuteA.obj0.element, LV_OBJ_FLAG_SCROLLABLE );
        //     lv_img_set_pivot(c_minuteA.obj0.element, pvX, pvY);

        //     for (int i = 0; i < group_size && i < 20; i++) {
        //         c_minuteA.obj0.path[i] = strdup(group[i]);
        //     }
        // } else if (!is_obj_valid(c_secondA.obj0.element)){
        //     c_secondA.obj0.element = lv_img_create(root);
        //     lv_img_set_src(c_secondA.obj0.element, image);
        //     lv_obj_set_width(c_secondA.obj0.element, LV_SIZE_CONTENT);
        //     lv_obj_set_height(c_secondA.obj0.element, LV_SIZE_CONTENT);
        //     lv_obj_set_x(c_secondA.obj0.element, x);
        //     lv_obj_set_y(c_secondA.obj0.element, y);
        //     lv_obj_add_flag(c_secondA.obj0.element, LV_OBJ_FLAG_ADV_HITTEST );
        //     lv_obj_clear_flag(c_secondA.obj0.element, LV_OBJ_FLAG_SCROLLABLE );
        //     lv_img_set_pivot(c_secondA.obj0.element, pvX, pvY);

        //     for (int i = 0; i < group_size && i < 20; i++) {
        //         c_secondA.obj0.path[i] = strdup(group[i]);
        //     }
        // }
    }
    else
    {
        lv_obj_t *elem = lv_img_create(root);
        lv_img_set_src(elem, image);
        lv_obj_set_width(elem, LV_SIZE_CONTENT);
        lv_obj_set_height(elem, LV_SIZE_CONTENT);
        lv_obj_set_x(elem, x);
        lv_obj_set_y(elem, y);
        lv_obj_add_flag(elem, LV_OBJ_FLAG_ADV_HITTEST);
        lv_obj_clear_flag(elem, LV_OBJ_FLAG_SCROLLABLE);
    }
#endif
}

void set_obj_src(lv_obj_t *obj, const char *path)
{
    if (is_obj_valid(obj))
    {
        lv_img_set_src(obj, path);
    }
}

void set_obj_angle(lv_obj_t *obj, int16_t angle)
{
    if (is_obj_valid(obj))
    {
        lv_img_set_angle(obj, angle);
    }
}

void set_obj_flag(lv_obj_t *obj, bool visible)
{
    if (is_obj_valid(obj))
    {
        if (visible)
        {
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void init_face_custom(void (*callback)(const char *, const lv_image_dsc_t *, lv_obj_t **))
{
}
void update_time_custom(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday)
{
#ifdef ENABLE_CUSTOM_FACE
    if (b_second != second)
    {
        set_obj_src(c_second.obj0.element, c_second.obj0.path[(second / 1) % 10]);
        set_obj_src(c_second.obj1.element, c_second.obj1.path[(second / 10) % 10]);
        // set_obj_angle(c_secondA.obj0.element, second * 60);
        b_second = second;
    }

    if (b_minute != minute)
    {
        set_obj_src(c_minute.obj0.element, c_minute.obj0.path[(minute / 1) % 10]);
        set_obj_src(c_minute.obj1.element, c_minute.obj1.path[(minute / 10) % 10]);
        // set_obj_angle(c_minuteA.obj0.element, (minute * 60) + second);
        b_minute = minute;
    }

    if (b_hour != hour)
    {
        set_obj_src(c_hour.obj0.element, c_hour.obj0.path[(hour / 1) % 10]);
        set_obj_src(c_hour.obj1.element, c_hour.obj1.path[(hour / 10) % 10]);
        // set_obj_angle(c_hourA.obj0.element, hour * 300 + (minute * 5) + (second * (5 / 60)));
        b_hour = hour;
    }

    if (b_day != day)
    {
        set_obj_src(c_date.obj0.element, c_date.obj0.path[(day / 1) % 10]);
        set_obj_src(c_date.obj1.element, c_date.obj1.path[(day / 10) % 10]);
        b_day = day;
    }
    if (b_month != month)
    {
        set_obj_src(c_month_digit.obj0.element, c_month_digit.obj0.path[(month / 1) % 10]);
        set_obj_src(c_month_digit.obj1.element, c_month_digit.obj1.path[(month / 10) % 10]);
        set_obj_src(c_month.obj0.element, c_month.obj0.path[(month / 12) % 12]);
        b_month = month;
    }

    if (b_year != year)
    {
        set_obj_src(c_year.obj0.element, c_year.obj0.path[(year / 1) % 10]);
        set_obj_src(c_year.obj1.element, c_year.obj1.path[(year / 10) % 10]);
        set_obj_src(c_year.obj2.element, c_year.obj2.path[(year / 100) % 10]);
        set_obj_src(c_year.obj3.element, c_year.obj3.path[(year / 1000) % 10]);
        b_year = year;
    }

    if (b_weekday != weekday)
    {
        set_obj_src(c_weekday.obj0.element, c_weekday.obj0.path[((weekday + 6) / 1) % 7]);
        b_weekday = weekday;
    }

#endif
}
void update_weather_custom(int temp, int icon)
{
}
void update_status_custom(int battery, bool connection)
{
}
void update_activity_custom(int steps, int distance, int kcal)
{
}
void update_health_custom(int bpm, int oxygen)
{
}
void update_all_custom(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
                       int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
}
void update_check_custom(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
                         int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
}