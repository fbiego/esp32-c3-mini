/*
   MIT License

  Copyright (c) 2023 Felix Biego

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ______________  _____
  ___  __/___  /_ ___(_)_____ _______ _______
  __  /_  __  __ \__  / _  _ \__  __ `/_  __ \
  _  __/  _  /_/ /_  /  /  __/_  /_/ / / /_/ /
  /_/     /_.___/ /_/   \___/ _\__, /  \____/
                              /____/

*/

#define LGFX_USE_V1
#include "Arduino.h"
#include <LovyanGFX.hpp>
#include <Timber.h>
#include "app_hal.h"

#include <lvgl.h>
#include "ui/ui.h"

#include "ui/custom_face.h"

#include "main.h"
#include "splash.h"



#define buf_size 20

class LGFX : public lgfx::LGFX_Device
{

  lgfx::Panel_GC9A01 _panel_instance;
  // lgfx::Light_PWM _light_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Touch_CST816S _touch_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = 1;
      cfg.spi_mode = 0;
      cfg.freq_write = 10000000;
      cfg.pin_sclk = 10;
      cfg.pin_miso = -1;
      cfg.pin_mosi = 11;
      cfg.pin_dc = 8;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 9;
      cfg.pin_rst = 12;
      cfg.panel_width = 240;
      cfg.panel_height = 240;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.invert = true; // パネルの明暗が反転してしまう場合 trueに設定

      _panel_instance.config(cfg);
    }

    // {                                      // Set backlight control. (delete if not necessary)
    //   auto cfg = _light_instance.config(); // Get the structure for backlight configuration.

    //   cfg.pin_bl = 13;     // pin number to which the backlight is connected
    //   cfg.invert = false;  // true to invert backlight brightness
    //   cfg.freq = 44100;    // backlight PWM frequency
    //   cfg.pwm_channel = 1; // PWM channel number to use

    //   _light_instance.config(cfg);
    //   _panel_instance.setLight(&_light_instance); // Sets the backlight to the panel.
    // }

    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
      auto cfg = _touch_instance.config();

      cfg.x_min = 0;        // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max = WIDTH;    // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min = 0;        // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max = HEIGHT;   // タッチスクリーンから得られる最大のY値(生の値)
      cfg.pin_int = 2; // INTが接続されているピン番号
      // cfg.pin_rst = TP_RST;
      cfg.bus_shared = false;  // 画面と共通のバスを使用している場合 trueを設定
      cfg.offset_rotation = 0; // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定
      cfg.i2c_port = 1;        // 使用するI2Cを選択 (0 or 1)
      cfg.i2c_addr = 0x15;     // I2Cデバイスアドレス番号
      cfg.pin_sda = 6;   // SDAが接続されているピン番号
      cfg.pin_scl = 7;   // SCLが接続されているピン番号
      cfg.freq = 100000;       // I2Cクロックを設定

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance); // タッチスクリーンをパネルにセットします。
    }

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

LGFX tft;

static const uint32_t screenWidth = WIDTH;
static const uint32_t screenHeight = HEIGHT;

const unsigned int lvBufferSize = screenWidth * buf_size;
static uint8_t *lvBuffer; // [lvBufferSize];
static uint8_t *lvBuffer2;

static lv_display_t *lvDisplay;
static lv_indev_t *lvInput;

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

struct HourlyForecast
{
  int day;      // day of forecast
  int hour;     // hour of the forecast
  int icon;     // (0-7) // 0 - sun + cloud // 1 - sun // 2 - snow // 3 - rain // 4 - clouds // 5 - tornado // 6 - wind // 7 - sun + haze
  int temp;     //
  int uv;       // uv index
  int humidity; // %
  int wind;     // wind speed km/h
};


void update_faces();
void setupContacts();
void setupWeather();


