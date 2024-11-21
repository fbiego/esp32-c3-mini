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
#include <ChronosESP32.h>
#include <Timber.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "app_hal.h"

#include "tone.h"

#include <lvgl.h>
#include "ui/ui.h"

#include "ui/custom_face.h"
#include "common/api.h"

#include "main.h"
#include "splash.h"

#include "FS.h"
#include "FFat.h"

#ifdef ENABLE_APP_QMI8658C
#include "FastIMU.h"
#define QMI_ADDRESS 0x6B

#endif

#ifdef ENABLE_RTC
#include <RtcPCF8563.h>
RtcPCF8563<TwoWire> Rtc(Wire);
#endif

#define FLASH FFat
#define F_NAME "FATFS"
#define buf_size 10

class LGFX : public lgfx::LGFX_Device
{

  lgfx::Panel_GC9A01 _panel_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Touch_CST816S _touch_instance;

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

      cfg.memory_width = WIDTH;   // ドライバICがサポートしている最大の幅
      cfg.memory_height = HEIGHT; // ドライバICがサポートしている最大の高さ
      cfg.panel_width = WIDTH;    // 実際に表示可能な幅
      cfg.panel_height = HEIGHT;  // 実際に表示可能な高さ
      cfg.offset_x = OFFSET_X;    // パネルのX方向オフセット量
      cfg.offset_y = OFFSET_Y;    // パネルのY方向オフセット量
      cfg.offset_rotation = 0;    // 值在旋转方向的偏移0~7（4~7是倒置的）
      cfg.dummy_read_pixel = 8;   // 在读取像素之前读取的虚拟位数
      cfg.dummy_read_bits = 1;    // 读取像素以外的数据之前的虚拟读取位数
      cfg.readable = false;       // 如果可以读取数据，则设置为 true
      cfg.invert = true;          // 如果面板的明暗反转，则设置为 true
      cfg.rgb_order = RGB_ORDER;  // 如果面板的红色和蓝色被交换，则设置为 true
      cfg.dlen_16bit = false;     // 对于以 16 位单位发送数据长度的面板，设置为 true
      cfg.bus_shared = false;     // 如果总线与 SD 卡共享，则设置为 true（使用 drawJpgFile 等执行总线控制）

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

    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
      auto cfg = _touch_instance.config();

      cfg.x_min = 0;        // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max = WIDTH;    // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min = 0;        // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max = HEIGHT;   // タッチスクリーンから得られる最大のY値(生の値)
      cfg.pin_int = TP_INT; // INTが接続されているピン番号
      // cfg.pin_rst = TP_RST;
      cfg.bus_shared = true;   // 画面と共通のバスを使用している場合 trueを設定
      cfg.offset_rotation = 0; // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定
      cfg.i2c_port = 0;        // 使用するI2Cを選択 (0 or 1)
      cfg.i2c_addr = 0x15;     // I2Cデバイスアドレス番号
      cfg.pin_sda = I2C_SDA;   // SDAが接続されているピン番号
      cfg.pin_scl = I2C_SCL;   // SCLが接続されているピン番号
      cfg.freq = 400000;       // I2Cクロックを設定

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance); // タッチスクリーンをパネルにセットします。
    }

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

LGFX tft;

ChronosESP32 watch("Chronos C3");
Preferences prefs;

#ifdef ENABLE_APP_QMI8658C
QMI8658 qmi8658c;
calData calib = {0};
AccelData acc;
GyroData gyro;
#endif

static const uint32_t screenWidth = WIDTH;
static const uint32_t screenHeight = HEIGHT;

const unsigned int lvBufferSize = screenWidth * buf_size;
uint8_t lvBuffer[2][lvBufferSize];

bool weatherUpdate = true, notificationsUpdate = true, weatherUpdateFace = true;

ChronosTimer screenTimer;
ChronosTimer alertTimer;
ChronosTimer searchTimer;

Navigation nav;
bool navChanged = false;
bool navIcChanged = false;
uint32_t navIcCRC = 0xFFFFFFFF;

lv_obj_t *lastActScr;

bool circular = false;
bool alertSwitch = false;
bool gameActive = false;
bool readIMU = false;
bool updateSeconds = false;
bool hasUpdatedSec = false;
bool navSwitch = false;

String customFacePaths[15];
int customFaceIndex;
static bool transfer = false;
#ifdef ENABLE_CUSTOM_FACE
#error "Custom Watchface has not been migrated to LVGL 9 yet"
// watchface transfer
int cSize, pos, recv;
uint32_t total, currentRecv;
bool last;
String fName;
uint8_t buf1[1024];
uint8_t buf2[1024];
static bool writeFile = false, wSwitch = true;
static int wLen1 = 0, wLen2 = 0;
bool start = false;
int lastCustom;
#endif

TaskHandle_t gameHandle = NULL;

void showAlert();
bool isDay();
void setTimeout(int i);

void hal_setup(void);
void hal_loop(void);

void update_faces();
void updateQrLinks();

void flashDrive_cb(lv_event_t *e);
void driveList_cb(lv_event_t *e);

void checkLocal(bool faces = false);
void registerWatchface_cb(const char *name, const lv_image_dsc_t *preview, lv_obj_t **watchface, lv_obj_t **second);
void registerCustomFace(const char *name, const lv_image_dsc_t *preview, lv_obj_t **watchface, String path);

String hexString(uint8_t *arr, size_t len, bool caps = false, String separator = "");

bool loadCustomFace(String file);
bool deleteCustomFace(String file);
bool readDialBytes(const char *path, uint8_t *data, size_t offset, size_t size);
bool isKnown(uint8_t id);
void parseDial(const char *path, bool restart = false);
bool lvImgHeader(uint8_t *byteArray, uint8_t cf, uint16_t w, uint16_t h, uint16_t stride);

/* Display flushing */
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
  uint8_t gesture;
  uint16_t touchX, touchY;
  // RemoteTouch rt = watch.getTouch(); // remote touch
  // if (rt.state)
  // {
  //   // use remote touch when active
  //   touched = rt.state;
  //   touchX = rt.x;
  //   touchY = rt.y;
  // }
  // else
  // {
  //   touched = tft.getTouch(&touchX, &touchY);
  // }

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
    screenTimer.time = millis();
    screenTimer.active = true;
  }
}

#ifdef ELECROW_C3
// ELECROW C3 I2C IO extender
#define PI4IO_I2C_ADDR 0x43

// Extended IO function
void init_IO_extender()
{
  Wire.beginTransmission(PI4IO_I2C_ADDR);
  Wire.write(0x01); // test register
  Wire.endTransmission();
  Wire.requestFrom(PI4IO_I2C_ADDR, 1);
  uint8_t rxdata = Wire.read();
  Serial.print("Device ID: ");
  Serial.println(rxdata, HEX);

  Wire.beginTransmission(PI4IO_I2C_ADDR);
  Wire.write(0x03);                                                 // IO direction register
  Wire.write((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4)); // set pins 0, 1, 2 as outputs
  Wire.endTransmission();

  Wire.beginTransmission(PI4IO_I2C_ADDR);
  Wire.write(0x07);                                                    // Output Hi-Z register
  Wire.write(~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4))); // set pins 0, 1, 2 low
  Wire.endTransmission();
}

void set_pin_io(uint8_t pin_number, bool value)
{

  Wire.beginTransmission(PI4IO_I2C_ADDR);
  Wire.write(0x05); // test register
  Wire.endTransmission();
  Wire.requestFrom(PI4IO_I2C_ADDR, 1);
  uint8_t rxdata = Wire.read();
  Serial.print("Before the change: ");
  Serial.println(rxdata, HEX);

  Wire.beginTransmission(PI4IO_I2C_ADDR);
  Wire.write(0x05); // Output register

  if (!value)
    Wire.write((~(1 << pin_number)) & rxdata); // set pin low
  else
    Wire.write((1 << pin_number) | rxdata); // set pin high
  Wire.endTransmission();

  Wire.beginTransmission(PI4IO_I2C_ADDR);
  Wire.write(0x05); // test register
  Wire.endTransmission();
  Wire.requestFrom(PI4IO_I2C_ADDR, 1);
  rxdata = Wire.read();
  Serial.print("after the change: ");
  Serial.println(rxdata, HEX);
}
#endif

