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
#include "demos/lv_demos.h"
#include <LovyanGFX.hpp>
#include <Ticker.h>
#include "CST816D.h"
#include <ESP32Time.h>
#include <NimBLEDevice.h>

#include "ui/ui.h"

#define I2C_SDA 4
#define I2C_SCL 5
#define TP_INT 0
#define TP_RST 1

#define off_pin 35
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
      cfg.spi_host = SPI2_HOST; // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
      cfg.spi_mode = 0;                  // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 80000000;         // 传输时的SPI时钟（最高80MHz，四舍五入为80MHz除以整数得到的值）
      cfg.freq_read = 20000000;          // 接收时的SPI时钟
      cfg.spi_3wire = true;              // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock = true;               // 使用事务锁时设置为 true
      cfg.dma_channel = SPI_DMA_CH_AUTO; // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
      cfg.pin_sclk = 6;  // SPIのSCLKピン番号を設定
      cfg.pin_mosi = 7;  // SPIのCLKピン番号を設定
      cfg.pin_miso = -1; // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc = 2;    // SPIのD/Cピン番号を設定  (-1 = disable)

      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }

    {                                      // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs = 10;   // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst = -1;  // RSTが接続されているピン番号  (-1 = disable)
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

      cfg.pin_bl = 3;      // pin number to which the backlight is connected
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
ESP32Time rtc;

static const uint32_t screenWidth = 240;
static const uint32_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][screenWidth * buf_size];

#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

static BLECharacteristic *pCharacteristicTX;
static BLECharacteristic *pCharacteristicRX;

static bool deviceConnected = false;

String macAddr = "AA:BB:CC:DD:EE:FF";
bool hr24 = true;

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
    ui_img_chrns_png      // Chronos
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

struct Notification
{
  int icon;
  String time;
  String message;
};

struct Weather
{
  int icon;
  int day;
  int temp;
};

String weatherTime = "";
String weatherCity = "";
int weatherSize = 0;
Weather weather[7];

bool batRq, infoRq;
Timer infoTimer;
Timer screenTimer;
Timer alertTimer;
Timer searchTimer;

lv_event_t *click;

// Circular buffer size
#define BUFFER_SIZE 10
// Circular buffer for notifications
Notification notifications[BUFFER_SIZE];
int notificationIndex = 0;
bool bufferFull = false;
int msgLen = 0;