// some pre-generated data just for preview
Notification notifications[10] = {
    {.icon = 0xC0, .app = "Chronos", .time = "10:27", .message = "Chronos v3.7.5 is live! Enjoy enhanced notifications for finding your watch and viewing activity status, plus more. Upgrade now for an improved experience"},
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

HourlyForecast hourly[24] = {
    {1, 0, 0, 25, 5, 70, 15},   // Day 1, 00:00, Partly Cloudy, 25°C, UV 5, 70% humidity, 15 km/h wind
    {1, 1, 1, 23, 4, 65, 10},   // Day 1, 01:00, Sunny, 23°C, UV 4, 65% humidity, 10 km/h wind
    {1, 2, 3, 20, 2, 85, 12},   // Day 1, 02:00, Rain, 20°C, UV 2, 85% humidity, 12 km/h wind
    {1, 3, 0, 21, 1, 75, 14},   // Day 1, 03:00, Partly Cloudy, 21°C, UV 1, 75% humidity, 14 km/h wind
    {1, 4, 4, 19, 0, 90, 8},    // Day 1, 04:00, Cloudy, 19°C, UV 0, 90% humidity, 8 km/h wind
    {1, 5, 1, 22, 3, 60, 20},   // Day 1, 05:00, Sunny, 22°C, UV 3, 60% humidity, 20 km/h wind
    {1, 6, 0, 26, 6, 50, 18},   // Day 1, 06:00, Partly Cloudy, 26°C, UV 6, 50% humidity, 18 km/h wind
    {1, 7, 1, 28, 7, 45, 22},   // Day 1, 07:00, Sunny, 28°C, UV 7, 45% humidity, 22 km/h wind
    {1, 8, 2, 16, 1, 85, 5},    // Day 1, 08:00, Snow, 16°C, UV 1, 85% humidity, 5 km/h wind
    {1, 9, 6, 30, 8, 35, 25},   // Day 1, 09:00, Windy, 30°C, UV 8, 35% humidity, 25 km/h wind
    {1, 10, 0, 32, 9, 40, 30},  // Day 1, 10:00, Partly Cloudy, 32°C, UV 9, 40% humidity, 30 km/h wind
    {1, 11, 5, 27, 4, 65, 18},  // Day 1, 11:00, Tornado, 27°C, UV 4, 65% humidity, 18 km/h wind
    {1, 12, 7, 35, 10, 20, 12}, // Day 1, 12:00, Sun + Haze, 35°C, UV 10, 20% humidity, 12 km/h wind
    {1, 13, 1, 34, 9, 25, 28},  // Day 1, 13:00, Sunny, 34°C, UV 9, 25% humidity, 28 km/h wind
    {1, 14, 4, 29, 8, 50, 24},  // Day 1, 14:00, Cloudy, 29°C, UV 8, 50% humidity, 24 km/h wind
    {1, 15, 3, 22, 6, 85, 14},  // Day 1, 15:00, Rain, 22°C, UV 6, 85% humidity, 14 km/h wind
    {1, 16, 0, 25, 5, 70, 19},  // Day 1, 16:00, Partly Cloudy, 25°C, UV 5, 70% humidity, 19 km/h wind
    {1, 17, 1, 26, 5, 65, 20},  // Day 1, 17:00, Sunny, 26°C, UV 5, 65% humidity, 20 km/h wind
    {1, 18, 6, 30, 7, 45, 17},  // Day 1, 18:00, Windy, 30°C, UV 7, 45% humidity, 17 km/h wind
    {1, 19, 0, 24, 4, 75, 22},  // Day 1, 19:00, Partly Cloudy, 24°C, UV 4, 75% humidity, 22 km/h wind
    {1, 20, 7, 28, 3, 80, 10},  // Day 1, 20:00, Sun + Haze, 28°C, UV 3, 80% humidity, 10 km/h wind
    {1, 21, 4, 22, 1, 85, 12},  // Day 1, 21:00, Cloudy, 22°C, UV 1, 85% humidity, 12 km/h wind
    {1, 22, 3, 20, 2, 90, 16},  // Day 1, 22:00, Rain, 20°C, UV 2, 90% humidity, 16 km/h wind
    {1, 23, 0, 18, 0, 95, 8}    // Day 1, 23:00, Partly Cloudy, 18°C, UV 0, 95% humidity, 8 km/h wind
};

const char *daysWk[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void my_disp_flush(lv_display_t *display, const lv_area_t *area, unsigned char *data)
{

  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  lv_draw_sw_rgb565_swap(data, w * h);

  if (tft.getStartCount() == 0)
  {
    tft.endWrite();
  }

  tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (uint16_t *)data);
  lv_display_flush_ready(display); /* tell lvgl that flushing is done */
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data)
{
  bool touched;
  uint16_t touchX, touchY;

  touched = tft.getTouch(&touchX, &touchY);

  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

void onLoadHome(lv_event_t *e) {}

void onClickAlert(lv_event_t *e) {}

void onForecastOpen(lv_event_t *e) {}

void onWeatherLoad(lv_event_t *e) 
{
  setupWeather();
}

void onNotificationsOpen(lv_event_t *e) {}

void onBrightnessChange(lv_event_t *e) {}

void onScrollMode(lv_event_t *e) {}

void onTimeoutChange(lv_event_t *e) {}

void onRotateChange(lv_event_t *e) {
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  uint16_t sel = lv_dropdown_get_selected(obj);
  tft.setRotation(sel);
  // screen rotation has changed, invalidate to redraw
  lv_obj_invalidate(lv_scr_act());
}

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

void onVolumeUp(lv_event_t *e)
{
  clearContactList();
  setupContacts();
}

void onVolumeDown(lv_event_t *e)
{
  setNoContacts();
}

void onAutoNavigation(lv_event_t *e) {}

void onAlertState(lv_event_t *e) {}

void onNavState(lv_event_t *e) {}

void onLanguageChange(lv_event_t *e)
{
}

void onWatchfaceChange(lv_event_t *e) {}

void onFaceSelected(lv_event_t *e) {}

void onCustomFaceSelected(int pathIndex) {}

void onRTWState(bool state)
{
}

void savePrefInt(const char *key, int value) {}

int getPrefInt(const char *key, int def_value)
{
  return def_value;
}

void toneOut(int pitch, int duration) {}

void onGameOpened() {}

void onGameClosed() {}

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

void addFaceList(lv_obj_t *parent, Face face) {}

void setupWeather()
{
  // lv_obj_set_style_bg_img_src(ui_weatherScreen, &ui_img_753022056, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_clear_flag(ui_weatherPanel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_forecastList, LV_OBJ_FLAG_HIDDEN);

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

  lv_obj_clean(ui_hourlyList);
  addHourlyWeather(0, 1, 0, 0, 0, 0, true);
  for (int h = 0; h < 24; h++)
  {
    addHourlyWeather(hourly[h].hour, hourly[h].icon, hourly[h].temp, hourly[h].humidity, hourly[h].wind, hourly[h].uv, false);
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

void setupContacts()
{

  clearContactList();
  for (int i = 0; i < 8; i++)
  {
    addContact("fbiego Chronos", "1234567890", i == 2);
  }
}

void my_log_cb(const char *buf)
{
  Serial2.write(buf, strlen(buf));
}

void loadSplash()
{
  int w = 122;
  int h = 130;
  int xOffset = 63;
  int yOffset = 55;
  tft.fillScreen(TFT_BLACK);
  // screenBrightness(200);
  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {
      tft.writePixel(x + xOffset, y + yOffset, uint16_t(splash[(((y * w) + x) * 2)] << 8 | splash[(((y * w) + x) * 2) + 1]));
    }
  }

  delay(2000);
}

static uint32_t my_tick(void)
{
    return millis();
}

void logCallback(Level level, unsigned long time, String message)
{
  Serial2.print(message);
}

int putchar(int ch) {
    Serial2.write(ch);  // Send character to Serial
    return ch;
}

void my_log_cb(lv_log_level_t level, const char *buf)
{
  Serial2.write(buf, strlen(buf));
}

UART Serial2(0,1, NC, NC);

void hal_setup()
{

  Serial2.begin(115200); /* prepare for possible serial debug */

  Timber.setLogCallback(logCallback);

  Timber.i("Starting up device");

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  tft.init();
  tft.initDMA();
  tft.startWrite();
  tft.fillScreen(TFT_BLACK);
  
  // tft.setRotation(rt);
  loadSplash();

  printf("Hello from printf!\n");

  lv_init();

  // lv_log_register_print_cb(my_log_cb);

  lv_tick_set_cb(my_tick);

  lvBuffer = (uint8_t*)malloc(lvBufferSize);
  if (lvBuffer == NULL) {
      // Handle memory allocation failure
      Timber.e("Failed to allocate memory %d", lvBufferSize);
  } else {
    Timber.i("Memory allocated %d", lvBufferSize);
  }

  lvBuffer2 = (uint8_t*)malloc(lvBufferSize);
  if (lvBuffer2 == NULL) {
      // Handle memory allocation failure
      Timber.e("Failed to allocate buffer2 %d", lvBufferSize);
  } else {
    Timber.i("Buffer2 allocated %d", lvBufferSize);
  }

  lvDisplay = lv_display_create(screenWidth, screenHeight);
  lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(lvDisplay, my_disp_flush);
  lv_display_set_buffers(lvDisplay, lvBuffer, lvBuffer2, lvBufferSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

  lvInput = lv_indev_create();
  lv_indev_set_type(lvInput, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(lvInput, my_touchpad_read);

  ui_init();

  // lv_obj_t *label = lv_label_create( lv_screen_active() );
  // lv_label_set_text( label, "Hello Arduino, I'm LVGL!" );
  // lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

  setupNotifications();
  // setupWeather();

  setupFiles();
  setupContacts();

  circular = true;

  lv_obj_scroll_to_y(ui_settingsList, 1, LV_ANIM_ON);
  lv_obj_scroll_to_y(ui_appList, 1, LV_ANIM_ON);
  lv_obj_scroll_to_y(ui_appInfoPanel, 1, LV_ANIM_ON);
  lv_obj_scroll_to_y(ui_gameList, 1, LV_ANIM_ON);
  lv_obj_add_state(ui_Switch2, LV_STATE_CHECKED);

  lv_label_set_text_fmt(ui_aboutText, "%s\nPico RP2040\nA1:B2:C3:D4:E5:F6", ui_info_text);

  ui_setup();

  Timber.i("Setup done");
}

void hal_loop()
{

  lv_timer_handler(); /* let the GUI do its work */
  delay(5); /* let this time pass */

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

  if (ui_home == face_custom_root)
  {
    update_time_custom(second, minute, hour, mode, am, day, month, year, weekday);
  }
  else
  {

    ui_update_watchfaces(second, minute, hour, mode, am, day, month, year, weekday,
                         temp, icon, battery, connection, steps, distance, kcal, bpm, oxygen);
  }
}