#ifdef ENABLE_RTC
bool wasError(const char *errorTopic = "")
{
  uint8_t error = Rtc.LastError();
  if (error != 0)
  {
    // we have a communications error
    // see https://www.arduino.cc/reference/en/language/functions/communication/wire/endtransmission/
    // for what the number means
    Serial.print("[");
    Serial.print(errorTopic);
    Serial.print("] WIRE communications error (");
    Serial.print(error);
    Serial.print(") : ");

    switch (error)
    {
    case Rtc_Wire_Error_None:
      Serial.println("(none?!)");
      break;
    case Rtc_Wire_Error_TxBufferOverflow:
      Serial.println("transmit buffer overflow");
      break;
    case Rtc_Wire_Error_NoAddressableDevice:
      Serial.println("no device responded");
      break;
    case Rtc_Wire_Error_UnsupportedRequest:
      Serial.println("device doesn't support request");
      break;
    case Rtc_Wire_Error_Unspecific:
      Serial.println("unspecified error");
      break;
    case Rtc_Wire_Error_CommunicationTimeout:
      Serial.println("communications timed out");
      break;
    }
    return true;
  }
  return false;
}
#endif

void toneOut(int pitch, int duration)
{ // pitch in Hz, duration in ms
#if defined(BUZZER) && (BUZZER != -1)
  int delayPeriod;
  long cycles, i;

  pinMode(BUZZER, OUTPUT);                        // turn on output pin
  delayPeriod = (500000 / pitch) - 7;             // calc 1/2 period in us -7 for overhead
  cycles = ((long)pitch * (long)duration) / 1000; // calc. number of cycles for loop

  for (i = 0; i <= cycles; i++)
  { // play note for duration ms
    digitalWrite(BUZZER, HIGH);
    delayMicroseconds(delayPeriod);
    digitalWrite(BUZZER, LOW);
    delayMicroseconds(delayPeriod - 1); // - 1 to make up for digitaWrite overhead
  }
  pinMode(BUZZER, INPUT); // shut off pin to avoid noise from other operations
#endif
}

String heapUsage()
{
  String usage;
  uint32_t total = ESP.getHeapSize();
  uint32_t free = ESP.getFreeHeap();
  usage += "Total: " + String(total);
  usage += "\tFree: " + String(free);
  usage += "\t" + String(((total - free) * 1.0) / total * 100, 2) + "%";
  return usage;
}

void *sd_open_cb(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
  char buf[256];
  sprintf(buf, "/%s", path);
  // Serial.print("path : ");
  // Serial.println(buf);

  File f;

  if (mode == LV_FS_MODE_WR)
  {
    f = FLASH.open(buf, FILE_WRITE);
  }
  else if (mode == LV_FS_MODE_RD)
  {
    f = FLASH.open(buf);
  }
  else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
  {
    f = FLASH.open(buf, FILE_WRITE);
  }

  if (!f)
  {
    return NULL; // Return NULL if the file failed to open
  }

  File *fp = new File(f); // Allocate the File object on the heap
  return (void *)fp;      // Return the pointer to the allocated File object
}

lv_fs_res_t sd_read_cb(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
  lv_fs_res_t res = LV_FS_RES_NOT_IMP;
  File *fp = (File *)file_p;
  uint8_t *buffer = (uint8_t *)buf;

  // Serial.print("name sd_read_cb : ");
  // Serial.println(fp->name());
  *br = fp->read(buffer, btr);

  res = LV_FS_RES_OK;
  return res;
}

lv_fs_res_t sd_seek_cb(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
  lv_fs_res_t res = LV_FS_RES_OK;
  File *fp = (File *)file_p;

  uint32_t actual_pos;

  switch (whence)
  {
  case LV_FS_SEEK_SET:
    actual_pos = pos;
    break;
  case LV_FS_SEEK_CUR:
    actual_pos = fp->position() + pos;
    break;
  case LV_FS_SEEK_END:
    actual_pos = fp->size() + pos;
    break;
  default:
    return LV_FS_RES_INV_PARAM; // Invalid parameter
  }

  if (!fp->seek(actual_pos))
  {
    return LV_FS_RES_UNKNOWN; // Seek failed
  }

  // Serial.print("name sd_seek_cb : ");
  // Serial.println(fp->name());

  return res;
}

lv_fs_res_t sd_tell_cb(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
  lv_fs_res_t res = LV_FS_RES_NOT_IMP;
  File *fp = (File *)file_p;

  *pos_p = fp->position();
  // Serial.print("name in sd_tell_cb : ");
  // Serial.println(fp->name());
  res = LV_FS_RES_OK;
  return res;
}

lv_fs_res_t sd_close_cb(lv_fs_drv_t *drv, void *file_p)
{
  File *fp = (File *)file_p;

  fp->close();
  // delete fp;  // Free the allocated memory

  return LV_FS_RES_OK;
}

void checkLocal(bool faces)
{

  File root = FLASH.open("/");
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
    }
    else
    {
#ifdef ENABLE_CUSTOM_FACE
      String nm = String(file.name());
      // addListFile(file.name(), file.size());
      if (faces)
      {
        if (nm.endsWith(".js")){
          Serial.print("Found raw face file: ");
          Serial.println(nm);
          String js = "/" + nm + "on";
          nm = "/" + nm;
          if(!FLASH.exists(js.c_str())){
            Serial.println("Parsing");
            parseDial(nm.c_str(), false);
          } else {
            Serial.println("Skipping, already parsed");
          }
          
        }
      }
      else
      {
       
        if (nm.endsWith(".json"))
        {
          // load watchface elements
          nm = "/" + nm;
          registerCustomFace(nm.c_str(), &ui_img_custom_preview_png, &face_custom_root, nm);
        }
      }
#endif
    }
    file = root.openNextFile();
  }
}

void screenBrightness(uint8_t value)
{
  tft.setBrightness(value);
#ifdef ELECROW_C3
  set_pin_io(2, value > 0); // ELECROW C3, no brightness control
#endif
}

String readFile(const char *path)
{
  String result;
  File file = FLASH.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return result;
  }

  Serial.println("- read from file:");
  while (file.available())
  {
    result += (char)file.read();
  }
  file.close();
  return result;
}

void deleteFile(const char *path)
{
  Serial.printf("Deleting file: %s\r\n", path);
  if (FLASH.remove(path))
  {
    Serial.println("- file deleted");
  }
  else
  {
    Serial.println("- delete failed");
  }
}

bool setupFS()
{

#ifndef ENABLE_CUSTOM_FACE
  return false;
#endif

  if (!FLASH.begin(true, "/ffat", MAX_FILE_OPEN))
  {
    FLASH.format();

    return false;
  }

  static lv_fs_drv_t sd_drv;
  lv_fs_drv_init(&sd_drv);
  sd_drv.cache_size = 512;

  sd_drv.letter = 'S';
  sd_drv.open_cb = sd_open_cb;
  sd_drv.close_cb = sd_close_cb;
  sd_drv.read_cb = sd_read_cb;
  sd_drv.seek_cb = sd_seek_cb;
  sd_drv.tell_cb = sd_tell_cb;
  lv_fs_drv_register(&sd_drv);

  checkLocal(true); // parse new faces

  checkLocal(); // register the local faces

  return true;
}

void listDir(const char *dirname, uint8_t levels)
{

  lv_obj_clean(ui_fileManagerPanel);

  addListBack(driveList_cb);

  File root = FLASH.open(dirname);
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      addListDir(file.name());
      // if (levels)
      // {
      //   listDir(file.path(), levels - 1);
      // }
    }
    else
    {
      addListFile(file.name(), file.size());
    }
    file = root.openNextFile();
  }
}

void flashDrive_cb(lv_event_t *e)
{
  lv_disp_t *display = lv_disp_get_default();
  lv_obj_t *actScr = lv_disp_get_scr_act(display);
  if (actScr != ui_filesScreen)
  {
    return;
  }

  listDir("/", 0);
}

