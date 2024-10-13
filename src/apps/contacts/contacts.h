
/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/

#ifndef _CONTACTS_APP_H
#define _CONTACTS_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "app_hal.h"

#ifdef ENABLE_APP_CONTACTS

extern lv_obj_t *ui_contactScreen;

LV_IMG_DECLARE(ui_img_contact_list_png);   // assets/contact_list.png
LV_IMG_DECLARE(ui_img_contact_png);   // assets/contact.png
LV_IMG_DECLARE(ui_img_sos_contact_png);   // assets/sos_contact.png

#endif

    void ui_contactScreen_screen_init(void (*callback)(const char *, const lv_image_dsc_t *, lv_obj_t **));

    void clearContactList();
    void addContact(const char* name, const char *number, bool sos);
    void setNoContacts();

    void ui_gameExit(void);
    
    void onScroll(lv_event_t *e);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif