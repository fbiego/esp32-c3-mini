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
#include <lvgl.h>
#include <LovyanGFX.hpp>
#include "CST816D.h"
#include <ChronosESP32.h>
#include <Timber.h>
#include <Preferences.h>

#include "ui/ui.h"
#include "main.h"

#define buf_size 50

class LGFX : public lgfx::LGFX_Device
{

  lgfx::Panel_GC9A01 _panel_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Bus_SPI _bus_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      // SPIバスの設定
      cfg.spi_host = SPI; // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
      cfg.spi_mode = 0;                  // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 80000000;         // 传输时的SPI时钟（最高80MHz，四舍五入为80MHz除以整数得到的值）
      cfg.freq_read = 20000000;          // 接收时的SPI时钟
      cfg.spi_3wire = true;              // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock = true;               // 使用事务锁时设置为 true
      cfg.dma_channel = SPI_DMA_CH_AUTO; // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
      cfg.pin_sclk = SCLK; // SPIのSCLKピン番号を設定
      cfg.pin_mosi = MOSI; // SPIのCLKピン番号を設定
      cfg.pin_miso = MISO; // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc = DC;     // SPIのD/Cピン番号を設定  (-1 = disable)

      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }

    {                                      // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs = CS;   // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst = RST; // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy = -1; // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定さ BUSYが接続されているピン番号 (-1 = disable)れていますので、不明な項目はコメントアウトして試してみてください。

      cfg.memory_width = 240;   // ドライバICがサポートしている最大の幅
      cfg.memory_height = 240;  // ドライバICがサポートしている最大の高さ
      cfg.panel_width = 240;    // 実際に表示可能な幅
      cfg.panel_height = 240;   // 実際に表示可能な高さ
      cfg.offset_x = 0;         // パネルのX方向オフセット量
      cfg.offset_y = 0;         // パネルのY方向オフセット量
      cfg.offset_rotation = 0;  // 值在旋转方向的偏移0~7（4~7是倒置的）
      cfg.dummy_read_pixel = 8; // 在读取像素之前读取的虚拟位数
      cfg.dummy_read_bits = 1;  // 读取像素以外的数据之前的虚拟读取位数
      cfg.readable = false;     // 如果可以读取数据，则设置为 true
      cfg.invert = true;        // 如果面板的明暗反转，则设置为 true
      cfg.rgb_order = false;    // 如果面板的红色和蓝色被交换，则设置为 true
      cfg.dlen_16bit = false;   // 对于以 16 位单位发送数据长度的面板，设置为 true
      cfg.bus_shared = false;   // 如果总线与 SD 卡共享，则设置为 true（使用 drawJpgFile 等执行总线控制）

      _panel_instance.config(cfg);
    }

    {                                      // Set backlight control. (delete if not necessary)
      auto cfg = _light_instance.config(); // Get the structure for backlight configuration.

      cfg.pin_bl = BL;     // pin number to which the backlight is connected
      cfg.invert = false;  // true to invert backlight brightness
      cfg.freq = 44100;    // backlight PWM frequency
      cfg.pwm_channel = 1; // PWM channel number to use

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); // Sets the backlight to the panel.
    }

    setPanel(&_panel_instance); // 使用するパネルをセットします。
                                //    { // バックライト制御の設定を行います。(必要なければ削除）
                                //    auto cfg = _light_instance.config();// バックライト設定用の構造体を取得します。
                                //    cfg.pin_bl = 8;             // バックライトが接続されているピン番号 BL
                                //    cfg.invert = false;          // バックライトの輝度を反転させる場合 true
                                //    cfg.freq   = 44100;          // バックライトのPWM周波数
                                //    cfg.pwm_channel = 7;         // 使用するPWMのチャンネル番号
                                //    _light_instance.config(cfg);
                                //    _panel_instance.setLight(&_light_instance);//バックライトをパネルにセットします。
                                //    }
  }
};

LGFX tft;
CST816D touch(I2C_SDA, I2C_SCL, TP_RST, TP_INT);
ChronosESP32 watch("Chronos C3");
Preferences prefs;