bool isDay();
int getNotificationIconIndex(int id);
int getWeatherIconIndex(int id);
void showAlert();

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

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
    infoTimer.active = true;
    infoTimer.time = millis();
  }
  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    pServer->startAdvertising();
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    // uint8_t *pData;
    // std::string value = pCharacteristic->getValue();
    // int len = value.length();
    // pData = pCharacteristic->getData();
    std::string pData = pCharacteristic->getValue();
    int len = pData.length();

    //        Serial.print("Write callback for characteristic ");
    //        Serial.print(pCharacteristic->getUUID().toString().c_str());
    //        Serial.print(" of data length ");
    //        Serial.println(len);
    Serial.print("RX  ");
    for (int i = 0; i < len; i++)
    {
      Serial.printf("%02X ", pData[i]);
    }
    Serial.println();

    if (pData[0] == 0xAB)
    {
      switch (pData[4])
      {
      case 0x93:
        rtc.setTime(pData[13], pData[12], pData[11], pData[10], pData[9], pData[7] * 256 + pData[8]);
        break;
      case 0x7C:
        hr24 = pData[6] == 0;
        break;

      case 0x9C:
        screenTimer.time = millis();
        screenTimer.active = true;
        if (pData[8] == 0x01)
        { // Style 1
          // get color RGB pData[5], pData[6], pData[7]
          uint32_t c = ((uint32_t)pData[5] << 16) | ((uint32_t)pData[6] << 8) | (uint32_t)pData[7];
          if (pData[9] == 0x01)
          { // TOP
            lv_obj_set_style_text_color(ui_hourLabel, lv_color_hex(c), LV_PART_MAIN | LV_STATE_DEFAULT);
          }
          if (pData[9] == 0x02)
          { // CENTER
            lv_obj_set_style_text_color(ui_minuteLabel, lv_color_hex(c), LV_PART_MAIN | LV_STATE_DEFAULT);
          }
          if (pData[9] == 0x03)
          { // BOTTOM
            lv_obj_set_style_text_color(ui_dayLabel, lv_color_hex(c), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(ui_dateLabel, lv_color_hex(c), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(ui_weatherTemp, lv_color_hex(c), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(ui_amPmLabel, lv_color_hex(c), LV_PART_MAIN | LV_STATE_DEFAULT);
          }
        }
        if (pData[8] == 0x02)
        { // Style 2
          if (pData[9] == 0x01)
          { // TOP
            lv_obj_set_style_bg_img_src(ui_clockScreen, &ui_img_forest_png, LV_PART_MAIN | LV_STATE_DEFAULT);
          }
          if (pData[9] == 0x02)
          { // CENTER
            lv_obj_set_style_bg_img_src(ui_clockScreen, &ui_img_lake_png, LV_PART_MAIN | LV_STATE_DEFAULT);
          }
          if (pData[9] == 0x03)
          { // BOTTOM
            lv_obj_set_style_bg_img_src(ui_clockScreen, &ui_img_mountain_png, LV_PART_MAIN | LV_STATE_DEFAULT);
          }
        }
        if (pData[8] == 0x03)
        { // Style 3
          if (pData[9] == 0x01)
          { // TOP
            lv_obj_set_style_bg_img_src(ui_clockScreen, &ui_img_stars_png, LV_PART_MAIN | LV_STATE_DEFAULT);
          }
          if (pData[9] == 0x02)
          { // CENTER
            lv_obj_set_style_bg_img_src(ui_clockScreen, &ui_img_753022056, LV_PART_MAIN | LV_STATE_DEFAULT);
          }
          if (pData[9] == 0x03)
          { // BOTTOM
            // lv_obj_set_style_bg_img_src( ui_clockScreen, &ui_img_forest_png, LV_PART_MAIN | LV_STATE_DEFAULT);
          }
        }

        break;

      case 0x7E:
        // AB 00 11 FF 7E 80 XY 12 00 15 00 15 10 16 00 15 10 15 10 15
        weatherTime = rtc.getTime("updated at\n%H:%M");
        weatherSize = 0;

        for (int k = 0; k < (len - 6) / 2; k++)
        {
          int icon = pData[(k * 2) + 6] >> 4;
          int sign = (pData[(k * 2) + 6] & 1) ? -1 : 1;
          int temp = ((int)pData[(k * 2) + 7]) * sign;
          int dy = rtc.getDayofWeek() + k;
          weather[weatherSize].day = dy % 7;
          weather[weatherSize].icon = icon;
          weather[weatherSize].temp = temp;

          if (k == 0)
          {
            lv_label_set_text_fmt(ui_weatherTemp, "%d°C", temp);
            // set icon ui_weatherIcon
            if (isDay())
            {
              lv_img_set_src(ui_weatherIcon, &weatherIcons[getWeatherIconIndex(icon)]);
            }
            else
            {
              lv_img_set_src(ui_weatherIcon, &weatherNtIcons[getWeatherIconIndex(icon)]);
            }
          }

          weatherSize++;
        }
        break;
      case 0x72:

        int icon = pData[6];

        if (icon == 0x02)
        {
          // skip cancel call command
          break;
        }

        notificationIndex++;
        notifications[notificationIndex % BUFFER_SIZE].icon = icon;
        notifications[notificationIndex % BUFFER_SIZE].time = rtc.getTime("%H:%M");

        String message = "";
        for (int i = 8; i < len; i++)
        {
          message += (char)pData[i];
        }

        notifications[notificationIndex % BUFFER_SIZE].message = message;

        msgLen = pData[2] + 2;

        if (msgLen <= 19)
        {
          // message is complete
          onNotificationsOpen(click);
          showAlert();
        }

        break;
      }
    }
    else if (pData[0] == 0xEA)
    {
      if (pData[4] == 0x7E)
      {
        String city = "";
        for (int c = 7; c < len; c++)
        {
          city += (char)pData[c];
        }
        Serial.print("City: ");
        Serial.println(city);
        weatherCity = city;
      }
    }
    else if (pData[0] <= 0x0F)
    {
      String message = "";
      for (int i = 1; i < len; i++)
      {
        message += (char)pData[i];
      }
      notifications[notificationIndex % BUFFER_SIZE].message += message;
      if (((msgLen > (pData[0] + 1) * 19) && (msgLen <= (pData[0] + 2) * 19)) || (pData[0] == 0x0F))
      {
        // message is complete || message is longer than expected, truncate
        onNotificationsOpen(click);
        showAlert();
      }
    }
  }
};

void init_BLE()
{
  BLEDevice::init("ESP32 C3 mini");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristicTX = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY);
  pCharacteristicRX = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  pCharacteristicRX->setCallbacks(new MyCallbacks());
  pCharacteristicTX->setCallbacks(new MyCallbacks());
  // pCharacteristicTX->addDescriptor(new BLE2902());
  // pCharacteristicTX->setNotifyProperty(true);
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  macAddr = BLEDevice::getAddress().toString().c_str();
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
  default:
    return 0;
  }
}

