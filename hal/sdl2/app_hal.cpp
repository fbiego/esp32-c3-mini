#include <unistd.h>
#include <ctime>
#include <cstring>
#include <stdio.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include SDL_INCLUDE_PATH
#include "display/monitor.h"
#include "indev/mouse.h"
#include "indev/mousewheel.h"
#include "indev/keyboard.h"
#include "sdl/sdl.h"
#include "app_hal.h"

#include <lvgl.h>
#include "ui/ui.h"


struct Notification
{
    int icon;
    const char *app;
    const char *time;
    const char *message;
};

struct Weather
{
    int icon;
    int day;
    int temp;
    int high;
    int low;
};

void hal_setup(void);
void hal_loop(void);

void update_faces();


// some pre-generated data just for preview
Notification notifications[10] = {
    {.icon = 0xC0, .app = "Chronos", .time = "10:27", .message = "Chronos v3.7.0 is live! Enjoy enhanced notifications for finding your watch and viewing activity status, plus more. Upgrade now for an improved experience"},
    {.icon = 0x08, .app = "Skype", .time = "09:30", .message = "Hey there! Just reminding you about our meeting at 10:00 AM. Please make sure to prepare the presentation slides and gather all necessary documents beforehand. Looking forward to a productive discussion!"},
    {.icon = 0x10, .app = "Facebook", .time = "14:20", .message = "You've got 3 new friend requests. Check them out now! Don't forget to catch up with your old friends and see what they've been up to lately. It's always nice to reconnect and expand your social circle."},
    {.icon = 0x18, .app = "Telegram", .time = "16:45", .message = "New message from John: 'Hey, have you seen the latest news?' Let's catch up later today and discuss the latest updates together. It's always interesting to exchange ideas and opinions on current events."},
    {.icon = 0x11, .app = "Messenger", .time = "19:10", .message = "Sarah sent you a photo. Tap to view it! Sarah has always been good at capturing moments. Let's see what memorable snapshot she has shared this time. It might bring back some fond memories or inspire us for our next adventure."},
    {.icon = 0x12, .app = "Instagram", .time = "11:55", .message = "Your post got 50 likes! Keep up the good work. Your creativity and unique perspective are truly appreciated by your followers. Let's continue to share meaningful content and inspire others along the way."},
    {.icon = 0x13, .app = "Weibo", .time = "07:30", .message = "Trending topic: #TravelTuesday. Share your latest adventures! Whether it's a breathtaking landscape, a delicious local dish, or an unforgettable cultural experience, your travel stories never fail to captivate your audience. Let's share another exciting chapter of your journey!"},
    {.icon = 0x09, .app = "Wechat", .time = "22:15", .message = "New message from Mom: 'Don't forget to buy milk on your way home!' Mom always has our best interests at heart. Let's make sure to pick up the milk and any other groceries she needs. It's a small gesture of appreciation for all her love and care."},
    {.icon = 0x0F, .app = "Twitter", .time = "18:00", .message = "Breaking news: SpaceX launches its latest satellite into orbit. The advancements in space exploration never cease to amaze us. Let's stay updated on the latest developments and continue to support the incredible work being done in the field of aerospace engineering."},
    {.icon = 0x07, .app = "Tencent", .time = "13:40", .message = "Your gaming buddy is online. Ready for a match? It's time to put our skills to the test and embark on another thrilling gaming adventure together. Let's strategize, communicate, and emerge victorious as a team!"}};

Weather weather[7] = {
    {.icon = 0, .day = 0, .temp = 21, .high = 22, .low = 18},
    {.icon = 4, .day = 1, .temp = 25, .high = 26, .low = 24},
    {.icon = 5, .day = 2, .temp = 23, .high = 24, .low = 17},
    {.icon = 2, .day = 3, .temp = 20, .high = 23, .low = 12},
    {.icon = 0, .day = 4, .temp = 27, .high = 27, .low = 23},
    {.icon = 3, .day = 5, .temp = 22, .high = 25, .low = 18},
    {.icon = 2, .day = 6, .temp = 24, .high = 26, .low = 19},
};