static const uint32_t screenWidth = 240;
static const uint32_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][screenWidth * buf_size];

String days[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

lv_img_dsc_t notificationIcons[] = {
    ui_img_sms_png,       // SMS
    ui_img_mail_png,      // Mail
    ui_img_penguin_png,   // Penguin
    ui_img_skype_png,     // Skype
    ui_img_whatsapp_png,  // WhatsApp
    ui_img_mail_png,      // Mail2
    ui_img_line_png,      // Line
    ui_img_twitter_png,   // Twitter
    ui_img_facebook_png,  // Facebook
    ui_img_messenger_png, // Messenger
    ui_img_instagram_png, // Instagram
    ui_img_weibo_png,     // Weibo
    ui_img_kakao_png,     // Kakao
    ui_img_viber_png,     // Viber
    ui_img_vkontakte_png, // Vkontakte
    ui_img_telegram_png,  // Telegram
    ui_img_chrns_png,     // Chronos
    ui_img_wechat_png     // Wechat
};

lv_img_dsc_t weatherIcons[] = {
    ui_img_602206286,
    ui_img_602205261,
    ui_img_602199888,
    ui_img_602207311,
    ui_img_dy4_png,
    ui_img_602200913,
    ui_img_602195540,
    ui_img_602202963};

lv_img_dsc_t weatherNtIcons[] = {
    ui_img_229834011, // assets\nt-0.png
    ui_img_229835036, // assets\nt-1.png
    ui_img_229827613, // assets\nt-2.png
    ui_img_229828638, // assets\nt-3.png
    ui_img_229838359, // assets\nt-4.png
    ui_img_229839384, // assets\nt-5.png
    ui_img_229831961, // assets\nt-6.png
    ui_img_229832986, // assets\nt-7.png
};

struct Timer
{
  unsigned long time;
  long duration = 5000;
  bool active;
};

Timer screenTimer;
Timer alertTimer;
Timer searchTimer;

lv_event_t *click;
lv_obj_t *ui_cameraPanel;
lv_obj_t *ui_cameraTitle;
lv_obj_t *ui_cameraIcon;
lv_obj_t *ui_cameraLabel;
lv_obj_t *ui_cameraButton;
lv_obj_t *ui_cameraButtonLabel;

bool circular = false;

void showAlert();
bool isDay();
int getNotificationIconIndex(int id);
int getWeatherIconIndex(int id);
void cameraPanel();
void setTimeout(int i);

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  if (tft.getStartCount() == 0)
  {
    tft.endWrite();
  }

  tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::swap565_t *)&color_p->full);
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{

  bool touched;
  uint8_t gesture;
  uint16_t touchX, touchY;

  touched = touch.getTouch(&touchX, &touchY, &gesture);

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
    screenTimer.time = millis();
    screenTimer.active = true;
  }
}

void connectionCallback(bool state)
{
  Timber.d("Connection change");
  lv_obj_add_flag(ui_cameraPanel, LV_OBJ_FLAG_HIDDEN);
}

void notificationCallback(Notification notification)
{
  Timber.d("Notification Received from " + notification.app + " at " + notification.time);
  Timber.d(notification.message);
  onNotificationsOpen(click);
  showAlert();
}

