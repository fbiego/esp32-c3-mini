
/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/

#include "contacts.h"

#ifdef ENABLE_APP_CONTACTS

lv_obj_t *ui_contactScreen;
lv_obj_t *ui_contactList;

void ui_event_contactScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        lv_obj_scroll_by(ui_contactList, 0, -1, LV_ANIM_ON);
    }
    // if (event_code == LV_EVENT_SCREEN_LOADED)
    // {
    //     onGameOpened();
    // }
    // if (event_code == LV_EVENT_SCREEN_UNLOAD_START)
    // {
    // }
    // if (event_code == LV_EVENT_SCREEN_UNLOADED)
    // {
    //     onGameClosed();
    // }

    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        ui_gameExit();
    }
}
#endif

void clearContactList()
{
#ifdef ENABLE_APP_CONTACTS
    lv_obj_clean(ui_contactList);
#endif
}

void addContact(const char *name, const char *number, bool sos)
{
#ifdef ENABLE_APP_CONTACTS
    lv_obj_t *ui_contactPanel = lv_obj_create(ui_contactList);
    lv_obj_set_width(ui_contactPanel, 200);
    lv_obj_set_height(ui_contactPanel, 47);
    lv_obj_set_align(ui_contactPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_contactPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_contactPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_contactPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_contactPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_contactPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_contactPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_contactPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_contactPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_contactPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_contactPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_contactPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_contactPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_contactIcon = lv_img_create(ui_contactPanel);
    lv_img_set_src(ui_contactIcon, sos ? &ui_img_sos_contact_png : &ui_img_contact_png);
    lv_obj_set_width(ui_contactIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_contactIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_contactIcon, 10);
    lv_obj_set_y(ui_contactIcon, 0);
    lv_obj_set_align(ui_contactIcon, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_contactIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_contactIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    lv_obj_t *ui_contactNumber = lv_label_create(ui_contactPanel);
    lv_obj_set_width(ui_contactNumber, 140);
    lv_obj_set_height(ui_contactNumber, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_contactNumber, 50);
    lv_obj_set_y(ui_contactNumber, -5);
    lv_obj_set_align(ui_contactNumber, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_set_style_text_font(ui_contactNumber, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_contactNumber, number);

    lv_obj_t *ui_contactName = lv_label_create(ui_contactPanel);
    lv_obj_set_width(ui_contactName, 140);
    lv_obj_set_height(ui_contactName, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_contactName, 50);
    lv_obj_set_y(ui_contactName, 3);
    lv_label_set_long_mode(ui_contactName, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_font(ui_contactName, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_contactName, name);
#endif
}

void setNoContacts()
{
#ifdef ENABLE_APP_CONTACTS
    lv_obj_clean(ui_contactList);
    lv_obj_t *info = lv_label_create(ui_contactList);
    lv_obj_set_width(info, 180);
    lv_obj_set_y(info, 20);
    lv_obj_set_height(info, LV_SIZE_CONTENT); /// 1
    lv_label_set_long_mode(info, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(info, "No contacts available. Add contacts from Chronos app and sync them");
#endif
}

void ui_contactScreen_screen_init(void (*callback)(const char *, const lv_image_dsc_t *, lv_obj_t **))
{

#ifdef ENABLE_APP_CONTACTS
    ui_contactScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_contactScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_contactScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_contactScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_contactList = lv_obj_create(ui_contactScreen);
    lv_obj_set_width(ui_contactList, lv_pct(100));
    lv_obj_set_height(ui_contactList, lv_pct(100));
    lv_obj_set_align(ui_contactList, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_contactList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_contactList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(ui_contactList, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(ui_contactList, LV_DIR_VER);
    lv_obj_set_style_bg_color(ui_contactList, lv_color_hex(0x070707), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_contactList, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_contactList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_contactList, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_contactList, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_contactList, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_contactList, 100, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_contactList, onScroll, LV_EVENT_SCROLL, NULL);

    lv_obj_add_event_cb(ui_contactScreen, ui_event_contactScreen, LV_EVENT_ALL, NULL);


    setNoContacts();

    callback("Contacts", &ui_img_contact_list_png, &ui_contactScreen);

#endif
}