void sdDrive_cb(lv_event_t *e)
{
  lv_disp_t *display = lv_disp_get_default();
  lv_obj_t *actScr = lv_disp_get_scr_act(display);
  if (actScr != ui_filesScreen)
  {
    return;
  }

  showError("Error", "SD card is currently unavaliable");
}

void driveList_cb(lv_event_t *e)
{
  lv_obj_clean(ui_fileManagerPanel);

  addListDrive(F_NAME, FLASH.totalBytes(), FLASH.usedBytes(), flashDrive_cb);
  addListDrive("SD card", 0, 0, sdDrive_cb); // dummy SD card drive
}

bool loadCustomFace(String file)
{
  String path = file;
  if (!path.startsWith("/"))
  {
    path = "/" + path;
  }
  String read = readFile(path.c_str());
  JsonDocument face;
  DeserializationError err = deserializeJson(face, read);
  if (!err)
  {
    if (!face.containsKey("elements"))
    {
      return false;
    }
    String name = face["name"].as<String>();
    JsonArray elements = face["elements"].as<JsonArray>();
    int sz = elements.size();

    Serial.print(sz);
    Serial.println(" elements");

    invalidate_all();
    lv_obj_clean(face_custom_root);

    for (int i = 0; i < sz; i++)
    {
      JsonObject element = elements[i];
      int id = element["id"].as<int>();
      int x = element["x"].as<int>();
      int y = element["y"].as<int>();
      int pvX = element["pvX"].as<int>();
      int pvY = element["pvY"].as<int>();
      String image = element["image"].as<String>();
      JsonArray group = element["group"].as<JsonArray>();

      const char *group_arr[20];
      int group_size = group.size();
      for (int j = 0; j < group_size && j < 20; j++)
      {
        group_arr[j] = group[j].as<const char *>();
      }

      add_item(face_custom_root, id, x, y, pvX, pvY, image.c_str(), group_arr, group_size);
    }

    return true;
  }
  else
  {
    Serial.println("Deserialize failed");
  }

  return false;
}

bool deleteCustomFace(String file)
{
  String path = file;
  if (!path.startsWith("/"))
  {
    path = "/" + path;
  }
  String read = readFile(path.c_str());
  JsonDocument face;
  DeserializationError err = deserializeJson(face, read);
  if (!err)
  {
    if (!face.containsKey("assets"))
    {
      return false;
    }

    JsonArray assets = face["assets"].as<JsonArray>();
    int sz = assets.size();

    for (int j = 0; j < sz; j++)
    {
      deleteFile(assets[j].as<const char *>());
    }

    deleteFile(path.c_str());

    return true;
  }
  else
  {
    Serial.println("Deserialize failed");
  }

  return false;
}

void registerCustomFace(const char *name, const lv_image_dsc_t *preview, lv_obj_t **watchface, String path)
{
  if (numFaces >= MAX_FACES)
  {
    return;
  }
  faces[numFaces].name = name;
  faces[numFaces].preview = preview;
  faces[numFaces].watchface = watchface;

  faces[numFaces].customIndex = customFaceIndex;
  faces[numFaces].custom = true;

  addWatchface(faces[numFaces].name, faces[numFaces].preview, numFaces);

  customFacePaths[customFaceIndex] = path;
  customFaceIndex++;

  Timber.i("Custom Watchface: %s registered at %d", name, numFaces);
  numFaces++;
}

void onCustomDelete(lv_event_t *e)
{
  int index = (int)lv_event_get_user_data(e);

  Serial.println("Delete custom watchface");
  Serial.println(customFacePaths[index]);
  showError("Delete", "The watchface will be deleted from storage, ESP32 will restart after deletion");
  if (deleteCustomFace(customFacePaths[index]))
  {
    lv_scr_load_anim(ui_appListScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, false);
    ESP.restart();
  }
  else
  {
    showError("Error", "Failed to delete watchface");
  }
}