void configCallback(Config config, uint32_t a, uint32_t b)
{
  switch (config)
  {
  case CF_WEATHER:

    if (a)
    {
      lv_label_set_text_fmt(ui_weatherTemp, "%d°C", watch.getWeatherAt(0).temp);
      // set icon ui_weatherIcon
      if (isDay())
      {
        lv_img_set_src(ui_weatherIcon, &weatherIcons[getWeatherIconIndex(watch.getWeatherAt(0).icon)]);
      }
      else
      {
        lv_img_set_src(ui_weatherIcon, &weatherNtIcons[getWeatherIconIndex(watch.getWeatherAt(0).icon)]);
      }
    }

    break;
  case CF_FONT:
    screenTimer.time = millis();
    screenTimer.active = true;
    if (((b >> 16) & 0xFFFF) == 0x01)
    { // Style 1
      if ((b & 0xFFFF) == 0x01)
      { // TOP
        lv_obj_set_style_text_color(ui_hourLabel, lv_color_hex(a), LV_PART_MAIN | LV_STATE_DEFAULT);
      }
      if ((b & 0xFFFF) == 0x02)
      { // CENTER
        lv_obj_set_style_text_color(ui_minuteLabel, lv_color_hex(a), LV_PART_MAIN | LV_STATE_DEFAULT);
      }
      if ((b & 0xFFFF) == 0x03)
      { // BOTTOM
        lv_obj_set_style_text_color(ui_dayLabel, lv_color_hex(a), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_dateLabel, lv_color_hex(a), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_weatherTemp, lv_color_hex(a), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_amPmLabel, lv_color_hex(a), LV_PART_MAIN | LV_STATE_DEFAULT);
      }
    }
    if (((b >> 16) & 0xFFFF) == 0x02)
    { // Style 2
      if ((b & 0xFFFF) == 0x01)
      { // TOP
        lv_obj_set_style_bg_img_src(ui_clockScreen, &ui_img_forest_png, LV_PART_MAIN | LV_STATE_DEFAULT);
      }
      if ((b & 0xFFFF) == 0x02)
      { // CENTER
        lv_obj_set_style_bg_img_src(ui_clockScreen, &ui_img_lake_png, LV_PART_MAIN | LV_STATE_DEFAULT);
      }
      if ((b & 0xFFFF) == 0x03)
      { // BOTTOM
        lv_obj_set_style_bg_img_src(ui_clockScreen, &ui_img_mountain_png, LV_PART_MAIN | LV_STATE_DEFAULT);
      }
    }
    if (((b >> 16) & 0xFFFF) == 0x03)
    { // Style 3
      if ((b & 0xFFFF) == 0x01)
      { // TOP
        lv_obj_set_style_bg_img_src(ui_clockScreen, &ui_img_stars_png, LV_PART_MAIN | LV_STATE_DEFAULT);
      }
      if ((b & 0xFFFF) == 0x02)
      { // CENTER
        lv_obj_set_style_bg_img_src(ui_clockScreen, &ui_img_753022056, LV_PART_MAIN | LV_STATE_DEFAULT);
      }
      if ((b & 0xFFFF) == 0x03)
      { // BOTTOM
        // lv_obj_set_style_bg_img_src( ui_clockScreen, &ui_img_forest_png, LV_PART_MAIN | LV_STATE_DEFAULT);
      }
    }

    break;
  case CF_CAMERA:
    if (b)
    {
      lv_disp_load_scr(ui_clockScreen);
      lv_obj_clear_flag(ui_cameraPanel, LV_OBJ_FLAG_HIDDEN);
      screenTimer.time = millis() + 50;
      screenTimer.active = true;
    }
    else
    {
      lv_obj_add_flag(ui_cameraPanel, LV_OBJ_FLAG_HIDDEN);
      screenTimer.active = true;
    }
    break;
  }
}

int getWeatherIconIndex(int id)
{
  switch (id)
  {
  case 0:
    return 0;
  case 1:
    return 1;
  case 2:
    return 2;
  case 3:
    return 3;
  case 4:
    return 4;
  case 5:
    return 5;
  case 6:
    return 6;
  case 7:
    return 7;
  default:
    return 0;
  }
}

int getNotificationIconIndex(int id)
{
  switch (id)
  {
  case 0x03:
    return 0;
  case 0x04:
    return 1;
  case 0x07:
    return 2;
  case 0x08:
    return 3;
  case 0x0A:
    return 4;
  case 0x0B:
    return 5;
  case 0x0E:
    return 6;
  case 0x0F:
    return 7;
  case 0x10:
    return 8;
  case 0x11:
    return 9;
  case 0x12:
    return 10;
  case 0x13:
    return 11;
  case 0x14:
    return 12;
  case 0x16:
    return 13;
  case 0x17:
    return 14;
  case 0x18:
    return 15;
  case 0xC0:
    return 16;
  case 0x09:
    return 17;
  default:
    return 0;
  }
}