const char *daysWk[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
/**
 * A task to measure the elapsed time for LittlevGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data)
{
    (void)data;

    while (1)
    {
        SDL_Delay(5);   /*Sleep for 5 millisecond*/
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

void onLoadHome(lv_event_t *e) {}

void onClickAlert(lv_event_t *e) {}

void onForecastOpen(lv_event_t *e) {}

void onWeatherLoad(lv_event_t *e) {}

void onNotificationsOpen(lv_event_t *e) {}

void onBrightnessChange(lv_event_t *e) {}

void onScrollMode(lv_event_t *e) {}

void onTimeoutChange(lv_event_t *e) {}

void onBatteryChange(lv_event_t *e) {}

void onMusicPlay(lv_event_t *e) 
{
    showError("Error", "This is a test error message\nClick the button below to close this window");
}

void onMusicPrevious(lv_event_t *e)
{
    lv_label_set_text(ui_callName, "World");
    lv_scr_load_anim(ui_callScreen, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}

void onMusicNext(lv_event_t *e)
{
    lv_label_set_text(ui_cameraLabel, "Click capture to close to close");
    lv_scr_load_anim(ui_cameraScreen, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}

void onStartSearch(lv_event_t *e) {}

void onEndSearch(lv_event_t *e) {}

void onVolumeUp(lv_event_t *e) {}

void onVolumeDown(lv_event_t *e) {}

void onAutoNavigation(lv_event_t *e) {}

void onAlertState(lv_event_t *e) {}

void onLanguageChange(lv_event_t *e)
{

}


void onWatchfaceChange(lv_event_t *e) {}

void onFaceSelected(lv_event_t *e) {}

void onCustomFaceSelected(int pathIndex) {}
void onRTWState(bool state)
{
  
}

void onGameOpened(){}

void onGameClosed(){}

bool loadCustomFace(const char *file) 
{
    return true;
}

void onMessageClick(lv_event_t *e)
{
    // Your code here
    // int index = (int)lv_event_get_user_data(e);
    intptr_t index = (intptr_t)lv_event_get_user_data(e);

    index %= 10;

    lv_label_set_text(ui_messageTime, notifications[index].time);
    lv_label_set_text(ui_messageContent, notifications[index].message);
    setNotificationIcon(ui_messageIcon, notifications[index].icon);

    lv_obj_scroll_to_y(ui_messagePanel, 0, LV_ANIM_ON);
    lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
}

void onCaptureClick(lv_event_t *e)
{
    lv_scr_load_anim(ui_home, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}

void addFaceList(lv_obj_t *parent, Face face){}

void setupWeather()
{
    // lv_obj_set_style_bg_img_src(ui_weatherScreen, &ui_img_753022056, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_clear_flag(ui_weatherPanel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_forecastPanel, LV_OBJ_FLAG_HIDDEN);

    const char *updateTime = "Updated at\n10:47";
    lv_label_set_text(ui_weatherCity, "Nairobi");
    lv_label_set_text(ui_weatherUpdateTime, updateTime);
    lv_label_set_text_fmt(ui_weatherCurrentTemp, "%d°C", weather[0].temp);

    setWeatherIcon(ui_weatherCurrentIcon, weather[0].icon, true);

    lv_label_set_text_fmt(ui_weatherTemp, "%d°C", weather[0].temp);
    setWeatherIcon(ui_weatherIcon, weather[0].icon, true);

    lv_obj_clean(ui_forecastList);

    for (int i = 0; i < 7; i++)
    {
        addForecast(weather[i].day, weather[i].temp, weather[i].icon);
    }
}

void setupNotifications()
{
    lv_obj_clean(ui_messageList);

    for (int i = 0; i < 10; i++)
    {
        addNotificationList(notifications[i].icon, notifications[i].message, i);
    }

    lv_obj_scroll_to_y(ui_messageList, 1, LV_ANIM_ON);
    lv_obj_clear_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
}

void setupFiles()
{

    // addListDrive("C:/", 16777216, 8456213);
    // addListDrive("D:/", 6456326, 456213);

    addListDir("watchface");
    addListDir("extracted");
    addListDir("bluetooth");

    addListFile("kenya.bin", 152453);
    addListFile("kenya.wf", 453);
    addListFile("list.txt", 2453);

    lv_obj_scroll_to_y(ui_fileManagerPanel, 1, LV_ANIM_ON);
}


void hal_setup(void)
{
// Workaround for sdl2 `-m32` crash
// https://bugs.launchpad.net/ubuntu/+source/libsdl2/+bug/1775067/comments/7
#ifndef WIN32
    setenv("DBUS_FATAL_WARNINGS", "0", 1);
#endif

    lv_init();

    /* Add a display
     * Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/

    static lv_disp_draw_buf_t disp_buf;
    static lv_color_t buf[SDL_HOR_RES * 10];                       /*Declare a buffer for 10 lines*/
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, SDL_HOR_RES * 10); /*Initialize the display buffer*/

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);           /*Basic initialization*/
    disp_drv.flush_cb = sdl_display_flush; /*Used when `LV_VDB_SIZE != 0` in lv_conf.h (buffered drawing)*/
    disp_drv.draw_buf = &disp_buf;
    disp_drv.hor_res = SDL_HOR_RES;
    disp_drv.ver_res = SDL_VER_RES;
    // disp_drv.disp_fill = monitor_fill;      /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
    // disp_drv.disp_map = monitor_map;        /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
    lv_disp_drv_register(&disp_drv);

    /* Add the mouse as input device
     * Use the 'mouse' driver which reads the PC's mouse*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv); /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = sdl_mouse_read; /*This function will be called periodically (by the library) to get the mouse position and state*/
    lv_indev_drv_register(&indev_drv);

    sdl_init();

    ui_init();

    setupNotifications();
    setupWeather();

    setupFiles();

    // int wf = 4; // load watchface 4
    // if (wf >= numFaces)
    // {
    //     wf = 0; // default
    // }
    // ui_home = *faces[wf].watchface; 
    // lv_disp_load_scr(ui_home);

    circular = true;
    lv_obj_scroll_to_y(ui_settingsList, 1, LV_ANIM_ON);
    lv_obj_scroll_to_y(ui_appList, 1, LV_ANIM_ON);
    lv_obj_scroll_to_y(ui_appInfoPanel, 1, LV_ANIM_ON);
    lv_obj_scroll_to_y(ui_gameList, 1, LV_ANIM_ON);
    lv_obj_add_state(ui_Switch2, LV_STATE_CHECKED);

    lv_label_set_text_fmt(ui_aboutText, "%s\nLVGL Simulator\nA1:B2:C3:D4:E5:F6", ui_info_text);

    
    /* Tick init.
     * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about how much time were elapsed
     * Create an SDL thread to do this*/
    SDL_CreateThread(tick_thread, "tick", NULL);
}

void hal_loop(void)
{
    while (1)
    {
        SDL_Delay(5);
        lv_task_handler();
        if (ui_home == ui_clockScreen)
        {
            time_t now = time(0);
            tm *ltm = localtime(&now);

            int second = ltm->tm_sec;
            int minute = ltm->tm_min;
            int hour = ltm->tm_hour;
            bool am = hour < 12;
            int day = ltm->tm_mday;
            int month = 1 + ltm->tm_mon;    // Month starts from 0
            int year = 1900 + ltm->tm_year; // Year is since 1900
            int weekday = ltm->tm_wday;

            lv_label_set_text_fmt(ui_hourLabel, "%02d", hour);
            lv_label_set_text_fmt(ui_dayLabel, "%s", daysWk[weekday]);
            lv_label_set_text_fmt(ui_minuteLabel, "%02d", minute);
            lv_label_set_text_fmt(ui_dateLabel, "%02d\n%s", day, months[month - 1]);
            lv_label_set_text(ui_amPmLabel, "");
        }
        else
        {
            update_faces();
        }

        // this works just okay on native, esp32 implementation is different
        ui_games_update();
        
    }
}

void update_faces()
{
    time_t now = time(0);
    tm *ltm = localtime(&now);

    // Extract time fields
    int second = ltm->tm_sec;
    int minute = ltm->tm_min;
    int hour = ltm->tm_hour;
    bool am = hour < 12;
    int day = ltm->tm_mday;
    int month = 1 + ltm->tm_mon;    // Month starts from 0
    int year = 1900 + ltm->tm_year; // Year is since 1900
    int weekday = ltm->tm_wday;

    // int second = rand() % 60;
    // int minute = rand() % 60;
    // int hour = rand() % 24;
    // bool am = hour < 12;
    // int day = 11;
    // int month = 5;
    // int year = 2024;
    // int weekday = rand() % 7;

    bool mode = true;

    int temp = 22;
    int icon = 1;

    int battery = 75; // rand() % 100;
    bool connection = true;

    int steps = 2735;
    int distance = 17;
    int kcal = 348;
    int bpm = 76;
    int oxygen = 97;

    ui_update_watchfaces(second, minute, hour, mode, am, day, month, year, weekday,
                         temp, icon, battery, connection, steps, distance, kcal, bpm, oxygen);
}