void addFaceList(lv_obj_t *parent, Face face)
{

  lv_obj_t *ui_faceItemPanel = lv_obj_create(parent);
  lv_obj_set_width(ui_faceItemPanel, 240);
  lv_obj_set_height(ui_faceItemPanel, 50);
  lv_obj_set_align(ui_faceItemPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_faceItemPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_radius(ui_faceItemPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_faceItemPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_faceItemPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_faceItemPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_faceItemPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_faceItemPanel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_faceItemPanel, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_color(ui_faceItemPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
  // lv_obj_set_style_bg_opa(ui_faceItemPanel, 100, LV_PART_MAIN | LV_STATE_PRESSED);

  lv_obj_t *ui_faceItemIcon = lv_img_create(ui_faceItemPanel);
  lv_img_set_src(ui_faceItemIcon, &ui_img_clock_png);
  lv_obj_set_width(ui_faceItemIcon, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_faceItemIcon, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_faceItemIcon, 10);
  lv_obj_set_y(ui_faceItemIcon, 0);
  lv_obj_set_align(ui_faceItemIcon, LV_ALIGN_LEFT_MID);
  lv_obj_add_flag(ui_faceItemIcon, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
  lv_obj_clear_flag(ui_faceItemIcon, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  lv_obj_t *ui_faceItemName = lv_label_create(ui_faceItemPanel);
  lv_obj_set_width(ui_faceItemName, 117);
  lv_obj_set_height(ui_faceItemName, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_faceItemName, 50);
  lv_obj_set_y(ui_faceItemName, 0);
  lv_obj_set_align(ui_faceItemName, LV_ALIGN_LEFT_MID);
  lv_label_set_long_mode(ui_faceItemName, LV_LABEL_LONG_CLIP);
  if (face.custom)
  {
    lv_label_set_text(ui_faceItemName, customFacePaths[face.customIndex].c_str());
  }
  else
  {
    lv_label_set_text(ui_faceItemName, face.name);
  }

  lv_obj_set_style_text_font(ui_faceItemName, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_faceItemDelete = lv_img_create(ui_faceItemPanel);
  lv_img_set_src(ui_faceItemDelete, &ui_img_bin_png);
  lv_obj_set_width(ui_faceItemDelete, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_faceItemDelete, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_faceItemDelete, -10);
  lv_obj_set_y(ui_faceItemDelete, 0);
  lv_obj_set_align(ui_faceItemDelete, LV_ALIGN_RIGHT_MID);
  lv_obj_add_flag(ui_faceItemDelete, LV_OBJ_FLAG_CLICKABLE);    /// Flags
  lv_obj_add_flag(ui_faceItemDelete, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
  lv_obj_clear_flag(ui_faceItemDelete, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_radius(ui_faceItemDelete, 20, LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_style_bg_color(ui_faceItemDelete, lv_color_hex(0xF34235), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_style_bg_opa(ui_faceItemDelete, 255, LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_style_border_color(ui_faceItemDelete, lv_color_hex(0xF34235), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_style_border_opa(ui_faceItemDelete, 255, LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_style_border_width(ui_faceItemDelete, 2, LV_PART_MAIN | LV_STATE_PRESSED);

  if (!face.custom)
  {
    lv_obj_add_flag(ui_faceItemDelete, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    lv_obj_add_event_cb(ui_faceItemDelete, onCustomDelete, LV_EVENT_CLICKED, (void *)face.customIndex);
  }
}

void connectionCallback(bool state)
{
  Timber.d(state ? "Connected" : "Disconnected");
  if (state)
  {
    lv_obj_clear_state(ui_btStateButton, LV_STATE_CHECKED);
  }
  else
  {
    lv_obj_add_state(ui_btStateButton, LV_STATE_CHECKED);
  }
  lv_label_set_text_fmt(ui_appConnectionText, "Status\n%s", state ? "Connected" : "Disconnected");
}

void ringerCallback(String caller, bool state)
{
  lv_disp_t *display = lv_disp_get_default();
  lv_obj_t *actScr = lv_disp_get_scr_act(display);

  if (state)
  {
    screenTimer.time = millis() + 50;

    lastActScr = actScr;
    Serial.print("Ringer: Incoming call from ");
    Serial.println(caller);
    lv_label_set_text(ui_callName, caller.c_str());
    lv_scr_load_anim(ui_callScreen, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
  }
  else
  {
    Serial.println("Ringer dismissed");
    // load last active screen
    if (actScr == ui_callScreen && lastActScr != nullptr)
    {
      lv_scr_load_anim(lastActScr, LV_SCR_LOAD_ANIM_FADE_OUT, 500, 0, false);
    }
  }
  screenTimer.active = true;
}

void notificationCallback(Notification notification)
{
  Timber.d("Notification Received from " + notification.app + " at " + notification.time);
  Timber.d(notification.message);
  notificationsUpdate = true;
  // onNotificationsOpen(click);
  showAlert();
}

void configCallback(Config config, uint32_t a, uint32_t b)
{
  switch (config)
  {
  case CF_TIME:
    // time has been synced from BLE
#ifdef ENABLE_RTC
    // set the RTC time
    Rtc.SetDateTime(RtcDateTime(watch.getYear(), watch.getMonth() + 1, watch.getDay(), watch.getHour(true), watch.getMinute(), watch.getSecond()));

#endif
    // ui_update_seconds(watch.getSecond());
    if (!hasUpdatedSec)
    {
      hasUpdatedSec = true;
      updateSeconds = true;
    }

    break;
  case CF_RST:

    Serial.println("Reset request, formating storage");
    FLASH.format();
    delay(2000);
    ESP.restart();

    break;
  case CF_WEATHER:

    if (a)
    {
      weatherUpdateFace = true;
    }
    if (a == 2)
    {
      weatherUpdate = true;
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

    break;
  case CF_CAMERA:
  {
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);

    if (b)
    {
      screenTimer.time = millis() + 50;
      lastActScr = actScr;
      lv_scr_load_anim(ui_cameraScreen, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
      screenTimer.active = true;
    }
    else
    {
      if (actScr == ui_cameraScreen && lastActScr != nullptr)
      {
        lv_scr_load_anim(lastActScr, LV_SCR_LOAD_ANIM_FADE_OUT, 500, 0, false);
      }
      screenTimer.active = true;
    }
  }
  break;

  case CF_APP:
    // state is saved internally
    Serial.print("Chronos App; Code: ");
    Serial.print(a); // int code = watch.getAppCode();
    Serial.print(" Version: ");
    Serial.println(watch.getAppVersion());
    lv_label_set_text_fmt(ui_appDetailsText, "Chronos app\nv%s (%d)", watch.getAppVersion().c_str(), a);
    break;
  case CF_QR:
    if (a == 1)
    {
      updateQrLinks();
    }
    break;
  case CF_NAV_DATA:
    navChanged = true;
    break;
  case CF_NAV_ICON:
    if (a == 2)
    {
      navIcChanged = true;
      Timber.w("Navigation icon received. CRC 0x%04X", b);
    }
    break;
  case CF_CONTACT:
    if (a == 0)
    {
      Serial.println("Receiving contacts");
      Serial.print("SOS index: ");
      Serial.print(uint8_t(b >> 8));
      Serial.print("\tSize: ");
      Serial.println(uint8_t(b));
      setNoContacts();
    }
    if (a == 1)
    {
      Serial.println("Received all contacts");
      int n = uint8_t(b);      // contacts size -> watch.getContactCount();
      int s = uint8_t(b >> 8); // sos contact index -> watch.getSOSContactIndex();

      clearContactList();

      for (int i = 0; i < n; i++)
      {
        Contact cn = watch.getContact(i);
        Serial.print("Name: ");
        Serial.print(cn.name);
        Serial.print(s == i ? " [SOS]" : "");
        Serial.print("\tNumber: ");
        Serial.println(cn.number);
        addContact(cn.name.c_str(), cn.number.c_str(), s == i);
      }
    }
    break;
  }
}

void onMessageClick(lv_event_t *e)
{

  lv_disp_t *display = lv_disp_get_default();
  lv_obj_t *actScr = lv_disp_get_scr_act(display);
  if (actScr != ui_notificationScreen)
  {
    Timber.i("Message screen inactive");
    return;
  }
  // Your code here
  int index = (int)lv_event_get_user_data(e);

  index %= NOTIF_SIZE;
  Timber.i("Message clicked at index %d", index);

  lv_label_set_text(ui_messageTime, watch.getNotificationAt(index).time.c_str());
  lv_label_set_text(ui_messageContent, watch.getNotificationAt(index).message.c_str());
  setNotificationIcon(ui_messageIcon, watch.getNotificationAt(index).icon);

  lv_obj_scroll_to_y(ui_messagePanel, 0, LV_ANIM_ON);
  lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
}

void onCaptureClick(lv_event_t *e)
{
  watch.capturePhoto();
}

void onForecastOpen(lv_event_t *e)
{
  // lv_obj_scroll_to_y(ui_forecastList, 0, LV_ANIM_ON);
}

void onScrollMode(lv_event_t *e)
{
  prefs.putBool("circular", circular);
}

void onAlertState(lv_event_t *e)
{
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  alertSwitch = lv_obj_has_state(obj, LV_STATE_CHECKED);

  prefs.putBool("alerts", alertSwitch);
}

void onNavState(lv_event_t *e)
{
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  navSwitch = lv_obj_has_state(obj, LV_STATE_CHECKED);

  prefs.putBool("autonav", navSwitch);
}

void savePrefInt(const char *key, int value)
{
  prefs.putInt(key, value);
}

int getPrefInt(const char *key, int def_value)
{
  return prefs.getInt(key, def_value);
}

void onNotificationsOpen(lv_event_t *e)
{
  if (!notificationsUpdate)
  {
    return;
  }
  notificationsUpdate = false;

  lv_obj_clean(ui_messageList);
  int c = watch.getNotificationCount();
  for (int i = 0; i < c; i++)
  {
    addNotificationList(watch.getNotificationAt(i).icon, watch.getNotificationAt(i).message.c_str(), i);
  }
  // addNotificationList(watch.getNotificationAt(0).icon, watch.getNotificationAt(0).message.c_str(), i);

  lv_obj_scroll_to_y(ui_messageList, 1, LV_ANIM_ON);
  lv_obj_clear_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
}

void onWeatherLoad(lv_event_t *e)
{
  lv_obj_clear_flag(ui_weatherPanel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_forecastList, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_hourlyList, LV_OBJ_FLAG_HIDDEN);

  if (!weatherUpdate)
  {
    return;
  }
  weatherUpdate = false;

  if (watch.getWeatherCount() > 0)
  {
    String updateTime = "Updated at\n" + watch.getWeatherTime();
    lv_label_set_text(ui_weatherCity, watch.getWeatherCity().c_str());
    lv_label_set_text(ui_weatherUpdateTime, updateTime.c_str());
    lv_label_set_text_fmt(ui_weatherCurrentTemp, "%d°C", watch.getWeatherAt(0).temp);
    setWeatherIcon(ui_weatherIcon, watch.getWeatherAt(0).icon, isDay());
    setWeatherIcon(ui_weatherCurrentIcon, watch.getWeatherAt(0).icon, isDay());

    lv_obj_clean(ui_forecastList);
    int c = watch.getWeatherCount();
    for (int i = 0; i < c; i++)
    {
      addForecast(watch.getWeatherAt(i).day, watch.getWeatherAt(i).temp, watch.getWeatherAt(i).icon);
    }

    // lv_obj_scroll_by(ui_forecastList, 0, -1, LV_ANIM_OFF);

    lv_obj_clean(ui_hourlyList);
    addHourlyWeather(0, watch.getWeatherAt(0).icon, 0, 0, 0, 0, true);
    for (int h = watch.getHour(true); h < 24; h++)
    {
      HourlyForecast hf = watch.getForecastHour(h);
      addHourlyWeather(hf.hour, hf.icon, hf.temp, hf.humidity, hf.wind, hf.uv, false);
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
  lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
  int v = lv_slider_get_value(slider);
  screenBrightness(v);

  prefs.putInt("brightness", v);
}

void onFaceSelected(lv_event_t *e)
{
  int index = (int)lv_event_get_user_data(e);
  prefs.putInt("watchface", index);
}

void onCustomFaceSelected(int pathIndex)
{
#ifdef ENABLE_CUSTOM_FACE

  if (pathIndex < 0)
  {
    prefs.putString("custom", "");
    return;
  }
  if (lv_obj_get_child_cnt(face_custom_root) > 0 && lastCustom == pathIndex)
  {
    ui_home = face_custom_root;
  }
  else if (loadCustomFace(customFacePaths[pathIndex]))
  {
    lastCustom = pathIndex;
    ui_home = face_custom_root;
  }

  lv_scr_load_anim(ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, false);

  prefs.putString("custom", customFacePaths[pathIndex]);
#endif
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
  setNotificationIcon(ui_messageIcon, watch.getNotificationAt(0).icon);

  lv_obj_scroll_to_y(ui_messagePanel, 0, LV_ANIM_ON);
  lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
}

void onTimeoutChange(lv_event_t *e)
{
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  uint16_t sel = lv_dropdown_get_selected(obj);
  Timber.i("Selected index: %d", sel);

  setTimeout(sel);
  prefs.putInt("timeout", sel);
}

void onRotateChange(lv_event_t *e)
{
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  uint16_t sel = lv_dropdown_get_selected(obj);
  Timber.i("Selected index: %d", sel);

  prefs.putInt("rotate", sel);

  tft.setRotation(sel);
  // screen rotation has changed, invalidate to redraw
  lv_obj_invalidate(lv_scr_act());
}

void onLanguageChange(lv_event_t *e)
{
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

void onVolumeUp(lv_event_t *e)
{
  watch.musicControl(VOLUME_UP);
}

void onVolumeDown(lv_event_t *e)
{
  watch.musicControl(VOLUME_DOWN);
}

void updateQrLinks()
{
#if LV_USE_QRCODE == 1
  lv_obj_clean(ui_qrPanel);
  for (int i = 0; i < 9; i++)
  {
    addQrList(i, watch.getQrAt(i).c_str());
  }
#endif
}

void onRTWState(bool state)
{
}

void gameLoop(void *pvParameters)
{
  for (;;)
  {
    ui_games_update();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void onGameOpened()
{
  gameActive = true;

  if (gameHandle == NULL)
  {
    // create task to run the game loop
    xTaskCreate(gameLoop, "Game Task", 8192, NULL, 1, &gameHandle);
  }
}

void onGameClosed()
{
  gameActive = false;

  if (gameHandle != NULL)
  {
    vTaskDelete(gameHandle);
    gameHandle = NULL;
  }
  screenTimer.active = true;
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
    setNotificationIcon(ui_messageIcon, watch.getNotificationAt(0).icon);

    lv_obj_scroll_to_y(ui_messagePanel, 0, LV_ANIM_ON);
    lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    if (!alertSwitch)
    {
      return;
    }
    // attach the alert panel to current screen
    lv_obj_set_parent(ui_alertPanel, actScr);

    // load the last received message
    lv_label_set_text(ui_alertText, watch.getNotificationAt(0).message.c_str());
    setNotificationIcon(ui_alertIcon, watch.getNotificationAt(0).icon);

    // turn screen for timeout + 5 seconds
    screenTimer.time = millis() + 5000;
    screenTimer.active = true;

    alertTimer.time = millis();
    alertTimer.active = true;

    // show the alert
    lv_obj_clear_flag(ui_alertPanel, LV_OBJ_FLAG_HIDDEN);
  }
}

void rawDataCallback(uint8_t *data, int len)
{

#ifdef ENABLE_CUSTOM_FACE
  if (data[0] == 0xB0)
  {
    // this is a chunk header data command
    cSize = data[1] * 256 + data[2];                                                           // data chunk size
    pos = data[3] * 256 + data[4];                                                             // position of the chunk, ideally sequential 0..
    last = data[7] == 1;                                                                       // whether this is the last chunk (1) or not (0)
    total = (data[8] * 256 * 256 * 256) + (data[9] * 256 * 256) + (data[10] * 256) + data[11]; // total size of the whole file
    recv = 0;                                                                                  // counter for the chunk data

    start = pos == 0;
    if (pos == 0)
    {
      // this is the first chunk
      transfer = true;
      currentRecv = 0;

      fName = "/" + String(total, HEX) + "-" + String(total) + ".cbn";
    }
  }
  if (data[0] == 0xAF)
  {
    // this is the chunk data, line by line. The complete chunk will have several of these
    // actual data starts from index 5
    int ln = ((data[1] * 256 + data[2]) - 5); // byte 1 and 2 make up the (total size of data - 5)

    if (wSwitch)
    {
      memcpy(buf1 + recv, data + 5, ln);
    }
    else
    {
      memcpy(buf2 + recv, data + 5, ln);
    }

    recv += ln; // increment the received chunk data size by current received size

    currentRecv += ln; // track the progress

    if (recv == cSize)
    { // received expected? if data chunk size equals chunk receive size then chunk is complete
      if (wSwitch)
      {
        wLen1 = cSize;
      }
      else
      {
        wLen2 = cSize;
      }

      wSwitch = !wSwitch;
      writeFile = true;

      pos++;
      uint8_t lst = last ? 0x01 : 0x00;
      uint8_t cmd[5] = {0xB0, 0x02, highByte(pos), lowByte(pos), lst};
      watch.sendCommand(cmd, 5); // notify the app that we received the chunk, this will trigger transfer of next chunk
    }

    if (last)
    {
    }
  }
#endif
}

void dataCallback(uint8_t *data, int length)
{
  // Serial.println("Received Data");
  // for (int i = 0; i < length; i++)
  // {
  //   Serial.printf("%02X ", data[i]);
  // }
  // Serial.println();
}


void imu_init()
{
#ifdef ENABLE_APP_QMI8658C
  int err = qmi8658c.init(calib, QMI_ADDRESS);
  if (err != 0)
  {
    showError("IMU State", "Failed to init");
  }
#endif
}

imu_data_t get_imu_data()
{
  imu_data_t qmi;
#ifdef ENABLE_APP_QMI8658C

  qmi8658c.update();
  qmi8658c.getAccel(&acc);
  qmi8658c.getGyro(&gyro);


  qmi.ax = acc.accelX;
  qmi.ay = acc.accelY;
  qmi.az = acc.accelZ;
  qmi.gx = gyro.gyroX;
  qmi.gy = gyro.gyroY;
  qmi.gz = gyro.gyroZ;
  qmi.temp = qmi8658c.getTemp();
  qmi.success = true;
#else
  qmi.success = false;
#endif
  return qmi;
}

void imu_close()
{
#ifdef ENABLE_APP_QMI8658C

#endif
}

void logCallback(Level level, unsigned long time, String message)
{
  Serial.print(message);
  Serial1.print(message);
}

// void lv_log_register_print_cb(lv_log_print_g_cb_t print_cb) {
//   // Do nothing, not needed here!
// }

// void my_log_cb(lv_log_level_t level, const char *buf)
// {
//   Serial.write(buf, strlen(buf));
//   Serial1.write(buf, strlen(buf));
// }

int putchar(int ch) {
    Serial.write(ch);  // Send character to Serial
    return ch;
}

void loadSplash()
{
  int w = 122;
  int h = 130;
  int xOffset = 63;
  int yOffset = 55;
  tft.fillScreen(TFT_BLACK);
  screenBrightness(200);
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

void hal_setup()
{

  Serial.begin(115200); /* prepare for possible serial debug */
  Serial1.begin(115200);

  Timber.setLogCallback(logCallback);

  Timber.i("Starting up device");

  prefs.begin("my-app");

  int rt = prefs.getInt("rotate", 0);

#ifdef ELECROW_C3
  Wire.begin(4, 5);
  init_IO_extender();
  delay(100);
  set_pin_io(2, true);
  set_pin_io(3, true);
  set_pin_io(4, true);
#endif

  tft.init();
  tft.initDMA();
  tft.startWrite();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(rt);
  loadSplash();

  toneOut(TONE_EN * 2, 170);
  toneOut(TONE_FS * 2, 170);
  toneOut(TONE_GN * 2, 170);

  Serial.println(heapUsage());

  lv_init();

  lv_tick_set_cb(my_tick);

  static auto *lvDisplay = lv_display_create(screenWidth, screenHeight);
  lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(lvDisplay, my_disp_flush);

  lv_display_set_buffers(lvDisplay, lvBuffer[0], lvBuffer[1], lvBufferSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

  static auto *lvInput = lv_indev_create();
  lv_indev_set_type(lvInput, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(lvInput, my_touchpad_read);

  // lv_log_register_print_cb(my_log_cb);

  // _lv_fs_init();

  ui_init();

  Serial.println(heapUsage());

  bool fsState = setupFS();
  if (fsState)
  {
    // driveList_cb(NULL);
    Serial.println("Setup FS success");
  }
  else
  {
    Serial.println("Setup FS failed");
    // showError(F_NAME, "Failed to mount the partition");
  }
  Serial.println(heapUsage());

  int wf = prefs.getInt("watchface", 0);
  // String custom = prefs.getString("custom", "");
  if (wf >= numFaces)
  {
    wf = 0; // default
  }
  currentIndex = wf;
  // if (custom != "" && fsState && loadCustomFace(custom))
  // {
  //   ui_home = face_custom_root;
  // }
  // else
  // {
    ui_home = *faces[wf].watchface; // load saved watchface power on
  // }
  lv_disp_load_scr(ui_home);

  int ch = lv_obj_get_child_count(ui_faceSelect);
  if (wf < ch)
  {
    lv_obj_scroll_to_view(lv_obj_get_child(ui_faceSelect, wf), LV_ANIM_OFF);
  }

#ifdef ESPS3_1_69
  watch.setScreen(CS_240x296_191_RTF);
#endif
  String chip = String(ESP.getChipModel());
  watch.setName(chip);
  watch.setConnectionCallback(connectionCallback);
  watch.setNotificationCallback(notificationCallback);
  watch.setConfigurationCallback(configCallback);
  watch.setRingerCallback(ringerCallback);
  watch.setDataCallback(dataCallback);
  watch.setRawDataCallback(rawDataCallback);
  watch.begin();
  watch.set24Hour(true);
  watch.setBattery(85);

  String about = String(ui_info_text) + "\n" + chip + "\n" + watch.getAddress();
  lv_label_set_text(ui_aboutText, about.c_str());

#if LV_USE_QRCODE == 1
  String address = watch.getAddress();
  address.toUpperCase();
  String qrCode = "{\"Name\":\"" + chip + "\", \"Mac\":\"" + address + "\"}";
  lv_qrcode_update(ui_connectImage, qrCode.c_str(), qrCode.length());
  lv_label_set_text(ui_connectText, "Scan to connect");
#endif
  // bool intro = prefs.getBool("intro", true);

  // if (intro)
  // {
  //   showAlert();
  //   prefs.putBool("intro", false);
  // }
  // else
  // {
  //   lv_obj_add_flag(ui_alertPanel, LV_OBJ_FLAG_HIDDEN);
  // }

  // load saved preferences
  int tm = prefs.getInt("timeout", 0);

  int br = prefs.getInt("brightness", 100);
  circular = prefs.getBool("circular", false);
  alertSwitch = prefs.getBool("alerts", false);
  navSwitch = prefs.getBool("autonav", false);

  lv_obj_scroll_to_y(ui_settingsList, 1, LV_ANIM_ON);
  lv_obj_scroll_to_y(ui_appList, 1, LV_ANIM_ON);
  lv_obj_scroll_to_y(ui_appInfoPanel, 1, LV_ANIM_ON);
  lv_obj_scroll_to_y(ui_gameList, 1, LV_ANIM_ON);

  if (tm > 4)
  {
    tm = 4;
  }
  else if (tm < 0)
  {
    tm = 0;
  }

  screenBrightness(br);

  lv_dropdown_set_selected(ui_timeoutSelect, tm);
  lv_dropdown_set_selected(ui_rotateSelect, rt);
  lv_slider_set_value(ui_brightnessSlider, br, LV_ANIM_OFF);
  if (circular)
  {
    lv_obj_add_state(ui_Switch2, LV_STATE_CHECKED);
  }
  else
  {
    lv_obj_clear_state(ui_Switch2, LV_STATE_CHECKED);
  }
  if (alertSwitch)
  {
    lv_obj_add_state(ui_alertStateSwitch, LV_STATE_CHECKED);
  }
  else
  {
    lv_obj_clear_state(ui_alertStateSwitch, LV_STATE_CHECKED);
  }
#ifdef ENABLE_APP_NAVIGATION
  if (navSwitch)
  {
    lv_obj_add_state(ui_navStateSwitch, LV_STATE_CHECKED);
  }
  else
  {
    lv_obj_clear_state(ui_navStateSwitch, LV_STATE_CHECKED);
  }
#endif

  screenTimer.active = true;
  screenTimer.time = millis();

  setTimeout(tm);

  // if (!fsState){
  //   showError(F_NAME, "Failed to mount the partition");
  // }

  if (lv_fs_is_ready('S'))
  {
    Serial.println("Drive S is ready");
  }
  else
  {
    Serial.println("Drive S is not ready");
  }

  imu_init();

#ifdef ENABLE_RTC
  Rtc.Begin();

  if (!Rtc.GetIsRunning())
  {
    uint8_t error = Rtc.LastError();
    if (error != 0)
    {
      showError("RTC", "Error on RTC");
    }
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();

  watch.setTime(now.Second(), now.Minute(), now.Hour(), now.Day(), now.Month(), now.Year());

  Rtc.StopAlarm();
  Rtc.StopTimer();
  Rtc.SetSquareWavePin(PCF8563SquareWavePinMode_None);
#endif

  ui_update_seconds(watch.getSecond());

  lv_rand_set_seed(millis());

  navigateInfo("Navigation", "Chronos", "Start navigation on Google maps");

  watch.clearNotifications();
  notificationsUpdate = false;
  lv_obj_clean(ui_messageList);
  lv_obj_t *info = lv_label_create(ui_messageList);
  lv_obj_set_width(info, 180);
  lv_obj_set_y(info, 20);
  lv_obj_set_height(info, LV_SIZE_CONTENT); /// 1
  lv_label_set_long_mode(info, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_font(info, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(info, "No notifications available. Connect Chronos app to receive phone notifications");

  ui_setup();

  Timber.i("Setup done");
  Timber.i(about);
}

void hal_loop()
{

  if (!transfer)
  {
    lv_timer_handler(); // Update the UI-
    delay(5);

    watch.loop();

    if (updateSeconds)
    {
      updateSeconds = false;
      ui_update_seconds(watch.getSecond());
    }

    if (ui_home == ui_clockScreen)
    {
      lv_label_set_text(ui_hourLabel, watch.getHourZ().c_str());
      lv_label_set_text(ui_dayLabel, watch.getTime("%A").c_str());
      lv_label_set_text(ui_minuteLabel, watch.getTime("%M").c_str());
      lv_label_set_text(ui_dateLabel, watch.getTime("%d\n%b").c_str());
      lv_label_set_text(ui_amPmLabel, watch.getAmPmC(false).c_str());
    }
    else
    {
      update_faces();
    }


    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if (actScr != ui_home)
    {
    }

    if (weatherUpdateFace){
      lv_label_set_text_fmt(ui_weatherTemp, "%d°C", watch.getWeatherAt(0).temp);
      // set icon ui_weatherIcon
      setWeatherIcon(ui_weatherIcon, watch.getWeatherAt(0).icon, isDay());
      weatherUpdateFace = false;
    }

    if (navChanged)
    {
      navChanged = false;
      nav = watch.getNavigation();
      if (!nav.active)
      {
        nav.directions = "Start navigation on Google maps";
        nav.title = "Chronos";
        nav.duration = watch.isConnected() ? "Inactive" : "Disconnected";
        nav.eta = "Navigation";
        nav.distance = "";
        navIcCRC = 0xFFFFFFFF;
      }
      navIconState(nav.active && nav.hasIcon);

      if (!nav.isNavigation)
      {
        nav.directions = nav.title;
        nav.title = "";
      }

      String navText = nav.eta + "\n" + nav.duration + " " + nav.distance;
      navigateInfo(navText.c_str(), nav.title.c_str(), nav.directions.c_str());

#ifdef ENABLE_APP_NAVIGATION
      if (actScr != ui_navScreen && nav.active && navSwitch)
      {
        lastActScr = actScr;
        lv_scr_load_anim(ui_navScreen, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
        gameActive = true;
        screenTimer.active = true;
      }
      if (actScr == ui_navScreen && !nav.active && navSwitch && lastActScr != nullptr)
      {
        screenTimer.active = true;
        lv_scr_load_anim(lastActScr, LV_SCR_LOAD_ANIM_FADE_OUT, 500, 0, false);
      }
#endif
    }
    if (navIcChanged)
    {
      navIcChanged = false;
      nav = watch.getNavigation();

      if (nav.iconCRC != navIcCRC)
      {
        navIcCRC = nav.iconCRC;
        navIconState(nav.active && nav.hasIcon);
        for (int y = 0; y < 48; y++)
        {
          for (int x = 0; x < 48; x++)
          {
            int byte_index = (y * 48 + x) / 8;
            int bit_pos = 7 - (x % 8);
            bool px_on = (nav.icon[byte_index] >> bit_pos) & 0x01;
            setNavIconPx(x, y, px_on);
          }
        }
      }
    }

    if (actScr == ui_appInfoScreen)
    {
      lv_label_set_text_fmt(ui_appBatteryText, "Battery - %d%%", watch.getPhoneBattery());
      lv_bar_set_value(ui_appBatteryLevel, watch.getPhoneBattery(), LV_ANIM_OFF);
      if (watch.isPhoneCharging())
      {
        lv_img_set_src(ui_appBatteryIcon, &ui_img_battery_plugged_png);
      }
      else
      {
        lv_img_set_src(ui_appBatteryIcon, &ui_img_battery_state_png);
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
      screenBrightness(lvl);

      if (screenTimer.duration < 0)
      {
        Timber.w("Always On active");
        screenTimer.active = false;
      }
      else if (watch.isCameraReady() || gameActive)
      {
        screenTimer.active = false;
      }
      else if (screenTimer.time + screenTimer.duration < millis())
      {
        Timber.w("Screen timeout");
        screenTimer.active = false;

        screenBrightness(0);
        lv_disp_load_scr(ui_home);
      }
    }
  }

#ifdef ENABLE_CUSTOM_FACE
  if (writeFile && transfer)
  {
    if (start)
    {
      screenBrightness(200);
      tft.fillScreen(TFT_BLUE);

      tft.drawRoundRect(70, 120, 100, 20, 5, TFT_WHITE);
    }

    writeFile = false;

    File file = FLASH.open(fName, start ? FILE_WRITE : FILE_APPEND);
    if (file)
    {

      if (!wSwitch)
      {
        file.write(buf1, wLen1);
      }
      else
      {
        file.write(buf2, wLen2);
      }

      file.close();

      // Serial.print(last ? "Complete: " :  "");
      // Serial.print("Receieved ");
      // Serial.print(currentRecv);
      // Serial.print("/");
      // Serial.print(total);
      // Serial.print("   ");
      // Serial.println(hexString(cmd, 5, true, "-"));

      if (total > 0)
      {
        int progress = (100 * currentRecv) / total;

        // Serial.println(String(progress, 2) + "%");
        tft.setTextColor(TFT_WHITE, TFT_BLUE);
        tft.setTextSize(2);
        tft.setCursor(80, 80);
        tft.print(progress);
        tft.print("%");

        tft.fillRoundRect(70, 120, progress, 20, 5, TFT_WHITE);
      }

      if (last)
      {
        // the file transfer has ended
        transfer = false;

        tft.setTextColor(TFT_WHITE, TFT_BLUE);
        tft.setTextSize(2);
        tft.setCursor(60, 80);
        tft.print("Processing");

        parseDial(fName.c_str(), true); // process the file
      }
    }
    else
    {
      Serial.println("- failed to open file for writing");

      transfer = false;

      ESP.restart();
    }
  }

#endif
}

bool isDay()
{
  return watch.getHour(true) > 7 && watch.getHour(true) < 21;
}

void update_faces()
{
  int second = watch.getSecond();
  int minute = watch.getMinute();
  int hour = watch.getHourC();
  bool mode = watch.is24Hour();
  bool am = watch.getHour(true) < 12;
  int day = watch.getDay();
  int month = watch.getMonth() + 1;
  int year = watch.getYear();
  int weekday = watch.getDayofWeek();

  int temp = watch.getWeatherAt(0).temp;
  int icon = watch.getWeatherAt(0).icon;

  int battery = watch.getPhoneBattery();
  bool connection = watch.isConnected();

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

bool readDialBytes(const char *path, uint8_t *data, size_t offset, size_t size)
{
  File file = FLASH.open(path, "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return false;
  }

  if (!file.seek(offset))
  {
    Serial.println("Failed to seek file");
    file.close();
    return false;
  }

  int bytesRead = file.readBytes((char *)data, size);

  if (bytesRead <= 0)
  {
    Serial.println("Error reading file");
    file.close();
    return false;
  }

  file.close();
  return true;
}

bool isKnown(uint8_t id)
{
  if (id < 0x1E)
  {
    if (id != 0x04 || id != 0x05 || id != 0x12 || id != 0x18 || id != 0x20)
    {
      return true;
    }
  }
  else
  {
    if (id == 0xFA || id == 0xFD)
    {
      return true;
    }
  }
  return false;
}

String hexString(uint8_t *arr, size_t len, bool caps, String separator)
{
  String hexString = "";
  for (size_t i = 0; i < len; i++)
  {
    char hex[3];
    sprintf(hex, caps ? "%02X" : "%02x", arr[i]);
    hexString += separator;
    hexString += hex;
  }
  return hexString;
}

String longHexString(unsigned long l)
{
  char buffer[9];             // Assuming a 32-bit long, which requires 8 characters for hex representation and 1 for null terminator
  sprintf(buffer, "%08x", l); // Format as 8-digit hex with leading zeros
  return String(buffer);
}

void parseDial(const char *path, bool restart)
{

#ifdef ENABLE_CUSTOM_FACE
  String name = longHexString(watch.getEpoch());

  Serial.print("Parsing dial:");
  Serial.println(path);

  JsonDocument json;
  JsonDocument elements;
  JsonDocument assetFiles;
  JsonArray elArray = elements.to<JsonArray>();
  JsonArray assetArray = assetFiles.to<JsonArray>();

  json["name"] = name;
  json["file"] = String(path);

  JsonDocument rsc;
  int errors = 0;

  uint8_t az[1];
  if (!readDialBytes(path, az, 0, 1))
  {
    Serial.println("Failed to read watchface header");
    errors++;
  }
  uint8_t j = az[0];

  static uint8_t item[20];
  static uint8_t table[512];

  uint8_t lid = 0;
  int a = 0;
  int lan = 0;
  int tp = 0;
  int wt = 0;

  for (int i = 0; i < j; i++)
  {
    if (i >= 60)
    {
      Serial.println("Too many watchface elements >= 60");
      break;
    }

    JsonDocument element;

    if (!readDialBytes(path, item, (i * 20) + 4, 20))
    {
      Serial.println("Failed to read element properties");
      errors++;
    }

    uint8_t id = item[0];

    element["id"] = id;

    uint16_t xOff = item[5] * 256 + item[4];
    uint16_t yOff = item[7] * 256 + item[6];

    element["x"] = xOff;
    element["y"] = yOff;

    uint16_t xSz = item[9] * 256 + item[8];
    uint16_t ySz = item[11] * 256 + item[10];

    uint32_t clt = item[15] * 256 * 256 * 258 + item[14] * 256 * 256 + item[13] * 256 + item[12];
    uint32_t dat = item[19] * 256 * 256 * 256 + item[18] * 256 * 256 + item[17] * 256 + item[16];

    uint8_t id2 = item[1];

    bool isG = (item[1] & 0x80) == 0x80;

    if (id == 0x08)
    {
      isG = true;
    }

    uint8_t cmp = isG ? (item[1] & 0x7F) : 1;

    int aOff = item[2];

    bool isM = (item[3] & 0x80) == 0x80;
    uint8_t cG = isM ? (item[3] & 0x7F) : 1;

    if (!isKnown(id))
    {
      continue;
    }

    if (id == 0x16 && (item[1] == 0x06 || item[1] == 0x00))
    {
      // weather (-) label
      continue;
    }
    if (isM)
    {
      lan++;
    }

    if (tp == 0x09 && id == 0x09)
    {
      a++;
    }
    else if (tp != id)
    {
      tp = id;
      a++;
    }
    else if (lan == 1)
    {
      a++;
    }

    if (xSz == 0 || ySz == 0)
    {
      continue;
    }

    int z = i;
    int rs = -1;

    bool createFile = false;

    if (rsc.containsKey(String(clt)))
    {
      z = rsc[String(clt)].as<int>();
      rs = z;
    }

    bool drawable = (id == 0x0d) ? (lan == 1 || lan == 17 || lan == 33) : true;

    JsonDocument grp;
    JsonArray grpArr = grp.to<JsonArray>();

    if (rs == -1 && drawable)
    {
      rsc[String(clt)] = i;

      for (int aa = 0; aa < cmp; aa++)
      {
        unsigned long nm = (i * 10000) + (clt * 10) + aa;
        grpArr.add("S:" + name + "_" + longHexString(nm) + ".bin");
      }

      if (id == 0x17)
      {
      }
      else if (id == 0x0A)
      {
      }
      else if (cmp == 1)
      {
      }
      else
      {
      }

      // save asset
      createFile = true;
    }
    else if (id == 0x16 && id2 == 0x00)
    {
      // save asset
      createFile = true;

      for (int aa = 0; aa < cmp; aa++)
      {
        unsigned long nm = (z * 10000) + (clt * 10) + aa;
        grpArr.add("S:" + name + "_" + longHexString(nm) + ".bin");
      }
    }
    else
    {

      for (int aa = 0; aa < cmp; aa++)
      {
        unsigned long nm = (z * 10000) + (clt * 10) + aa;
        grpArr.add("S:" + name + "_" + longHexString(nm) + ".bin");
      }
    }

    if (cmp <= 1)
    {
      // grp is null
      grpArr.clear();
    }

    if (id == 0x0A)
    {
      // if (connIC.count { it == '\n' } < 3) {
      //     continue
      // }
    }

    if (isM)
    {
      if (lan == cG)
      {
        lan = 0;
      }
      else if (id == 0x0d && (lan == 1 || lan == 32 || lan == 40 || lan == 17 || lan == 33))
      {
        yOff -= (ySz - aOff);
        xOff -= aOff;
      }
      else
      {
        continue;
      }
    }
    if (id == 0x17)
    {
      wt++;
      if (wt != 1)
      {
        continue;
      }
    }

    if (id == 0x16 && id2 == 0x06)
    {
      continue;
    }

    if (drawable)
    {
      element["pvX"] = aOff;
      element["pvY"] = ySz - aOff;

      unsigned long nm = (z * 10000) + (clt * 10) + 0;

      element["image"] = "S:" + name + "_" + longHexString(nm) + ".bin";
      element["group"] = grpArr;

      elArray.add(element);
    }

    Serial.printf("i:%d, id:%d, xOff:%d, yOff:%d, xSz:%d, ySz:%d, clt:%d, dat:%d, cmp:%d\n", i, id, xOff, yOff, xSz, ySz, clt, dat, cmp);

    if (!createFile)
    {
      continue;
    }
    uint8_t cf = (id == 0x09 && i == 0) || (id == 0x19) ? 0x12 : 0x13;
    bool tr = cf == 0x13;
    uint16_t st = (cf == 0x12) ? 2 : 3;

    for (int b = 0; b < cmp; b++)
    {
      unsigned long nm = (z * 10000) + (clt * 10) + b;

      String asset = "/" + name + "_" + longHexString(nm) + ".bin";
      Serial.print("Create asset-> ");
      Serial.print(asset);

      assetArray.add(asset);

      uint8_t header[12];

      lvImgHeader(header, cf, xSz, ySz / cmp, xSz * st);

      Serial.print("\t");
      Serial.println(hexString(header, 12));

      File ast = FLASH.open(asset.c_str(), FILE_WRITE);
      if (ast)
      {
        ast.write(header, 12);

        if (!readDialBytes(path, table, clt, 512))
        {
          Serial.println("Could not read color table bytes from file");
          errors++;
          break;
        }

        uint16_t yZ = uint16_t(ySz / cmp); // height of individual element

        File file = FLASH.open(path, "r");
        if (!file)
        {
          Serial.println("Failed to open file for reading");
          errors++;
          break;
        }
        int offset = (xSz * yZ) * b;

        if (!file.seek(dat + offset))
        {
          Serial.println("Failed to seek file");
          file.close();
          errors++;
          break;
        }

        int x = 0;
        if (id == 0x19)
        {
          for (int z = 0; z < (xSz * yZ); z++)
          {
            uint8_t pixel[2];
            pixel[0] = item[13];
            pixel[1] = item[12];
            ast.write(pixel, 2);
          }
        }
        else
        {
          while (file.available())
          {
            uint16_t index = file.read();

            uint8_t pixel[3];
            if (tr)
            {
              pixel[1] = table[index * 2];
              pixel[2] = table[(index * 2) + 1];
              pixel[0] = (uint16_t(pixel[1] * 256 + pixel[2]) == 0) ? 0x00 : 0xFF; // alpha byte (black pixel [0] is transparent)
              ast.write(pixel, 3);
            }
            else
            {
              pixel[0] = table[index * 2];
              pixel[1] = table[(index * 2) + 1];

              ast.write(pixel, 2);
            }
            x++;
            if (x >= (xSz * yZ))
            {
              break;
            }
          }
        }
        file.close();

        ast.close();
      }
      else
      {
        errors++;
      }
    }
  }

  json["elements"] = elements;
  json["assets"] = assetFiles;

  serializeJsonPretty(json, Serial);

  String jsnFile = "/" + name + ".json";
  assetArray.add(jsnFile);
  File jsn = FLASH.open(jsnFile, FILE_WRITE);

  if (jsn)
  {
    serializeJsonPretty(json, jsn);
    jsn.flush();
    jsn.close();
  }
  else
  {
    errors++;
  }

  if (errors > 0)
  {
    // failed to parse watchface files
    // probably delete assetfiles
    Serial.print(errors);
    Serial.println(" errors encountered when parsing watchface");
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(80, 80);
    tft.print("Failed");
  }
  else
  {
    // success
    // probably delete source file

    tft.fillScreen(TFT_GREEN);
    tft.setTextColor(TFT_WHITE, TFT_GREEN);
    tft.setTextSize(2);
    tft.setCursor(80, 80);
    tft.print("Success");

    deleteFile(path);
    Serial.println("Watchface parsed successfully");

    prefs.putString("custom", jsnFile);
  }

  if (restart)
  {
    delay(500);
    ESP.restart();
  }

#endif
}

bool lvImgHeader(uint8_t *byteArray, uint8_t cf, uint16_t w, uint16_t h, uint16_t stride)
{

  byteArray[0] = LV_IMAGE_HEADER_MAGIC;
  byteArray[1] = cf;
  byteArray[2] = 0;
  byteArray[3] = 0;


  byteArray[4] = (w & 0xFF);
  byteArray[5] = (w >> 8) & 0xFF;

  byteArray[6] = (h & 0xFF);
  byteArray[7] = (h >> 8) & 0xFF;

  byteArray[8] = (stride & 0xFF);
  byteArray[9] = (stride >> 8) & 0xFF;

  byteArray[10] = 0;
  byteArray[11] = 0;

  return true;
}