static void onScroll(lv_event_t *e)
{
  lv_obj_t *list = lv_event_get_target(e);

  lv_area_t list_a;
  lv_obj_get_coords(list, &list_a);
  lv_coord_t list_y_center = list_a.y1 + lv_area_get_height(&list_a) / 2;

  lv_coord_t r = lv_obj_get_height(list) * 7 / 10;
  uint32_t i;
  uint32_t child_cnt = lv_obj_get_child_cnt(list);
  for (i = 0; i < child_cnt; i++)
  {
    lv_obj_t *child = lv_obj_get_child(list, i);
    lv_area_t child_a;
    lv_obj_get_coords(child, &child_a);

    lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

    lv_coord_t diff_y = child_y_center - list_y_center;
    diff_y = LV_ABS(diff_y);

    /*Get the x of diff_y on a circle.*/
    lv_coord_t x;
    /*If diff_y is out of the circle use the last point of the circle (the radius)*/
    if (diff_y >= r)
    {
      x = r;
    }
    else
    {
      /*Use Pythagoras theorem to get x from radius and y*/
      uint32_t x_sqr = r * r - diff_y * diff_y;
      lv_sqrt_res_t res;
      lv_sqrt(x_sqr, &res, 0x8000); /*Use lvgl's built in sqrt root function*/
      x = r - res.i;
    }

    /*Translate the item by the calculated X coordinate*/
    lv_obj_set_style_translate_x(child, circular ? x : 0, 0);

    /*Use some opacity with larger translations*/
    // lv_opa_t opa = lv_map(x, 0, r, LV_OPA_TRANSP, LV_OPA_COVER);
    // lv_obj_set_style_opa(child, LV_OPA_COVER - opa, 0);
  }
}

void onMessageClick(lv_event_t *e)
{
  // Your code here
  int index = (int)lv_event_get_user_data(e);

  index %= NOTIF_SIZE;
  Timber.i("Message clicked at index %d", index);

  lv_label_set_text(ui_messageTime, watch.getNotificationAt(index).time.c_str());
  lv_label_set_text(ui_messageContent, watch.getNotificationAt(index).message.c_str());
  lv_img_set_src(ui_messageIcon, &notificationIcons[getNotificationIconIndex(watch.getNotificationAt(index).icon)]);

  lv_obj_scroll_to_y(ui_messagePanel, 0, LV_ANIM_ON);
  lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
}