void onMessageClick(lv_event_t *e)
{
  // Your code here
  int index = (int)lv_event_get_user_data(e);

  Serial.print("Message clicked at index ");
  Serial.print(index);
  index %= BUFFER_SIZE;
  Serial.print(" >> ");
  Serial.println(index);

  lv_label_set_text(ui_messageTime, notifications[index].time.c_str());
  lv_label_set_text(ui_messageContent, notifications[index].message.c_str());
  lv_img_set_src(ui_messageIcon, &notificationIcons[getNotificationIconIndex(notifications[index].icon)]);

  lv_obj_scroll_to_y(ui_notificationPanel, 0, LV_ANIM_ON);
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

void onForecastOpen(lv_event_t * e){

  lv_obj_scroll_to_y(ui_forecastPanel, 0, LV_ANIM_ON);
}

void onNotificationsOpen(lv_event_t *e)
{
  lv_obj_clean(ui_messageList);
  for (int j = notificationIndex; j >= 0; j--)
  {
    if (j <= notificationIndex - BUFFER_SIZE)
    {
      break;
    }
    addNotificationList(ui_messageList, notifications[j % BUFFER_SIZE], j);
  }
  lv_obj_scroll_to_y(ui_notificationPanel, 0, LV_ANIM_ON);
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
  if (weatherSize > 0)
  {
    lv_label_set_text(ui_weatherCity, weatherCity.c_str());
    lv_label_set_text(ui_weatherUpdateTime, weatherTime.c_str());
    lv_label_set_text_fmt(ui_weatherCurrentTemp, "%d°C", weather[0].temp);
    if (isDay())
    {
      lv_img_set_src(ui_weatherCurrentIcon, &weatherIcons[getWeatherIconIndex(weather[0].icon)]);
    }
    else
    {
      lv_img_set_src(ui_weatherCurrentIcon, &weatherNtIcons[getWeatherIconIndex(weather[0].icon)]);
    }

    lv_obj_clean(ui_forecastList);
    for (int i = 0; i < weatherSize; i++)
    {
      addForecast(ui_forecastList, weather[i]);
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
  tft.setBrightness(lv_slider_get_value(slider));
}

void onBatteryChange(lv_event_t *e)
{
  batRq = true;
}

void onStartSearch(lv_event_t *e)
{
  uint8_t startCmd[] = {0xAB, 0x00, 0x04, 0xFF, 0x7D, 0x80, 0x01};
  pCharacteristicTX->setValue(startCmd, 7);
  pCharacteristicTX->notify();
  vTaskDelay(200);
}

void onEndSearch(lv_event_t *e)
{
  uint8_t endCmd[] = {0xAB, 0x00, 0x04, 0xFF, 0x7D, 0x80, 0x00};
  pCharacteristicTX->setValue(endCmd, 7);
  pCharacteristicTX->notify();
  vTaskDelay(200);
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
  lv_label_set_text(ui_messageTime, notifications[notificationIndex % BUFFER_SIZE].time.c_str());
  lv_label_set_text(ui_messageContent, notifications[notificationIndex % BUFFER_SIZE].message.c_str());
  lv_img_set_src(ui_messageIcon, &notificationIcons[getNotificationIconIndex(notifications[notificationIndex % BUFFER_SIZE].icon)]);

  lv_obj_scroll_to_y(ui_notificationPanel, 0, LV_ANIM_ON);
  lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
}

void onTimeoutChange(lv_event_t *e)
{
  lv_obj_t *obj = lv_event_get_target(e);
  uint16_t sel = lv_dropdown_get_selected(obj);
  Serial.print("Selected index: ");
  Serial.println(sel);

  if (sel == 4)
  {
    screenTimer.duration = -1; // always on
  }
  else if (sel == 0)
  {
    screenTimer.duration = 5000; // 5 seconds
    screenTimer.active = true;
  }
  else if (sel < 4)
  {
    screenTimer.duration = 10000 * sel; // 10, 20, 30 seconds
    screenTimer.active = true;
  }
}

void onMusicPlay(lv_event_t *e)
{
  uint8_t playCmd[] = {0xAB, 0x00, 0x04, 0xFF, 0x99, 0x80, 0x00};
  pCharacteristicTX->setValue(playCmd, 7);
  pCharacteristicTX->notify();
  vTaskDelay(200);
}

void onMusicPrevious(lv_event_t *e)
{
  uint8_t prevCmd[] = {0xAB, 0x00, 0x04, 0xFF, 0x99, 0x80, 0x01};
  pCharacteristicTX->setValue(prevCmd, 7);
  pCharacteristicTX->notify();
  vTaskDelay(200);
}

void onMusicNext(lv_event_t *e)
{
  uint8_t nextCmd[] = {0xAB, 0x00, 0x04, 0xFF, 0x99, 0x80, 0x02};
  pCharacteristicTX->setValue(nextCmd, 7);
  pCharacteristicTX->notify();
  vTaskDelay(200);
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
    lv_label_set_text(ui_messageTime, notifications[notificationIndex % BUFFER_SIZE].time.c_str());
    lv_label_set_text(ui_messageContent, notifications[notificationIndex % BUFFER_SIZE].message.c_str());
    lv_img_set_src(ui_messageIcon, &notificationIcons[getNotificationIconIndex(notifications[notificationIndex % BUFFER_SIZE].icon)]);

    lv_obj_scroll_to_y(ui_notificationPanel, 0, LV_ANIM_ON);
    lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    // attach the alert panel to current screen
    lv_obj_set_parent(ui_alertPanel, actScr);

    // load the last received message
    lv_label_set_text(ui_alertText, notifications[notificationIndex % BUFFER_SIZE].message.c_str());
    lv_img_set_src(ui_alertIcon, &notificationIcons[getNotificationIconIndex(notifications[notificationIndex % BUFFER_SIZE].icon)]);

    // turn screen for timeout + 5 seconds
    screenTimer.time = millis() + 5000;
    screenTimer.active = true;

    alertTimer.time = millis();
    alertTimer.active = true;

    // show the alert
    lv_obj_clear_flag(ui_alertPanel, LV_OBJ_FLAG_HIDDEN);
  }
}

void setup()
{

  Serial.begin(115200); /* prepare for possible serial debug */
  Serial.println("Starting up device");

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

  init_BLE();

  String about = "v1.0 [fbiego]\nESP32 C3 Mini\n" + macAddr;
  lv_label_set_text(ui_aboutText, about.c_str());

  notifications[0].icon = 0xC0;
  notifications[0].time = "Chronos";
  notifications[0].message = "Download from Google Play to sync time and receive notifications";

  showAlert();

  screenTimer.active = true;
  screenTimer.time = millis();

  Serial.println("Setup done");
  Serial.println(about);
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);

  if (hr24)
  {
    lv_label_set_text(ui_hourLabel, rtc.getTime("%H").c_str());
    lv_obj_add_flag(ui_amPmLabel, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    lv_label_set_text(ui_hourLabel, rtc.getTime("%I").c_str());
    lv_obj_clear_flag(ui_amPmLabel, LV_OBJ_FLAG_HIDDEN);
  }

  if (!deviceConnected)
  {
    lv_obj_add_state(ui_btStateButton, LV_STATE_CHECKED);
  }
  else
  {
    lv_obj_clear_state(ui_btStateButton, LV_STATE_CHECKED);
  }
  lv_label_set_text(ui_dayLabel, rtc.getTime("%A").c_str());
  lv_label_set_text(ui_minuteLabel, rtc.getTime("%M").c_str());
  lv_label_set_text(ui_dateLabel, rtc.getTime("%d\n%B").c_str());
  lv_label_set_text(ui_amPmLabel, rtc.getAmPm(false).c_str());

  if (infoTimer.active)
  {
    if (infoTimer.time + infoTimer.duration < millis())
    {
      // timer end
      infoTimer.active = false;

      infoRq = true;
      batRq = true;
    }
  }
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
      // Serial.println("Always On active");
      screenTimer.active = false;
    }
    else if (screenTimer.time + screenTimer.duration < millis())
    {
      Serial.println("Screen timeout");
      screenTimer.active = false;

      tft.setBrightness(0);
      lv_disp_load_scr(ui_clockScreen);
    }
  }
  if (batRq)
  {
    batRq = false;
    uint8_t lvl = lv_slider_get_value(ui_batterySlider);
    uint8_t batCmd[] = {0xAB, 0x00, 0x05, 0xFF, 0x91, 0x80, 0x00, lvl};
    pCharacteristicTX->setValue(batCmd, 8);
    pCharacteristicTX->notify();
    vTaskDelay(200);
  }

  if (infoRq)
  {
    infoRq = false;
    uint8_t infoCmd[] = {0xab, 0x00, 0x11, 0xff, 0x92, 0xc0, 0x01, 0x00, 0x00, 0x87, 0x53, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0xe2, 0x07, 0x80};
    pCharacteristicTX->setValue(infoCmd, 20);
    pCharacteristicTX->notify();
    vTaskDelay(200);
  }
}

bool isDay()
{
  return rtc.getHour(true) > 7 && rtc.getHour(true) < 21;
}
