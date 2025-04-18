
/**
 * @file app_manager.h
 */

#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    const char *name;
    const lv_image_dsc_t *icon;
    void (*callback)(lv_event_t *e);
} app_info_t;

typedef void (*app_register_cb)(const char *name, const lv_image_dsc_t *icon, void (*callback)(lv_event_t *e));

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void app_registry_register(const app_info_t *info);
void app_registry_iterate(app_register_cb cb);
void app_registry_iterate_unsorted(app_register_cb cb);
int app_registry_get_count();

/**********************
 *      MACROS
 **********************/

/**
 * @brief Register an app with the app manager
 * @param _title The title of the app
 * @param _icon The icon of the app
 * @param _screen_var The variable name of the screen
 * @param _screen_init_fn The function to initialize the screen
 * @note This macro should be used in the source file where the app is implemented.
 *       It will automatically create a static variable for the screen and a callback function
 *       to launch the app. The callback function will be registered with the app manager.
 */
#define REGISTER_APP(_title, _icon, _screen_var, _screen_init_fn)      \
static lv_obj_t *_screen_var;                                          \
static void _screen_var##_launch_cb(lv_event_t *e)                     \
{                                                                      \
    ui_app_load(&_screen_var, _screen_init_fn);                        \
}                                                                      \
static app_info_t _app_info_##_screen_var = {                          \
    .name = _title,                                                    \
    .icon = _icon,                                                     \
    .callback = _screen_var##_launch_cb};                              \
static void __attribute__((constructor)) _register_##_screen_var(void) \
{                                                                      \
    app_registry_register(&_app_info_##_screen_var);                   \
}



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* APP_MANAGER_H */