void addNotificationList(lv_obj_t *parent, Notification notif, int index)
{
  lv_obj_t *notificationItem = lv_obj_create(parent);
  lv_obj_set_width(notificationItem, 200);
  lv_obj_set_height(notificationItem, LV_SIZE_CONTENT); /// 50
  lv_obj_set_align(notificationItem, LV_ALIGN_CENTER);
  lv_obj_set_flex_flow(notificationItem, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(notificationItem, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(notificationItem, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_radius(notificationItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(notificationItem, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(notificationItem, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(notificationItem, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(notificationItem, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(notificationItem, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(notificationItem, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_left(notificationItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_right(notificationItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_top(notificationItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_bottom(notificationItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *notificationIcon = lv_img_create(notificationItem);
  lv_img_set_src(notificationIcon, &notificationIcons[getNotificationIconIndex(notif.icon)]);
  lv_obj_set_width(notificationIcon, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(notificationIcon, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(notificationIcon, LV_ALIGN_CENTER);
  lv_obj_add_flag(notificationIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
  lv_obj_clear_flag(notificationIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  lv_obj_t *notificationText = lv_label_create(notificationItem);
  lv_obj_set_width(notificationText, 140);
  lv_obj_set_height(notificationText, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(notificationText, LV_ALIGN_CENTER);
  lv_label_set_long_mode(notificationText, LV_LABEL_LONG_DOT);
  lv_label_set_text(notificationText, notif.message.c_str());

  lv_obj_add_event_cb(notificationItem, onMessageClick, LV_EVENT_CLICKED, (void *)index);
}

void addForecast(lv_obj_t *parent, Weather weather)
{
  lv_obj_t *forecastItem = lv_obj_create(parent);
  lv_obj_set_width(forecastItem, 200);
  lv_obj_set_height(forecastItem, 40);
  lv_obj_set_align(forecastItem, LV_ALIGN_CENTER);
  lv_obj_clear_flag(forecastItem, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_radius(forecastItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(forecastItem, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(forecastItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(forecastItem, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(forecastItem, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(forecastItem, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(forecastItem, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_left(forecastItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_right(forecastItem, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_top(forecastItem, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_bottom(forecastItem, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *forecastIcon = lv_img_create(forecastItem);
  lv_img_set_src(forecastIcon, &weatherIcons[getWeatherIconIndex(weather.icon)]);
  lv_obj_set_width(forecastIcon, LV_SIZE_CONTENT);  /// 40
  lv_obj_set_height(forecastIcon, LV_SIZE_CONTENT); /// 32
  lv_obj_set_align(forecastIcon, LV_ALIGN_CENTER);
  lv_obj_add_flag(forecastIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
  lv_obj_clear_flag(forecastIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_img_set_zoom(forecastIcon, 150);

  lv_obj_t *forecastTemp = lv_label_create(forecastItem);
  lv_obj_set_width(forecastTemp, 58);
  lv_obj_set_height(forecastTemp, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(forecastTemp, 68);
  lv_obj_set_y(forecastTemp, 3);
  lv_obj_set_align(forecastTemp, LV_ALIGN_CENTER);
  lv_label_set_long_mode(forecastTemp, LV_LABEL_LONG_CLIP);
  lv_label_set_text_fmt(forecastTemp, "%d°C", weather.temp);
  lv_obj_set_style_text_font(forecastTemp, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *forecastDay = lv_label_create(forecastItem);
  lv_obj_set_width(forecastDay, 68);
  lv_obj_set_height(forecastDay, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(forecastDay, -56);
  lv_obj_set_y(forecastDay, 3);
  lv_obj_set_align(forecastDay, LV_ALIGN_CENTER);
  lv_label_set_long_mode(forecastDay, LV_LABEL_LONG_CLIP);
  lv_label_set_text(forecastDay, (days[weather.day]).c_str());
  lv_obj_set_style_text_font(forecastDay, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void onCaptureClick(lv_event_t *e)
{
  watch.capturePhoto();
}

void cameraPanel()
{
  ui_cameraPanel = lv_obj_create(ui_clockScreen);
  lv_obj_set_width(ui_cameraPanel, 240);
  lv_obj_set_height(ui_cameraPanel, 240);
  lv_obj_set_align(ui_cameraPanel, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_cameraPanel, LV_OBJ_FLAG_HIDDEN);                                    /// Flags
  lv_obj_clear_flag(ui_cameraPanel, LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_bg_color(ui_cameraPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_cameraPanel, 235, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_cameraPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_left(ui_cameraPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_right(ui_cameraPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_top(ui_cameraPanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_bottom(ui_cameraPanel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_cameraTitle = lv_label_create(ui_cameraPanel);
  lv_obj_set_width(ui_cameraTitle, 150);
  lv_obj_set_height(ui_cameraTitle, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_cameraTitle, 0);
  lv_obj_set_y(ui_cameraTitle, 16);
  lv_obj_set_align(ui_cameraTitle, LV_ALIGN_TOP_MID);
  lv_label_set_text(ui_cameraTitle, "Camera");
  lv_obj_set_style_text_align(ui_cameraTitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_cameraTitle, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_cameraTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_cameraTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_cameraTitle, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_cameraTitle, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_left(ui_cameraTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_right(ui_cameraTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_top(ui_cameraTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_bottom(ui_cameraTitle, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_cameraIcon = lv_img_create(ui_cameraPanel);
  lv_img_set_src(ui_cameraIcon, &ui_img_camera_png);
  lv_obj_set_width(ui_cameraIcon, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_cameraIcon, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_cameraIcon, -79);
  lv_obj_set_y(ui_cameraIcon, -5);
  lv_obj_set_align(ui_cameraIcon, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_cameraIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
  lv_obj_clear_flag(ui_cameraIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  ui_cameraLabel = lv_label_create(ui_cameraPanel);
  lv_obj_set_width(ui_cameraLabel, 160);
  lv_obj_set_height(ui_cameraLabel, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_cameraLabel, 28);
  lv_obj_set_y(ui_cameraLabel, -6);
  lv_obj_set_align(ui_cameraLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_cameraLabel, "To close the dialog, exit the camera from the app");
  lv_obj_set_style_text_font(ui_cameraLabel, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_cameraButton = lv_btn_create(ui_cameraPanel);
  lv_obj_set_width(ui_cameraButton, 108);
  lv_obj_set_height(ui_cameraButton, 40);
  lv_obj_set_x(ui_cameraButton, 0);
  lv_obj_set_y(ui_cameraButton, 62);
  lv_obj_set_align(ui_cameraButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_cameraButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
  lv_obj_clear_flag(ui_cameraButton, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
  lv_obj_add_event_cb(ui_cameraButton, onCaptureClick, LV_EVENT_CLICKED, NULL);

  ui_cameraButtonLabel = lv_label_create(ui_cameraButton);
  lv_obj_set_width(ui_cameraButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_cameraButtonLabel, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(ui_cameraButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_cameraButtonLabel, "Capture");
}

void onForecastOpen(lv_event_t *e)
{
  lv_obj_scroll_to_y(ui_forecastPanel, 0, LV_ANIM_ON);
}

void onScrollMode(lv_event_t *e)
{
  lv_obj_t *obj = lv_event_get_target(e);
  circular = lv_obj_has_state(obj, LV_STATE_CHECKED);
  lv_obj_scroll_by(ui_settingsList, 0, circular ? 1 : -1, LV_ANIM_ON);

  prefs.putBool("circular", circular);
}

void onNotificationsOpen(lv_event_t *e)
{
  lv_obj_clean(ui_messageList);
  int c = watch.getNotificationCount();
  for (int i = 0; i < c; i++)
  {
    addNotificationList(ui_messageList, watch.getNotificationAt(i), i);
  }
  lv_obj_scroll_to_y(ui_messageList, 1, LV_ANIM_ON);
  lv_obj_clear_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
}

void onWeatherLoad(lv_event_t *e)
{
  if (isDay())
  {
    lv_obj_set_style_bg_img_src(ui_weatherScreen, &ui_img_857483832, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_img_src(ui_weatherScreen, &ui_img_753022056, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  lv_obj_clear_flag(ui_weatherPanel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_forecastPanel, LV_OBJ_FLAG_HIDDEN);
  if (watch.getWeatherCount() > 0)
  {
    String updateTime = "Updated at\n" + watch.getWeatherTime();
    lv_label_set_text(ui_weatherCity, watch.getWeatherCity().c_str());
    lv_label_set_text(ui_weatherUpdateTime, updateTime.c_str());
    lv_label_set_text_fmt(ui_weatherCurrentTemp, "%d°C", watch.getWeatherAt(0).temp);
    if (isDay())
    {
      lv_img_set_src(ui_weatherCurrentIcon, &weatherIcons[getWeatherIconIndex(watch.getWeatherAt(0).icon)]);
    }
    else
    {
      lv_img_set_src(ui_weatherCurrentIcon, &weatherNtIcons[getWeatherIconIndex(watch.getWeatherAt(0).icon)]);
    }

    lv_obj_clean(ui_forecastList);
    int c = watch.getWeatherCount();
    for (int i = 0; i < c; i++)
    {
      addForecast(ui_forecastList, watch.getWeatherAt(i));
    }
  }
}

void onLoadHome(lv_event_t *e)
{
  // if (isDay())
  // {
  //   lv_obj_set_style_bg_img_src( ui_clockScreen, &ui_img_857483832, LV_PART_MAIN | LV_STATE_DEFAULT);
  // }
  // else
  // {
  //   lv_obj_set_style_bg_img_src( ui_clockScreen, &ui_img_753022056, LV_PART_MAIN | LV_STATE_DEFAULT);
  // }
}

void onBrightnessChange(lv_event_t *e)
{
  // Your code here
  lv_obj_t *slider = lv_event_get_target(e);
  int v = lv_slider_get_value(slider);
  tft.setBrightness(v);

  prefs.putInt("brightness", v);
}

void onBatteryChange(lv_event_t *e)
{
  uint8_t lvl = lv_slider_get_value(ui_batterySlider);
  watch.setBattery(lvl);
}

void onStartSearch(lv_event_t *e)
{
  watch.findPhone(true);
}

void onEndSearch(lv_event_t *e)
{
  watch.findPhone(false);
}

void onClickAlert(lv_event_t *e)
{

  // cancel alert timer
  alertTimer.active = false;
  // change screen to notifications
  lv_disp_load_scr(ui_notificationScreen);

  // enable screen for timeout + 5 seconds
  screenTimer.time = millis() + 5000;
  screenTimer.active = true;

  // load the last received message
  lv_label_set_text(ui_messageTime, watch.getNotificationAt(0).time.c_str());
  lv_label_set_text(ui_messageContent, watch.getNotificationAt(0).message.c_str());
  lv_img_set_src(ui_messageIcon, &notificationIcons[getNotificationIconIndex(watch.getNotificationAt(0).icon)]);

  lv_obj_scroll_to_y(ui_messagePanel, 0, LV_ANIM_ON);
  lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
}

void onTimeoutChange(lv_event_t *e)
{
  lv_obj_t *obj = lv_event_get_target(e);
  uint16_t sel = lv_dropdown_get_selected(obj);
  Timber.i("Selected index: %d", sel);

  setTimeout(sel);
  prefs.putInt("timeout", sel);
}

void setTimeout(int i)
{
  if (i == 4)
  {
    screenTimer.duration = -1; // always on
  }
  else if (i == 0)
  {
    screenTimer.duration = 5000; // 5 seconds
    screenTimer.active = true;
  }
  else if (i < 4)
  {
    screenTimer.duration = 10000 * i; // 10, 20, 30 seconds
    screenTimer.active = true;
  }
}

void onMusicPlay(lv_event_t *e)
{
  watch.musicControl(MUSIC_TOGGLE);
}

void onMusicPrevious(lv_event_t *e)
{
  watch.musicControl(MUSIC_PREVIOUS);
}

void onMusicNext(lv_event_t *e)
{
  watch.musicControl(MUSIC_NEXT);
}

void showAlert()
{
  lv_disp_t *display = lv_disp_get_default();
  lv_obj_t *actScr = lv_disp_get_scr_act(display);
  if (actScr == ui_notificationScreen)
  {
    // at notifications screen, switch to message
    // enable screen for timeout + 5 seconds
    screenTimer.time = millis() + 5000;
    screenTimer.active = true;

    // load the last received message
    lv_label_set_text(ui_messageTime, watch.getNotificationAt(0).time.c_str());
    lv_label_set_text(ui_messageContent, watch.getNotificationAt(0).message.c_str());
    lv_img_set_src(ui_messageIcon, &notificationIcons[getNotificationIconIndex(watch.getNotificationAt(0).icon)]);

    lv_obj_scroll_to_y(ui_messagePanel, 0, LV_ANIM_ON);
    lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    // attach the alert panel to current screen
    lv_obj_set_parent(ui_alertPanel, actScr);

    // load the last received message
    lv_label_set_text(ui_alertText, watch.getNotificationAt(0).message.c_str());
    lv_img_set_src(ui_alertIcon, &notificationIcons[getNotificationIconIndex(watch.getNotificationAt(0).icon)]);

    // turn screen for timeout + 5 seconds
    screenTimer.time = millis() + 5000;
    screenTimer.active = true;

    alertTimer.time = millis();
    alertTimer.active = true;

    // show the alert
    lv_obj_clear_flag(ui_alertPanel, LV_OBJ_FLAG_HIDDEN);
  }
}

void logCallback(Level level, unsigned long time, String message)
{
  Serial.print(message);
}

void setup()
{

  Serial.begin(115200); /* prepare for possible serial debug */

  Timber.setLogCallback(logCallback);

  Timber.i("Starting up device");

  prefs.begin("my-app");

  tft.init();
  tft.initDMA();
  tft.startWrite();

  touch.begin();
  lv_init();

  lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * buf_size);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  ui_init();

  watch.setConnectionCallback(connectionCallback);
  watch.setNotificationCallback(notificationCallback);
  watch.setConfigurationCallback(configCallback);
  watch.begin();
  watch.set24Hour(true);

  String about = "v1.0 [fbiego]\nESP32 C3 Mini\n" + watch.getAddress();
  lv_label_set_text(ui_aboutText, about.c_str());

  lv_obj_add_event_cb(ui_settingsList, onScroll, LV_EVENT_SCROLL, NULL);
  lv_obj_add_event_cb(ui_messageList, onScroll, LV_EVENT_SCROLL, NULL);
  lv_obj_scroll_to_y(ui_settingsList, 1, LV_ANIM_ON);

  bool intro = prefs.getBool("intro", true);

  if (intro)
  {
    showAlert();
    prefs.putBool("intro", false);
  }

  cameraPanel();

  // load saved preferences
  int tm = prefs.getInt("timeout", 0);
  int br = prefs.getInt("brightness", 100);
  circular = prefs.getBool("circular", false);

  if (tm > 4)
  {
    tm = 4;
  }
  else if (tm < 0)
  {
    tm = 0;
  }

  tft.setBrightness(br);

  lv_dropdown_set_selected(ui_timeoutSelect, tm);
  lv_slider_set_value(ui_brightnessSlider, br, LV_ANIM_OFF);
  if (circular)
  {
    lv_obj_add_state(ui_Switch2, LV_STATE_CHECKED);
  }
  else
  {
    lv_obj_clear_state(ui_Switch2, LV_STATE_CHECKED);
  }

  screenTimer.active = true;
  screenTimer.time = millis();

  setTimeout(tm);

  Timber.i("Setup done");
  Timber.i(about);
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);

  watch.loop();

  if (!watch.isConnected())
  {
    lv_obj_add_state(ui_btStateButton, LV_STATE_CHECKED);
  }
  else
  {
    lv_obj_clear_state(ui_btStateButton, LV_STATE_CHECKED);
  }
  lv_label_set_text(ui_hourLabel, watch.getHourZ().c_str());
  lv_label_set_text(ui_dayLabel, watch.getTime("%A").c_str());
  lv_label_set_text(ui_minuteLabel, watch.getTime("%M").c_str());
  lv_label_set_text(ui_dateLabel, watch.getTime("%d\n%B").c_str());
  lv_label_set_text(ui_amPmLabel, watch.getAmPmC(false).c_str());

  if (alertTimer.active)
  {
    if (alertTimer.time + alertTimer.duration < millis())
    {
      alertTimer.active = false;
      lv_obj_add_flag(ui_alertPanel, LV_OBJ_FLAG_HIDDEN);
    }
  }

  if (screenTimer.active)
  {
    uint8_t lvl = lv_slider_get_value(ui_brightnessSlider);
    tft.setBrightness(lvl);

    if (screenTimer.duration < 0)
    {
      // Timber.w("Always On active");
      screenTimer.active = false;
    }
    else if (watch.isCameraReady())
    {
      screenTimer.active = false;
    }
    else if (screenTimer.time + screenTimer.duration < millis())
    {
      Timber.w("Screen timeout");
      screenTimer.active = false;

      tft.setBrightness(0);
      lv_disp_load_scr(ui_clockScreen);
    }
  }
}

bool isDay()
{
  return watch.getHour(true) > 7 && watch.getHour(true) < 21;
}
